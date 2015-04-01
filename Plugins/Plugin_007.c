//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                      Plugin-007 CONRAD RSL2                                       ##
//#######################################################################################################
/*********************************************************************************************\
 * This Plugin takes care of receiving of the Conrad RSL2 protocol
 *
 * Author             : StuntTeam
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical information:            
 * RF packets contain 66 pulses, 32 bits
 *
 * Conrad RSL2 Message Format: 
 * AABBCDDD EEEEEEEE EEEEEEEE EEEEEEEE
 *
 * A = always 10
 * B = Button code
 * C = on/off command (inverted for some buttons/groups)
 * D = group code
 * E = 24 bit address
 *
 * Details: http://www.mikrocontroller.net/topic/252895
 *
 * ConradSend address,switch,cmd;  => (16 bits,8 bits,on/off/allon/alloff)
 \*********************************************************************************************/
#define PLUGIN_ID 07
#define PLUGIN_NAME "Conrad RSL2"
#define PLUGIN_007_EVENT        "Conrad"
#define PLUGIN_007_COMMAND  "ConradSend"
#define CONRADRSL2_PULSECOUNT 66

void RSL2_Send(unsigned long address);

boolean Plugin_007(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef PLUGIN_007_CORE
  case PLUGIN_RAWSIGNAL_IN:
    {
      //==================================================================================
      unsigned long bitstream=0;
      byte checksum=0;
      byte button=0;
      byte group=0;
      byte action=0;
      char buffer[14]=""; 

      if (RawSignal.Number != CONRADRSL2_PULSECOUNT) return false;
      //==================================================================================
      // get bits
      for(byte x=1;x<RawSignal.Number-2;x=x+2) {
         if (RawSignal.Pulses[x]*RawSignal.Multiply > 600) {
            bitstream = (bitstream << 1) | 0x1;     // 
         } else {
            bitstream = (bitstream << 1);           // 
         }
      }
      //==================================================================================
      // Prevent repeating signals from showing up
      //==================================================================================
      if(!RawSignal.RepeatChecksum && (SignalHash!=SignalHashPrevious || RepeatingTimer<millis())) {
         // not seen the RF packet recently
      } else {
         // already seen the RF packet recently
         return true;
      }
      //==================================================================================
      // all bits received, make sure checksum is okay
      //==================================================================================
      checksum=((bitstream) >> 30);                 // first two bits should always be '10'
      if (checksum != 2) return false; 
      //==================================================================================
      byte hibyte=(bitstream)>> 16;
      button=((bitstream) >> 24);                   // 10100011
      // ----- check for possible valid values
      if (button < 0x81) return false;
      if (button > 0xbe) return false;
      byte temp=(button)&0xf;
      if (temp == 0x07 || temp == 0x0b || temp == 0x0f ) return false;
      if (button == 0x83 || button == 0x86 || button == 0x89 || button == 0x8c || button == 0x91 ) return false;
      if (button == 0x94 || button == 0x9a || button == 0x9d || button == 0xa1 || button == 0xa4 ) return false;
      if (button == 0xaa || button == 0xad || button == 0xb1 || button == 0xb3 || button == 0xb4 ) return false;
      if (button == 0xba || button == 0xbd ) return false;
      // -----
      group=(button) & 0x7;                         // --   111
      action=((button) >> 3) & 0x1;                 // --  a
      button = ((button) >> 4) & 0x3;               // --bb
      if (button==3) { 
         if (group == 6) button=0;
         if (group == 1 || group == 5) {
            button=4;
            action!=action;
         }
         if (group == 0) button=8;
         if (group == 2 || group == 4) {
            button=12;
            action!=action;
         }
      } else
      if (button==0) { 
         if (group == 6 || group == 1) {
            button=1;
            action!=action;
         }
         if (group == 5) button=5;
         if (group == 0 || group == 4) {
            button=9;
            action!=action;
         }
         if (group == 2) button=13;
      } else
      if (button==2) { 
         if (group == 6) button=2;
         if (group == 1 || group == 5) button=6;
         if (group == 0) button=10;
         if (group == 2 || group == 4) {
            button=14;
            action!=action;
         }
      } else
      if (button==1) { 
         if (group == 6) button=3;
         if (group == 1 || group == 5) {
            button=7;
            action!=action;
         }
         if (group == 0) button=11;
         if (group == 2 || group == 4) {
            button=15;
            action!=action;
         }
      } 
      //==================================================================================
      // Output
      // ----------------------------------
      sprintf(buffer, "20;%02X;", PKSequenceNumber++);// Node and packet number 
      Serial.print( buffer );
      // ----------------------------------
      Serial.print("Conrad RSL2;");                 // Label
      sprintf(buffer, "ID=%02x%04x;", hibyte, bitstream &0xffffff); // ID      
      Serial.print( buffer );
      sprintf(buffer, "SWITCH=%02x;", button);      // Button number
      Serial.print( buffer );
      Serial.print("CMD=");                         // command
      if (group==3) { 
         Serial.print("ALL");           
         if (button==1) Serial.print("ON;"); 
         if (button==2) Serial.print("OFF;");
      } else {
         if (action==0) Serial.print("ON;");
         if (action==1) Serial.print("OFF;");
      }
      Serial.println();
      //==================================================================================
      RawSignal.Repeats=true;                         // suppress RF signal repeats
      RawSignal.Number=0;
      success=true;
      break;
    }
    case PLUGIN_COMMAND:
      {
      event->Port=VALUE_ALL;                           // Signaal mag naar alle door de gebruiker met [Output] ingestelde poorten worden verzonden.
      RSL2_Send(event->Par2);                    // event->Par2 contains the full bitstream to send
      success=true;
      break;
    }    
#endif // PLUGIN_007_CORE
    #if NODO_MEGA
    case PLUGIN_MMI_IN:
      {
        char *str=(char*)malloc(INPUT_COMMAND_SIZE);
        if(GetArgv(string,str,1)) {
            event->Type  = 0;
            if(strcasecmp(str,PLUGIN_007_COMMAND)==0) {
                event->Type  = NODO_TYPE_PLUGIN_COMMAND;
            }

            if(event->Type) {
                event->Command = PLUGIN_ID;            // Plugin nummer  
                unsigned long command=0;   
     
                if(GetArgv(string,str,2)) {            // Het door de gebruiker ingegeven eerste parameter bevat het adres
                   event->Par2=str2int(str); 
                   if(GetArgv(string,str,3)) {         // Het door de gebruiker ingegeven tweede parameter bevat het button nummer
                      byte temp=str2int(str);
                      if (temp < 16) {                 // No button with a number higher than 15
                         if(GetArgv(string,str,4)) {   // Het door de gebruiker ingegeven derde parameter bevat het on/off commando
                            event->Par1=str2cmd(str);
                            if (event->Par1==VALUE_OFF) { 
                               if (temp == 0) command=0xbe;  
                               if (temp == 1) command=0x81;  
                               if (temp == 2) command=0xae; 
                               if (temp == 3) command=0x9e; 
                               if (temp == 4) command=0xb5;  
                               if (temp == 5) command=0x8d;  
                               if (temp == 6) command=0xa5; 
                               if (temp == 7) command=0x95; 
                               if (temp == 8) command=0xb8; 
                               if (temp == 9) command=0x84; 
                               if (temp ==10) command=0xa8; 
                               if (temp ==11) command=0x98; 
                               if (temp ==12) command=0xb2; 
                               if (temp ==13) command=0x8a; 
                               if (temp ==14) command=0xa2; 
                               if (temp ==15) command=0x92; 
                            } else     // ON
                            if (event->Par1==VALUE_ON) { 
                               if (temp == 0) command=0xb6; 
                               if (temp == 1) command=0x8e; 
                               if (temp == 2) command=0xa6; 
                               if (temp == 3) command=0x96; 
                               if (temp == 4) command=0xb9;  
                               if (temp == 5) command=0x85;  
                               if (temp == 6) command=0xa9; 
                               if (temp == 7) command=0x99; 
                               if (temp == 8) command=0xb0; 
                               if (temp == 9) command=0x88; 
                               if (temp ==10) command=0xa0; 
                               if (temp ==11) command=0x90; 
                               if (temp ==12) command=0xbc; 
                               if (temp ==13) command=0x82; 
                               if (temp ==14) command=0xac; 
                               if (temp ==15) command=0x9c; 
                            } else     // AllON
                            if (event->Par1==VALUE_ALLON) { 
                               command=0x93;
                            } else     // AllOff
                            if (event->Par1==VALUE_ALLOFF) { 
                               command=0xa3;
                            }
                            command=command << 24;
                            event->Par2=event->Par2+command;
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

void RSL2_Send(unsigned long address) { 
    int fpulse  = 650;                                  // Pulse witdh in microseconds 650? 
    int fpulse2 = 450;                                  // Pulse witdh in microseconds 650? 
    int fretrans = 4;                                  // Number of code retransmissions
    uint32_t fdatabit;
    uint32_t fdatamask = 0x80000000;
    uint32_t fsendbuff;

    digitalWrite(PIN_RF_RX_VCC,LOW);                   // Spanning naar de RF ontvanger uit om interferentie met de zender te voorkomen.
    digitalWrite(PIN_RF_TX_VCC,HIGH);                  // zet de 433Mhz zender aan
    delay(TRANSMITTER_STABLE_TIME);                    // kleine pauze om de zender de tijd te geven om stabiel te worden 

    
    for (int nRepeat = 0; nRepeat <= fretrans; nRepeat++) {
        fsendbuff=address;

        Serial.print("send:");   
        Serial.println(fsendbuff,HEX);   
    
        // send SYNC 1P High, 10P low
        digitalWrite(PIN_RF_TX_DATA, HIGH);
        delayMicroseconds(fpulse * 1);
        digitalWrite(PIN_RF_TX_DATA, LOW);
        delayMicroseconds(fpulse * 10);
        // end send SYNC
        // Send command
        for (int i = 0; i < 32; i++) {                 // 32 bits
            // read data bit
            fdatabit = fsendbuff & fdatamask;          // Get most left bit
            fsendbuff = (fsendbuff << 1);              // Shift left
            if (fdatabit != fdatamask) { // Write 0
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse2 * 1);
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse2 * 3);
            } else { // Write 1
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse2 * 3);
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse2 * 1);
            }
        }
        digitalWrite(PIN_RF_TX_DATA, HIGH);
        delayMicroseconds(fpulse2 * 1);
        digitalWrite(PIN_RF_TX_DATA, LOW);
        delayMicroseconds(fpulse * 14);
    }
    delay(TRANSMITTER_STABLE_TIME);                    // kleine pause zodat de ether even schoon blijft na de stopbit
    digitalWrite(PIN_RF_TX_VCC,LOW);                   // zet de 433Mhz zender weer uit
    digitalWrite(PIN_RF_RX_VCC,HIGH);                  // Spanning naar de RF ontvanger weer aan.
}
