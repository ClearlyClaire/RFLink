//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                          Plugin-006 Blyss                                         ##
//#######################################################################################################
/*********************************************************************************************\
 * This Plugin takes care of receiving of the Blyss protocol
 *
 * Author             : StuntTeam
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical information:            
 * RF packets contain 106 pulses, 52 bits
 *
 * BLYSS Message Format: 
 * AAAAAAAA BBBBCCCC CCCCCCCC CCCCDDDD | EEEEFFFF FFFFGGGG GGGG
 *
 * A = Preamble, always 0xFE
 * B = Global Channel (A=0,B=1,C=2,D=3)
 * C = Address
 * D = sub channel (channel 1=8, 2=4, 3=2, 4=1, 5=3) all channels = 0
 * E = Light Status
 * F = Rolling Code (0x98 -> 0xDA -> 0x1E -> 0xE6 -> 0x67)
 * G = Time Stamp (random value?)
 *  
 * Details https://skyduino.wordpress.com/2012/07/19/hack-partie-2-reverse-engineering-des-interrupteurs-domotique-blyss/
 * https://barbudor.wiki.zoho.com/Système-domotique-Blyss-de-Castorama.html
 *
 * BlyssSend address,switch,cmd;  => (16 bits,8 bits,on/off/allon/alloff)
 \*********************************************************************************************/
#define PLUGIN_ID 06
#define PLUGIN_NAME "Blyss"
#define PLUGIN_006_EVENT        "Blyss"
#define PLUGIN_006_COMMAND  "BlyssSend"
#define BLYSS_PULSECOUNT 106

void Blyss_Send(unsigned long address);

boolean Plugin_006(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef PLUGIN_006_CORE
  case PLUGIN_RAWSIGNAL_IN:
    {
      //==================================================================================
      unsigned long bitstream=0;
      unsigned long bitstream1=0;
      byte bitcounter=0;
      byte checksum=0;
      byte button=0;
      byte group=0;
      byte action=0;
      char buffer[14]=""; 

      if (RawSignal.Number != BLYSS_PULSECOUNT) return false;
      //==================================================================================
      // get bits
      for(byte x=2;x<RawSignal.Number;x=x+2) {
         if (RawSignal.Pulses[x]*RawSignal.Multiply > 500) {
            if (bitcounter < 32) {
                bitstream = (bitstream << 1); 
            } else {
                bitstream1 = (bitstream1 << 1); 
            }
            bitcounter++;
         } else {
            if (bitcounter < 32) {
                bitstream = (bitstream << 1) | 0x1;   
            } else {
                bitstream1 = (bitstream1 << 1) | 0x1;  
            }
            bitcounter++;
         }
      }
      //==================================================================================
      // all bits received, make sure checksum is okay
      //==================================================================================
      checksum=((bitstream) >> 24);                 
      if (checksum != 0xFE) return false;
      //==================================================================================
      //==================================================================================
      byte channel=((bitstream) >> 20) &0x0f;
      unsigned int address=((bitstream) >> 4) &0xffff;
      byte subchan=(bitstream) &0xf;
      byte status=((bitstream1) >> 16) &0x0f;
      channel=channel+0x41;
      if (subchan==8) {
         subchan=1;
      } else 
      if (subchan==4) {
         subchan=2;
      } else 
      if (subchan==2) {
         subchan=3;
      } else 
      if (subchan==1) {
         subchan=4;
      } else 
      if (subchan==3) {
         subchan=5;
      } 
      //==================================================================================
      // Output
      // ----------------------------------
      sprintf(buffer, "20;%02X;", PKSequenceNumber++);// Node and packet number 
      Serial.print( buffer );
      // ----------------------------------
      Serial.print("Blyss;");                 // Label
      sprintf(buffer, "ID=%04x;", address); // ID      
      Serial.print( buffer );
      sprintf(buffer, "SWITCH=%c%d;", channel,subchan);     
      Serial.print( buffer );
      Serial.print("CMD=");                         // command
      if (status==0) Serial.print("ON;"); 
      if (status==1) Serial.print("OFF;");
      if (status==2) Serial.print("ALLON;");
      if (status==3) Serial.print("ALLOFF;");
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
      Blyss_Send(event->Par2);                         // event->Par2 contains the middle part of the bitstream to send
      success=true;
      break;
    }    
#endif // PLUGIN_006_CORE
    #if NODO_MEGA
    case PLUGIN_MMI_IN:
      {
        char *str=(char*)malloc(INPUT_COMMAND_SIZE);
        // Hier aangekomen bevat string het volledige commando. Test als eerste of het opgegeven commando overeen komt met "EurodomestSend"
        // Dit is het eerste argument in het commando.
        if(GetArgv(string,str,1)) {
            event->Type  = 0;
            if (strcasecmp(str,PLUGIN_006_COMMAND)==0) {
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
                        } else
                        if (event->Par1==VALUE_ALLOFF) { 
                           event->Par2=event->Par2|3;
                        } else
                        if (event->Par1==VALUE_ALLON) { 
                           event->Par2=event->Par2|2;
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

void Blyss_Send(unsigned long address) { 
    int fpulse = 400;                                  // Pulse witdh in microseconds
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
        digitalWrite(PIN_RF_TX_DATA, LOW);
        delayMicroseconds(fpulse);
        digitalWrite(PIN_RF_TX_DATA, HIGH);
        delayMicroseconds(fpulse * 6);
        // end send SYNC
        // --------------
        // Send preamble (0xfe) - 8 bits
        fsendbuff=0xfe;
        fdatamask=0x80;
        for (int i = 0; i < 8; i++) {                  // Preamble
            // read data bit
            fdatabit = fsendbuff & fdatamask;          // Get most left bit
            fsendbuff = (fsendbuff << 1);              // Shift left
            if (fdatabit != fdatamask) { // Write 0
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse * 2);
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse * 1);
            } else { // Write 1
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse * 1);
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse * 2);
            }
        }
        // --------------
        fsendbuff=address;
        fdatamask=0x8000000;
        // Send command (channel/address/status) - 28 bits
        for (int i = 0; i < 28; i++) {                 
            // read data bit
            fdatabit = fsendbuff & fdatamask;          // Get most left bit
            fsendbuff = (fsendbuff << 1);              // Shift left
            if (fdatabit != fdatamask) { // Write 0
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse * 2);
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse * 1);
            } else { // Write 1
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse * 1);
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse * 2);
            }
        }
        // --------------
        // Send rolling code & timestamp - 16 bits
        fsendbuff=0x9800 + temp;
        fdatamask=0x8000;
        for (int i = 0; i < 16; i++) {    
            // read data bit
            fdatabit = fsendbuff & fdatamask;          // Get most left bit
            fsendbuff = (fsendbuff << 1);              // Shift left
            if (fdatabit != fdatamask) { // Write 0
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse * 2);
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse * 1);
            } else { // Write 1
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse * 1);
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse * 2);
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
