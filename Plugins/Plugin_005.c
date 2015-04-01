//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                 Plugin-03: Intertek Eurodomest 972080                             ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of sending and receiving the Intertek Eurodomest 972080 protocol. 
 * 
 * Author             : StuntTeam
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 ***********************************************************************************************
 * Incoming event: "Eurodomest <adres>,<unitcode>, <On | Off>
 * Send          : "EurodomestSend <Adres>,<unitcode>, <On | Off> 
 *
 * Address = hexadecimal address
 ***********************************************************************************************
 * Technical information:
 *
 * 0111 00011011 00011111 000 0
 * AAAA AAAAAAAA AAAAAAAA BBB C
 * 
 * A = ID (20 bits) Note: addresses with the highest 4 bits all set are not accepted for now.                  
 * B = UnitCode (3 bits)
 * C = switch code (ON/OFF) (1 bit)
 *
 * 20;9D;DEBUG;Pulses=50;Pulses(uSec)=1250,200,750,175,200,750,200,750,750,200,200,750,200,750,750,200,200,750,750,200,750,200,200,750,750,200,200,750,200,750,750,200,200,750,750,200,200,750,200,750,750,200,750,200,750,200,750,200,200;
 * 1010010110010110011010011001011001100101101010100
 * 00110110100101101011 000 0
 * 01110001101100011111 000 0
\*********************************************************************************************/
#define PLUGIN_ID 5

#define Eurodomest_PulseLength    50
#define Eurodomest_MaxPulse       900
#define PLUGIN_005_EVENT        "Eurodomest"
#define PLUGIN_005_COMMAND      "EurodomestSend"

void Eurodomest_Send(unsigned long address);

boolean Plugin_005(byte function, struct NodoEventStruct *event, char *string)
  {
    boolean success=false;
    unsigned long bitstream=0;
  
  switch(function)
    {
    #ifdef PLUGIN_005_CORE

    case PLUGIN_RAWSIGNAL_IN:
      {
      byte housecode=0;
      byte unitcode=0;
      byte command=0;
      char buffer[14]=""; 
      unsigned long address=0;
      // ==========================================================================
      if (RawSignal.Number!= (Eurodomest_PulseLength) ) return false; 
      if(RawSignal.Pulses[49]*RawSignal.Multiply > 400) return false;  // last pulse (stop bit) needs to be short, otherwise no Eurodomest protocol
      // get all 24 bits
      for(int x=2;x < Eurodomest_PulseLength;x+=2) {
          if(RawSignal.Pulses[x]*RawSignal.Multiply > 400) {
             if(RawSignal.Pulses[x]*RawSignal.Multiply > Eurodomest_MaxPulse) return false; // make sure the long pulse is within range
             bitstream = (bitstream << 1) | 0x1; 
         } else {
             bitstream = (bitstream << 1);
         }
      }
      //==================================================================================
      // perform sanity checks to prevent false positives
      if (bitstream==0) return false;               // no bits detected? 
      address=bitstream;
      address=(address >> 4) &0xfffff;
      address=(address >> 4) &0xfffff;
      if ( (address ) & 0xf0000 == 0xf0000) return false;   // Addresses with the highest 4 bits all set are not accepted 
      if (address==0) return false;                 // Address would never be 0 
      if (address==0xfffff) return false;           // Address would never be FFFFF
      // ----------------------------------
      unitcode=(( bitstream >> 1)& 0x7);
      command=((bitstream) & 0x01);              
      if (unitcode == 3) return false;              // invalid button code?
      if (unitcode == 4) unitcode--;                // unitcode 5 is present on the PCB and working.
      if (unitcode > 7) return false;               // invalid button code?
      housecode=(address >> 16) &0xff;
      //==================================================================================
      // Output
      // ----------------------------------
      sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
      Serial.print( buffer );
      // ----------------------------------
      Serial.print("Eurodomest;");                  // Label
      sprintf(buffer, "ID=%02x%04x;", housecode, (address)&0xffff) ; // ID    
      Serial.print( buffer );
      sprintf(buffer, "SWITCH=%02x;", unitcode);    // ID    
      Serial.print( buffer );
      Serial.print("CMD=");                    
      if ( unitcode > 4) {
         Serial.print("ALL");
         if ( command == 0) {
            Serial.print("OFF;");
         } else {
            Serial.print("ON;");
         }
      } else {     
         if ( command == 1) {
            Serial.print("OFF;");
         } else {
            Serial.print("ON;");
         }
      }
      Serial.println();
      // ----------------------------------
      RawSignal.Repeats = true;                     // Suppress repeating RF packets
      RawSignal.Number=0;
      success=true;
      break;
      }
    case PLUGIN_COMMAND:
      {
      event->Port=VALUE_ALL;                        // Signaal mag naar alle door de gebruiker met [Output] ingestelde poorten worden verzonden.
      Eurodomest_Send(event->Par2);                 // event->Par2 contains the full bitstream to send
      success=true;
      break;
    }
    #endif //PLUGIN_CORE_005
      
    #if NODO_MEGA
    case PLUGIN_MMI_IN:
      {
        char *str=(char*)malloc(INPUT_COMMAND_SIZE);
        if(GetArgv(string,str,1)) {
            event->Type  = 0;
            if(strcasecmp(str,PLUGIN_005_COMMAND)==0) {
                event->Type  = NODO_TYPE_PLUGIN_COMMAND;
            }
            if(event->Type) {
                event->Command = PLUGIN_ID;         // Plugin number
     
                if(GetArgv(string,str,2)) {         // Het door de gebruiker ingegeven eerste parameter bevat het adres
                   event->Par2=str2int(str); 
                   if(GetArgv(string,str,3)) {      // Het door de gebruiker ingegeven tweede parameter bevat het button nummer
                      byte temp=str2int(str);
                      event->Par2=(event->Par2) << 4;
                      if (temp == 1) event->Par2=event->Par2+0x02; // 0010
                      if (temp == 2) event->Par2=event->Par2+0x04; // 0100
                      if (temp == 3) event->Par2=event->Par2+0x08; // 1000
                      if (temp == 6) event->Par2=event->Par2+0x0d; // 1101
                      if (temp == 7) event->Par2=event->Par2+0x0f; // 1111
                      if (temp < 8) {
                         if(GetArgv(string,str,4)) {  // Het door de gebruiker ingegeven derde parameter bevat het on/off commando
                            event->Par1=str2cmd(str);
                            if (event->Par1==VALUE_OFF) { 
                                event->Par2=event->Par2|1;
                            }
                            success=true;
                         }
                      }
                   }
                }
            }
        }
        free(str);
        break;
      }
    #endif //MMI 
    }      
    return success;
}
          
void Eurodomest_Send(unsigned long address) { 
    int fpulse = 296;                               // Pulse witdh in microseconds
    int fretrans = 7;                               // Number of code retransmissions
    uint32_t fdatabit;
    uint32_t fdatamask = 0x800000;
    uint32_t fsendbuff;

    digitalWrite(PIN_RF_RX_VCC,LOW);                // Turn off power to the RF receiver 
    digitalWrite(PIN_RF_TX_VCC,HIGH);               // Enable the 433Mhz transmitter
    delay(TRANSMITTER_STABLE_TIME);                 // Short delay to get the transmitter stable

    for (int nRepeat = 0; nRepeat <= fretrans; nRepeat++) {
        fsendbuff=address;
        // Send command
        for (int i = 0; i < 24; i++) {              // Eurodomest packet is 24 bits 
            // read data bit
            fdatabit = fsendbuff & fdatamask;       // Get most left bit
            fsendbuff = (fsendbuff << 1);           // Shift left

            if (fdatabit != fdatamask) {            // Write 0
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse * 3);
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse * 1);
            } else {                                // Write 1
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse * 1);
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse * 3);
            }
        }
        digitalWrite(PIN_RF_TX_DATA, HIGH);         
        delayMicroseconds(fpulse * 1);
        digitalWrite(PIN_RF_TX_DATA, LOW);          // and lower the signal
        delayMicroseconds(fpulse * 32);
    }
    delay(TRANSMITTER_STABLE_TIME);                 // Short delay to keep the air clean after the stop bit
    digitalWrite(PIN_RF_TX_VCC,LOW);                // Turn thew 433Mhz transmitter off
    digitalWrite(PIN_RF_RX_VCC,HIGH);               // Turn the 433Mhz receiver on
}
