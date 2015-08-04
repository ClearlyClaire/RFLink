
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                      Plugin-10 RGB Controller                                     ##
//#######################################################################################################
/*********************************************************************************************\
 * Decodes signals from a wireless RGB controller remote control
 * 
 *
 * Author             : StuntTeam
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical information:
 * Decodes signals from a wireless RGB controller remote control
 * --------------------------------------------------------------------------------------------
 * _Byte 0_  _Byte 1_  _Byte 2_  _Byte 3_  _Bit_
 * 76543210  76543210  76543210  76543210  0
 * AAAAAAAA  BBBBBBBB  CCCCCCCC  DDDDDDDD  E
 *
 * A = Rolling Code
 * B = Rolling Code
 * C = Constant, always 0xFF
 * D = Command
 * E = Checksum. bit is XOR of all bits in the RF message
 * 
 * Commands:
 * 00 ON
 * 01 OFF
 * 02 Dim Down
 * 03 DIM UP
 * 06 Color Mix UP
 * 07 Color Mix Down
 * 19 Color Wheel Red
 * 29 Color Wheel Blue
 * 4D Color Wheel Yellow
 * 74 Color Wheel Green 

 10011011  9b  YELLOW
 00111000  38  RED
 00111000      BLUE
 11010011 D3   green
 
RGB Control:10001101110000101111111100000000 1  20;1B;RGB SWITCH;ID=8dc2;SWITCH=00;CMD=ON;
RGB Control:10001101110000101111111100000000 1  20;1D;RGB SWITCH;ID=8dc2;SWITCH=00;CMD=ON;
RGB Control:10001101110000101111111100010101 0  20;1F;RGB SWITCH;ID=8dc2;SWITCH=15;CMD=
RGB Control:10001101110000101111111111001000 0  20;21;RGB SWITCH;ID=8dc2;SWITCH=c8;CMD=
RGB Control:10001101110000101111111111000000 1  20;23;RGB SWITCH;ID=8dc2;SWITCH=c0;CMD=
RGB Control:10001101110000101111111101000001 1  20;25;RGB SWITCH;ID=8dc2;SWITCH=41;CMD=
 \*********************************************************************************************/
#define RGB_MIN_PULSECOUNT  134
#define RGB_MAX_PULSECOUNT  164

#define RGB_PULSE_STHI      1500/RAWSIGNAL_SAMPLE_RATE
#define RGB_PULSE_STLO      1200/RAWSIGNAL_SAMPLE_RATE
#define RGB_PULSE_HIHI      1000/RAWSIGNAL_SAMPLE_RATE
#define RGB_PULSE_HILO      750/RAWSIGNAL_SAMPLE_RATE
#define RGB_PULSE_LOHI      625/RAWSIGNAL_SAMPLE_RATE
#define RGB_PULSE_LOLO      250/RAWSIGNAL_SAMPLE_RATE


#ifdef PLUGIN_010
boolean Plugin_010(byte function, char *string) {
      if (RawSignal.Number < RGB_MIN_PULSECOUNT || RawSignal.Number > RGB_MAX_PULSECOUNT) return false; 
      unsigned long bitstream=0L;                   // holds first 32 bits 

      byte checksum=0;                              // holds the checksum calculation
      byte crc=0;                                   // holds the crc bit from the signal
      byte bitcounter=0;                            // counts number of received bits (converted from pulses)
      byte halfbit=0;                               // high pulse = 1, 2 low pulses = 0, halfbit keeps track of low pulses
      int command=0;
      byte start_stop=0;
      byte x=1;
      //==================================================================================
      for(x=1;x <RawSignal.Number-2;x++) {          // get bytes
         if (start_stop!=0x01) { 
            //if (RawSignal.Pulses[x]*RawSignal.Multiply > 1200 && RawSignal.Pulses[x]*RawSignal.Multiply < 1500) {
            if (RawSignal.Pulses[x] > RGB_PULSE_STLO && RawSignal.Pulses[x] < RGB_PULSE_STHI) {
               start_stop=0x01;
               continue;
            } else {
               if (x > 100) return false;           // bad packet
               continue;
            } 
         }
         if (RawSignal.Pulses[x]*RawSignal.Multiply > 750 && RawSignal.Pulses[x]*RawSignal.Multiply < 1000) {
            if (halfbit==1) {                       // cant receive a 1 bit after a single low value
               return false;                        // pulse error, must not be a UPM packet or reception error
            }
            if (bitcounter < 32) {
                bitstream = (bitstream << 1) | 0x1;
                bitcounter++;                       // only need to count the first 10 bits
            } else {
                crc =1;
                break;
            }
            halfbit=0;                              // wait for next first low or high pulse     
         } else {
            if (RawSignal.Pulses[x]*RawSignal.Multiply > 625 && RawSignal.Pulses[x]*RawSignal.Multiply < 250) return false; // Not a valid UPM pulse length
            if (halfbit == 0) {                     // 2 times a low value = 0 bit
               halfbit=1;                           // first half received   
            } else {
               if (bitcounter < 32) {
                  bitstream = (bitstream << 1); 
                  checksum=checksum^1;
                  bitcounter++;                     // only need to count the first 10 bits
               } else {
                  crc=0;
                  break;
               }
               halfbit=0;                           // wait for next first low or high pulse     
            }
         }
      }
      //==================================================================================
      // Validity checks
      if (RawSignal.Pulses[x+2]*RawSignal.Multiply < 1200 || RawSignal.Pulses[x+2]*RawSignal.Multiply > 1500) return false;
      //==================================================================================
      // perform a checksum check to make sure the packet is a valid RGB control packet
      // Checksum1: 3rd byte must be 0xFF
      // Checksum2: xor all odd and all even bits should match the last bit
      // ----------------------------------
      if ( (bitstream & 0xff00) != 0xff00 ) {
         //Serial.println("crc1 error");
         return false;  
      }
      // ----------------------------------
      if (checksum != crc) {
         //Serial.println("crc2 error");
         return false;  
      }

      //Serial.print("RGB Control:");
      //Serial.print(bitstream, BIN);
      //Serial.print(" ");
      //Serial.print(checksum,BIN);
      //Serial.println();
      //==================================================================================
      // now process the command      
      //==================================================================================
      command = (bitstream) &0xff;                  // command
      bitstream = (bitstream >> 16) &0xffff;        // rolling code
      //==================================================================================
      // Output
      // ----------------------------------
      sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
      Serial.print( pbuffer );
      Serial.print(F("RGB;"));                       // Label
      sprintf(pbuffer, "ID=%04x;", bitstream);       // ID    
      Serial.print( pbuffer );
      sprintf(pbuffer, "SWITCH=%02x;", command);     
      Serial.print( pbuffer );
      Serial.print(F("CMD="));
      if (command==0x00) Serial.print("ON;");
      else if (command==0x01) Serial.print(F("OFF;"));
      else if (command==0x02) Serial.print(F("DIM DOWN;"));
      else if (command==0x03) Serial.print(F("DIM UP;"));
      else if (command==0x06) Serial.print(F("COLORMIX UP;"));
      else if (command==0x07) Serial.print(F("COLORMIX DOWN;"));
      else if (command==0x19) Serial.print(F("COLOR RED;"));
      else if (command==0x29) Serial.print(F("COLOR BLUE;"));
      else if (command==0x4D) Serial.print(F("COLOR YELLOW;"));
      else if (command==0x74) Serial.print(F("COLOR GREEN;"));
      else {
        sprintf(pbuffer, "SET_LEVEL=%d;", command ); 
        Serial.print( pbuffer );        
      }
      Serial.println();
      //==================================================================================
      RawSignal.Repeats=true;                       // suppress repeats of the same RF packet
      RawSignal.Number=0;
      return true;
}
#endif // PLUGIN_010
