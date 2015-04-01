//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                             Plugin-70 Select Plus Wireless Doorbell                               ##
//#######################################################################################################
/*********************************************************************************************\
 * This Plugin takes care of reception of the Select Plus wireless doorbell (Sold at Action for 6 euro's)
 * PCB markings: Quhwa QH-C-CE-3V aka QH-832AC
 * Also sold as "1 by One" and "Delta" wireless doorbell.
 *
 * Author             : StuntTeam
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical information:
 *
 * There are two known models:
 * SelectPlus (200689103 - Black - Datecode:0614) also sold as "1 by One" (O00 QH-0031)
 * SelectPlus (200689101 - White - Datecode:0914)	

 * Each frame is 35 pulses long. It is composed of: 
 * 101011001010110010110011010 10101010
 * The first block appears to be an unique ID
 * The second block appears to be some kind of identifier which always is 0xAA (10101010) 
 * Converting the pulses into bits results in a 13 bit unique address and a 4 bit identifier: 
 *
 * B) 1110000110011 0000  => 1C33 0      B)lack push button
 * W) 1101110110100 0000  => 1BB4 0      W)hite push button
 *
 * Note: The transmitter sends 43 times the same packet when the bell button is pressed
 * the retransmit is killed to prevent reporting the same press multiple times
 *
 * Sample:
 * B) 20;62;DEBUG;Pulses=36;Pulses(uSec)=1000,1000,225,1000,225,1000,225,300,900,300,900,300,900,300,900,1000,225,1000,225,300,925,300,900,1000,225,1000,225,275,900,300,900,300,900,300,900;
 * W) 20;A2;DEBUG;Pulses=36;Pulses(uSec)=325,950,250,950,250,250,925,950,250,950,250,950,250,275,925,950,250,950,250,250,925,950,250,275,925,250,925,275,925,250,925,275,925,275,925;
 \*********************************************************************************************/
#define PLUGIN_ID 70
#define PLUGIN_NAME "SelectPlus"
#define PLUGIN_070_EVENT    "SelectPlus"
#define PLUGIN_070_COMMAND  "SelectPlusSend"

#define SELECTPLUS_PULSECOUNT 36

void SelectPlus_Send(unsigned long address);

boolean Plugin_070(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef PLUGIN_070_CORE
  case PLUGIN_RAWSIGNAL_IN:
    {
      if (RawSignal.Number !=SELECTPLUS_PULSECOUNT) return false; 
      byte basevar=0;
      unsigned long bitstream1=0;                   
      byte checksum=0;
      char buffer[11]=""; 
      //==================================================================================
      // get bytes 
      for(byte x=2;x<RawSignal.Number;x=x+2) {
         if (RawSignal.Pulses[x]*RawSignal.Multiply < 750) {
            bitstream1 = (bitstream1 << 1);
         } else {
            if (RawSignal.Pulses[x]*RawSignal.Multiply > 1200) return false;  // invalid pulse duration 
            bitstream1 = (bitstream1 << 1) | 0x1; 
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
      // all bytes received, make sure checksum is okay
      //==================================================================================
      checksum = (bitstream1)&0xf;                  // Second block 
      if (checksum != 0x0) {                        // last 4 bits should always be 0
         return false; 
         //Serial.print("crc error");
      }
      if (bitstream1 == 0) return false;            // sanity check
      //==================================================================================
      // Output
      // ----------------------------------
      sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
      Serial.print( buffer );
      // ----------------------------------
      Serial.print("SelectPlus;");                  // Label
      sprintf(buffer, "ID=%04x;",((bitstream1)>>4)&0xffff ); // ID    
      Serial.print( buffer );
      Serial.print("SWITCH=1;CMD=ON;");  
      Serial.print("CHIME=01;");
      Serial.println();
      //RepeatingTimer=millis()+SIGNAL_REPEAT_TIME+300;
      //==================================================================================
      //event->Par2          = bitstream1 & 0xFF;
      //event->Par1         |= (bitstream1 >> 8) & 0xFF;
      //event->SourceUnit    = 0;                     // Komt niet van een Nodo unit af.
      //event->Type          = NODO_TYPE_PLUGIN_EVENT;
      //event->Command       = 70;                    // nummer van deze plugin
      RawSignal.Repeats=true;                       // suppress repeats of the same RF packet
      RawSignal.Number=0;                           // do not process the packet any further
      success = true;                               // processing successful
      break;
    }
    case PLUGIN_COMMAND:
      {
      event->Port=VALUE_ALL;                        // Signaal mag naar alle door de gebruiker met [Output] ingestelde poorten worden verzonden.
      unsigned long address = 0;
      address = (event->Par2) << 4;                 // add 4 verification zeros to the address
      //Serial.println( address );
      SelectPlus_Send(address);                     // Send RF packet
      success=true;
      break;
    }
#endif // PLUGIN_070_CORE

    #if NODO_MEGA
    case PLUGIN_MMI_IN:
      {
        // Reserveer een kleine string en kap voor de zekerheid de inputstring af om te voorkomen dat er
        // buiten gereserveerde geheugen gewerkt wordt.
        char *str=(char*)malloc(INPUT_COMMAND_SIZE);

        // Hier aangekomen bevat string het volledige commando. Test als eerste of het opgegeven commando overeen komt met "SelectPlusSend"
        // Dit is het eerste argument in het commando.
        if(GetArgv(string,str,1)) {
            event->Type  = 0;
            if(strcasecmp(str,PLUGIN_070_COMMAND)==0) {
                event->Type  = NODO_TYPE_PLUGIN_COMMAND;
            }
            if(event->Type) {
                event->Command = 70;                // Plugin nummer  
                if(GetArgv(string,str,2)) {
                   event->Par2=str2int(str); 
                   success=true;
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
          
void SelectPlus_Send(unsigned long address) {
    int fpulse = 364;                               // Pulse witdh in microseconds
    int fretrans = 16;                              // number of RF packet retransmissions        
    uint32_t fdatabit;
    uint32_t fdatamask = 0x10000;
    uint32_t fsendbuff;

    digitalWrite(PIN_RF_RX_VCC,LOW);                // Power off the RF receiver (if wired that way) to protect against interference
    digitalWrite(PIN_RF_TX_VCC,HIGH);               // Enable 433Mhz transmitter
    delay(TRANSMITTER_STABLE_TIME);                 // Short pause to wait for stability of the transmitter

    for (int nRepeat = 0; nRepeat <= fretrans; nRepeat++) {
        fsendbuff=address;  
        
        // send SYNC 3P High
        digitalWrite(PIN_RF_TX_DATA, HIGH);
        delayMicroseconds(fpulse * 3);
        // end send SYNC
        // Send command
        for (int i = 0; i < 17;i++) {               // SelectPlus address is only 13 bits, last 4 bits are always zero
            // read data bit7
            fdatabit = fsendbuff & fdatamask;       // Get most left bit
            fsendbuff = (fsendbuff << 1);           // Shift left

            if (fdatabit != fdatamask) {            // Write 0
                digitalWrite(PIN_RF_TX_DATA, LOW);           // short low
                delayMicroseconds(fpulse * 1);
                digitalWrite(PIN_RF_TX_DATA, HIGH);          // long high
                delayMicroseconds(fpulse * 3);
            } else {                                // Write 1
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse * 3);      // long low
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse * 1);      // short high
            }
        }
        digitalWrite(PIN_RF_TX_DATA, LOW);                   // and lower the signal
        if (nRepeat < fretrans) {
            delayMicroseconds(fpulse * 16);         // delay between RF transmits
        }

    }
    delay(TRANSMITTER_STABLE_TIME);                 // Short pause to clear the air after the last bit
    digitalWrite(PIN_RF_TX_VCC,LOW);                // Disable the 433Mhz transmitter
    digitalWrite(PIN_RF_RX_VCC,HIGH);               // Enable the 433Mhz receiver
}
