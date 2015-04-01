//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                 Plugin-072 Byron Wireless Doorbell                                ##
//#######################################################################################################
/*********************************************************************************************\
 * This Plugin takes care of reception And sending of the Byron SX doorbell
 *
 * Author             : Maurice Ruiter (Dodge)
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical information:
 *
 * Complete signal:
 * 101010101010100110101001
 * ID               Ringtone
 * 1010101010101001|10101001
 *
 * Converting from pulses to bits taking every second pulse as bit value:
 * 1010101001101010
 *  0 0 0 0 1 0 0 0| 0 0 0 1   =>  ID=0x08 Tone=1
 * ---------------------------------------------------------
 * PULSEVALUES:
 * -----------
 * ID's:                            Ringtones:
 * 1010101001101010                 10101001   169   aa6a 43626
 * 1010101010011001                 10100110   166
 * 0101101010101010                 10010110   150
 * 1001011001010101                 01010110   86
 * 0101101010101010                 01101001   105
 * 0110101001101001                 01011001   89
 * 1001101010101001                 01011010   90
 * 1010011001101010                 01100110   102
 *      ^^^^
 * 0123456789012345                 67890123
 * ---------------------------------------------------------
 * BITVALUES:
 * ---------
 * ID's:                         Ringtones:                       ?  ?
 * 00001000      0x08            0001   0x01     000(1) = 0   
 * 00000101      0x05            0010   0x02     001(0) = 1
 * 11000000      0xC0            0110   0x06     011(0) = 3 
 * 01101111      0x6F            1110   0x0E     111(0) = 7
 * 00000001      0x01            1001   0x09     100(1) = 4
 * 10001001      0x89            1101   0x0D     110(1) = 6
 * 01000001      0x41            1100   0x0C     110(0) = 6
 * 00101000      0x28            1010   0x0A     101(0) = 5
 *    ^
 * 01234567                      8901
 \*********************************************************************************************/
#define PLUGIN_ID 72
#define PLUGIN_NAME "Byron"
#define PLUGIN_072_EVENT        "Byron"
#define PLUGIN_072_COMMAND  "ByronSend"
#define BYRON_PULSECOUNT 26

#define BYRONSTART                 100
#define BYRONSPACE                 250
#define BYRONLOW                   350
#define BYRONHIGH                  675

boolean Plugin_072(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef PLUGIN_072_CORE
  case PLUGIN_RAWSIGNAL_IN:
    {
      //==================================================================================
      byte basevar=0;
      unsigned long bitstream0=0;                     // holds first 16 bits
      unsigned long bitstream1=0;                     // holds the 4 bits for checksum
      unsigned long bitstream2=0;                     // holds last 8 bits
      byte checksum=0;
      byte bitcounter=0;                              // counts number of received bits (converted from pulses)
      char buffer[14]=""; 

      if (RawSignal.Number !=BYRON_PULSECOUNT) return false;
      //==================================================================================
      // get bytes 
      if (RawSignal.Pulses[0] != PLUGIN_ID) return false; // only accept plugin1 translated packets
      for(byte x=0;x<RawSignal.Number-1;x++) {
         if (RawSignal.Pulses[x]*RawSignal.Multiply < 425) {
            if ((bitcounter < 9) && (bitcounter > 4)) {
               bitstream0 = (bitstream0 << 1);        // only need from bit 5 to 8 to do the checksum
               bitstream1 = (bitstream1 << 1);
               bitcounter++;
            } else if (bitcounter < 17) {
               bitstream1 = (bitstream1 << 1);
               bitcounter++;                     
            } else {
               bitstream2 = (bitstream2 << 1);        // this should give the ringtone
            }
         } else {
            if ((bitcounter < 9) && (bitcounter > 4)) {
               bitstream0 = (bitstream0 << 1) | 0x1;  // only need from bit 5 to 8 to do the checksum
               bitstream1 = (bitstream1 << 1) | 0x1;
               bitcounter++;
            } else if (bitcounter < 17) {
               bitstream1 = (bitstream1 << 1) | 0x1;
               bitcounter++;
            } else {
               bitstream2 = (bitstream2 << 1) | 0x1;  // this should give the ringtone
            }
         }
      }
      //==================================================================================
      // all bytes received, make sure checksum is okay
      //==================================================================================
      checksum = bitstream0;                          // Second block
      if ((checksum != 0xA) && (checksum != 0x6))  return false; // Should be '1010' => 0xA or '0110' => 0x6
      //==================================================================================
      // Serial.print(" CRC=");
      // Serial.print(bitstream0,HEX);
      // Serial.print(" ID=");
      // Serial.print(bitstream1,HEX);
      // Serial.print(" Ring=");
      // Serial.println(bitstream2,HEX);
      // Serial.print(bitstream1,BIN);
      // Serial.println(bitstream2,BIN);
      //==================================================================================
      // Output
      // ----------------------------------
      sprintf(buffer, "20;%02X;", PKSequenceNumber++);// Node and packet number 
      Serial.print( buffer );
      // ----------------------------------
      Serial.print("Byron SX;");                         // Label
      sprintf(buffer, "ID=%04x;", bitstream1);        // ID      
      Serial.print( buffer );
      Serial.print("SWITCH=1;CMD=ON;");  
      sprintf(buffer, "CHIME=%02x;", bitstream2);     // chime number
      Serial.print( buffer );
      Serial.println();
      //==================================================================================
      //event->Par1=bitstream2;
      //event->Par2=bitstream1;
      //event->SourceUnit    = 0;                     // Komt niet van een Nodo unit af, dus unit op nul zetten
      //event->Port          = VALUE_SOURCE_RF;
      //event->Type          = NODO_TYPE_PLUGIN_EVENT;
      //event->Command       = 72; // Nummer van dit device
      //==================================================================================
      RawSignal.Repeats=true;                         // suppress RF signal repeats
      RawSignal.Number=0;
      success=true;
      break;
    }      //einde ontvangen
    
   case PLUGIN_COMMAND:
    {
        unsigned long bitstream1=event->Par2;       // address
        unsigned long bitstream=event->Par1;        // ringtone
        
        RawSignal.Multiply=50;
        RawSignal.Repeats=30;
        RawSignal.Delay=20;
        RawSignal.Pulses[1]=BYRONLOW/RawSignal.Multiply;
        for (byte x=17;x>=2;x=x-1) {
            if ((bitstream1 & 1) == 1) 
               RawSignal.Pulses[x] = BYRONHIGH/RawSignal.Multiply;
            else 
               RawSignal.Pulses[x] = BYRONLOW/RawSignal.Multiply;
            bitstream1 = bitstream1 >> 1;
        }
        for (byte x=25;x>=18;x=x-1) {
            if ((bitstream & 1) == 1) 
               RawSignal.Pulses[x] = BYRONHIGH/RawSignal.Multiply;
            else 
               RawSignal.Pulses[x] = BYRONLOW/RawSignal.Multiply;
            bitstream = bitstream >> 1;
        }
        RawSignal.Pulses[26]=BYRONSTART/RawSignal.Multiply;
        RawSignal.Number=26;
        RawSendRF();
        success=true;
        break;
    }
#endif // PLUGIN_072_CORE

#if NODO_MEGA
  case PLUGIN_MMI_IN:
      {
        char *str=(char*)malloc(INPUT_COMMAND_SIZE);
        if(GetArgv(string,str,1)) {
            event->Type  = 0;
            if(strcasecmp(str,PLUGIN_072_COMMAND)==0) {
                event->Type  = NODO_TYPE_PLUGIN_COMMAND;
            }
            if(event->Type) {
                event->Command = 72;                // Plugin nummer  
                if(GetArgv(string,str,2)) {         // Het door de gebruiker ingegeven eerste parameter bevat het adres
                   event->Par2=str2int(str); 
                   if(GetArgv(string,str,3)) {      // Het door de gebruiker ingegeven tweede parameter bevat het ringtone nummer
                     event->Par1=str2int(str); 
                     success=true;
                   }
                }
            }
        }
        free(str);
        break;
      }
#endif //NODO_MEGA
  }      
  return success;
}
