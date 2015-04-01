//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                      Plugin-008 Kambrook                                          ##
//#######################################################################################################
/*********************************************************************************************\
 * This Plugin takes care of receiving of the Kambrook protocol
 * Device models:  RF3399/RF3405/RF3672/RF3689/RF4471R 
 * Made by Ningbo Comen Electronics Technology Co. Ltd. 
 *
 * Author             : StuntTeam
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical information:            
 * RF packets contain 96 pulses, 48 bits
 *
 * Kambrook Message Format: 
 * AAAAAAAA BBBBBBBB BBBBBBBB BBBBBBBB CCCCCCCC DDDDDDDD
 *
 * A = Preamble, always 0x55
 * B = Address
 * C = Channel/Command
 * D = Trailing, always 0xFF
 *  
 * Details http://wiki.beyondlogic.org/index.php?title=Reverse_engineering_the_RF_protocol_on_a_Kambrook_Power_Point_Controller
 \*********************************************************************************************/
#define PLUGIN_ID   0x08
#define PLUGIN_NAME "Kambrook"
#define PLUGIN_008_EVENT        "Kambrook"
#define PLUGIN_008_COMMAND  "KambrookSend"
#define KAMBROOK_PULSECOUNT 96

void Kambrook_Send(unsigned long address);

boolean Plugin_008(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef PLUGIN_008_CORE
  case PLUGIN_RAWSIGNAL_IN:
    {
      //==================================================================================
      byte sync=0;
      unsigned long address=0;
      byte command=0;
      byte trailing=0;
      
      byte bitcounter=0;
      char buffer[14]=""; 
      byte status=0;
      byte channel=0;
      byte subchan=0;
      
      if (RawSignal.Number != KAMBROOK_PULSECOUNT) return false;
      //==================================================================================
      // get bits
      for(byte x=1;x<RawSignal.Number;x=x+2) {
         if (RawSignal.Pulses[x]*RawSignal.Multiply > 400) {
            if (bitcounter < 8) {
                sync = (sync << 1) | 0x1;   
            } else 
            if (bitcounter < 32) {
                address = (address << 1) | 0x1;  
            } else 
            if (bitcounter < 40) {
                command = (command << 1) | 0x1;  
            } else {
                trailing = (trailing << 1) | 0x1;  
            }
            bitcounter++;
         } else {
            if (bitcounter < 8) {
                sync = (sync << 1);   
            } else 
            if (bitcounter < 32) {
                address = (address << 1);  
            } else 
            if (bitcounter < 40) {
                command = (command << 1);  
            } else {
                trailing = (trailing << 1);  
            }
            bitcounter++;
         }
      }
      //==================================================================================
      // all bits received, make sure checksum is okay
      //==================================================================================
      if (sync != 0x55) return false;
      if (trailing != 0xff) return false;
      //==================================================================================
      //==================================================================================
      byte hibyte=(address)>>16;
      status=(command)&1;                   // 0/1 off/on

      subchan=(((command) >> 1)&7) + 1;     // button code
      if (status == 0) subchan--;

      byte temp=(command) >> 4;             // channel code
      channel=0x41+temp;
      //==================================================================================
      // Output
      // ----------------------------------
      sprintf(buffer, "20;%02X;", PKSequenceNumber++);// Node and packet number 
      Serial.print( buffer );
      // ----------------------------------
      Serial.print("Kambrook;");                 // Label
      sprintf(buffer, "ID=%02x%04x;", hibyte, address); // ID      
      Serial.print( buffer );
      sprintf(buffer, "SWITCH=%c%d;", channel,subchan);     
      Serial.print( buffer );
      Serial.print("CMD=");                         // command
      if (status==0) Serial.print("OFF;"); 
      if (status==1) Serial.print("ON;");
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
      Kambrook_Send(event->Par2);                      // event->Par2 contains the middle part of the bitstream to send
      success=true;
      break;
    }    
#endif // PLUGIN_008_CORE
    #if NODO_MEGA
    case PLUGIN_MMI_IN:
      {
        char *str=(char*)malloc(INPUT_COMMAND_SIZE);
        // Hier aangekomen bevat string het volledige commando. Test als eerste of het opgegeven commando overeen komt met "EurodomestSend"
        // Dit is het eerste argument in het commando.
        if(GetArgv(string,str,1)) {
            event->Type  = 0;
            if (strcasecmp(str,PLUGIN_008_COMMAND)==0) {
               event->Type  = NODO_TYPE_PLUGIN_COMMAND;
            }
            if (event->Type) {
               event->Command = PLUGIN_ID;            // Plugin nummer  
               unsigned long Home=0;                           // Blyss channel A..P
               byte Address=0;                        // Blyss subchannel 1..5
               byte c;
               byte x=0;                              // teller die wijst naar het te behandelen teken
               byte subchan=0;                        // subchannel
                
               if (GetArgv(string,str,2)) {           // Het door de gebruiker ingegeven eerste parameter bevat het adres
                  event->Par2=str2int(str); 
                  if (GetArgv(string,str,3)) {        // Het door de gebruiker ingegeven tweede parameter bevat adres volgens codering A0..P16 
                     while((c=tolower(str[x++]))!=0) {
                          if(c>='1' && c<='5'){Address=Address+c-'0';}
                          if(c>='a' && c<='p'){Home=c-'a';}                                 // KAKU home A is intern 0
                     }
                     
                     if (Address==1) subchan=0x80; 
                     if (Address==2) subchan=0x40; 
                     if (Address==3) subchan=0x20; 
                     if (Address==4) subchan=0x10; 
                     if (Address==5) subchan=0x30; 

                     Home = Home << 24;
                     event->Par2=(event->Par2) << 8;
                     event->Par2=event->Par2+subchan;
                     event->Par2=event->Par2+Home;
                     if (GetArgv(string,str,4)) {   // Het door de gebruiker ingegeven derde parameter bevat het on/off commando
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
        free(str);
        break;
      }
    #endif //MMI 
  }      
  return success;
}

void Kambrook_Send(unsigned long address) { 
    int fpulse = 300;                                  // Pulse witdh in microseconds
    int fretrans = 5;                                  // Number of code retransmissions
    uint32_t fdatabit;
    uint32_t fdatamask = 0x800000;
    uint32_t fsendbuff;

    digitalWrite(PIN_RF_RX_VCC,LOW);                   // Spanning naar de RF ontvanger uit om interferentie met de zender te voorkomen.
    digitalWrite(PIN_RF_TX_VCC,HIGH);                  // zet de 433Mhz zender aan
    delay(TRANSMITTER_STABLE_TIME);                    // kleine pauze om de zender de tijd te geven om stabiel te worden 
    byte temp=(millis() &0xff);                        // used for the timestamp at the end of the RF packet
    for (int nRepeat = 0; nRepeat <= fretrans; nRepeat++) {
        // send SYNC 1P low, 6P high
        //digitalWrite(PIN_RF_TX_DATA, LOW);
        //delayMicroseconds(fpulse);
        //digitalWrite(PIN_RF_TX_DATA, HIGH);
        //delayMicroseconds(fpulse * 6);
        // end send SYNC
        // --------------
        // Send preamble (0x55) - 8 bits
        fsendbuff=0x55;
        fdatamask=0x80;
        for (int i = 0; i < 8; i++) {                  // Preamble
            // read data bit
            fdatabit = fsendbuff & fdatamask;          // Get most left bit
            fsendbuff = (fsendbuff << 1);              // Shift left
            if (fdatabit != fdatamask) { // Write 0
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse * 1);
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse * 1);
            } else { // Write 1
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse * 2);
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse * 1);
            }
        }
        // --------------
        fsendbuff=address;
        fdatamask=0x80000000;
        // Send command (channel/address/status) - 32 bits
        for (int i = 0; i < 28; i++) {                 
            // read data bit
            fdatabit = fsendbuff & fdatamask;          // Get most left bit
            fsendbuff = (fsendbuff << 1);              // Shift left
            if (fdatabit != fdatamask) { // Write 0
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse * 1);
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse * 1);
            } else { // Write 1
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse * 2);
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse * 1);
            }
        }
        // --------------
        // Send trailing bits - 8 bits
        fsendbuff=0xFF;
        fdatamask=0x80;
        for (int i = 0; i < 16; i++) {    
            // read data bit
            fdatabit = fsendbuff & fdatamask;          // Get most left bit
            fsendbuff = (fsendbuff << 1);              // Shift left
            if (fdatabit != fdatamask) { // Write 0
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse * 1);
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse * 1);
            } else { // Write 1
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse * 2);
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse * 1);
            }
        }
        // --------------
        digitalWrite(PIN_RF_TX_DATA, LOW);
        delayMicroseconds(fpulse * 14);
    }
    delay(TRANSMITTER_STABLE_TIME);                    // kleine pause zodat de ether even schoon blijft na de stopbit
    digitalWrite(PIN_RF_TX_VCC,LOW);                   // zet de 433Mhz zender weer uit
    digitalWrite(PIN_RF_RX_VCC,HIGH);                  // Spanning naar de RF ontvanger weer aan.
}
