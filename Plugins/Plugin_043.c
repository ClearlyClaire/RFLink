//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                          Plugin-43 LaCrosse                                       ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of decoding LaCrosse  weatherstation outdoor sensors
 * It also works for all non LaCrosse sensors that follow this protocol.
 * Lacrosse TX3-TH  Thermo/Humidity, Lacrosse TX4
 * WS7000-15: Anemometer, WS7000-16: Rain precipitation, WS2500-19: Brightness Luxmeter, WS7000-20: Thermo/Humidity/Barometer
 *
 * Author             : StuntTeam
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical information:
 * Decodes signals from a LaCrosse Weatherstation outdoor unit, (88 pulses, 44 bits, 433 MHz).
 *
 * Temperature sensor (TX3)
 * Each frame is 44 bits long. It is composed of: 
 * • 2 blocks of four bits = 0A (start sequence)
 * • 8 blocks of four bits (data) 
 * • 1 block of four bits (checksum)
 * 
 * The active values of the frames are explained below: 
 * • a = Start sequence
 * • b = Type of measure (0=Thermo E=hygro) 
 * • c = Address of sensor 
 * • d = Parity bit  (c+d+e bits sum is even) 
 * • e = Measure  
 * • f = Checksum (Lower four bits of the sum of all words)
 * Example 
 * 0000 1010 0000 0000 1110 0111 0011 0001 0111 0011 1101 
 * aaaa aaaa bbbb cccc cccd eeee eeee eeee eeee eeee ffff
 * 0    A    0    0    7  0 7    3    1    7    3    D   
 * Checksum: (0 + A + 0 + 0 + E + 7 + 3 + 1 + 7 + 3) and F = D   D 
 * Sample:
 * 20;11;DEBUG;Pulses=88;Pulses(uSec)=1200,875,1125,875,1125,875,1125,900,400,900,1150,875,400,900,1150,875,1125,875,1125,875,1150,875,1150,875,400,900,400,875,375,900,1150,875,1125,875,400,900,1150,875,1125,875,1125,875,400,900,400,875,1125,900,400,875,1150,875,1150,900,1125,875,1150,875,400,900,400,875,400,900,1150,875,400,900,400,875,1125,875,400,900,1150,900,1125,875,1150,875,375,900,400,900,400,900,400;
 * 20;9E;DEBUG;Pulses=88;Pulses(uSec)=1300,925,1225,925,1225,925,1200,925,425,925,1225,925,425,925,1225,925,1225,925,1225,925,1225,925,1225,925,1225,925,425,925,1225,925,1225,925,1225,925,425,925,425,925,1225,925,1225,925,425,925,425,925,425,925,1225,925,425,925,425,925,1225,925,425,925,1225,925,1225,925,1225,925,1225,925,425,925,425,925,425,925,1200,925,425,925,425,925,1225,925,1225,925,425,925,425,925,1225;
 * 20;9F;LaCrosse;ID=0403;TEMP=010c;
 * 20;A1;DEBUG;Pulses=88;Pulses(uSec)=1325,925,1225,925,1225,925,1225,925,425,925,1225,925,425,925,1225,925,425,925,425,925,425,925,1225,925,1225,925,425,925,1225,925,1225,925,1225,925,425,925,425,925,1225,925,1225,925,425,925,1225,925,425,925,1225,925,425,950,425,925,1225,925,1225,925,1225,925,1225,925,1225,925,1225,925,425,925,1225,925,425,925,1200,925,425,925,425,925,1225,925,425,925,1225,925,1225,925,1225;
 * 20;A2;LaCrosse;ID=0403;HUM=56;
 * --------------------------------------------------------------------------------------------
 * Rain Packet:
 * Each frame is 46 bits long. It is composed of: 
 * 10bits of 0 (start sequence) 
 * 7 blocks of four bits separated by a bit 1 to be checked and skipped 
 *
 * The 1st bit of each word is LSB, so we have to reverse the 4 bits of each word. 
 *  Example 
 * 0010 1111 1011 0010 1011 1111 1101   
 * aaaa bbbb ccc1 ccc2 ccc3 dddd eeee 
 *   2    F     B    2    B    F   D   
 *
 * a = sensor type (2=Rain meter)
 * b = sensor address 
 * c = rain data (LSB thus the right order is c3 c2 c1)
 * d = Check Xor : (2 ^ F ^ B ^ 2 ^ B ^ F) = 0
 * e = Check Sum : (const5 + 2 + F + B + 2 + B + F) and F = D   
 *
 * --------------------------------------------------------------------------------------------
 * WS2355  
 * Each packet is 52 bits long. 4 different packet formats are transmitted. They are composed of: 
 * 
 * 1) 0000 1001 01 00 00100010 01111000 01010011 0011 10101100 0001   0+9+4+2+2+7+8+5+3+3+A+C=41 = 1
 * 2) 0000 1001 00 01 00100010 01111000 01010000 1101 10101111 1000   0+9+1+2+2+7+8+5+0+D+A+F=48 = 8
 * 3) 0000 1001 00 10 00100010 01111000 00001000 1100 11110111 1000
 * 4) 0000 1001 01 11 00100010 01111000 00000000 1100 11111111 1101
 *                 AA BBBBBBBB CCCCCCCC DDDDDDDD dddd EEEEEEEE FFFF    
 * 
 * A = packet type  00=TEMP, 01=HUM, 10=RAIN, 11=WIND
 * B = Rolling Code
 * C = Unknown
 * D = 12 bit value depending on the device/packet type
 * E = Unknown
 * F = Checksum
 *
 * TEMP Dd = temperature - 30 degrees offset, 12 bits, 0x533 - 0x300 = 0x233 = 23.3 degrees
 * HUM  D  = humidity value, 8 bits, 0x50 = RH of 50 
 * RAIN D = 
 * WIND d = Wind direction (0-16 in 22.5 degrees steps) D= wind speed
  \*********************************************************************************************/
#define LACROSSE_PULSECOUNT 88

boolean Plugin_043(byte function, char *string) {
  boolean success=false;

#ifdef PLUGIN_043_CORE
      if ( (RawSignal.Number < LACROSSE_PULSECOUNT - 4) || (RawSignal.Number > LACROSSE_PULSECOUNT + 4) ) return false; 
      unsigned long bitstream1=0L;                  // holds first 16 bits 
      unsigned long bitstream2=0L;                  // holds last 28 bits

      int temperature=0;
      int humidity=0;
      byte checksum=0;
      //byte parity=0;
      byte bitcounter=0;                            // counts number of received bits (converted from pulses)
      byte data[10];
      //==================================================================================
      // get bytes 
      for(int x=1;x<RawSignal.Number;x+=2) {
         if (RawSignal.Pulses[x]*RawSignal.Multiply > 750) {
            if (bitcounter < 16) {
               bitstream1 = (bitstream1 << 1);
               bitcounter++;                     // only need to count the first 16 bits
            } else {
               bitstream2 = (bitstream2 << 1);
            }
         } else {
            if (bitcounter < 16) {
               bitstream1 = (bitstream1 << 1) | 0x1; 
               bitcounter++;                     // only need to count the first 16 bits
            } else {
               bitstream2 = (bitstream2 << 1) | 0x1; 
            }
         }
      }
      //==================================================================================
      // all bytes received, make sure checksum is okay
      //==================================================================================
      if (bitstream1 == 0 && bitstream2 == 0) return false;
      data[0] = (bitstream1 >> 12) & 0x0f;  // prepare nibbles from bit stream
      data[1] = (bitstream1 >>  8) & 0x0f;
      data[2] = (bitstream1 >>  4) & 0x0f;
      data[3] = (bitstream1 >>  0) & 0x0f;
      data[4] = (bitstream2 >> 24) & 0x0f;
      data[5] = (bitstream2 >> 20) & 0x0f;
      data[6] = (bitstream2 >> 16) & 0x0f;
      data[7] = (bitstream2 >> 12) & 0x0f;
      data[8] = (bitstream2 >>  8) & 0x0f;
      data[9] = (bitstream2 >>  4) & 0x0f;
      //==================================================================================
      if ( (data[0] != 0x00) || (data[1] != 0x0a) ) return false;
      //parity=data[4] & 1; // no check of parity for now
      // first perform a checksum check to make sure the packet is a valid LaCrosse packet
      for (byte i=0;i<10;i++){ 
          checksum=checksum + data[i];
      }
      checksum=checksum & 0x0f;
      if (checksum != (bitstream2 &0x0f )) return false;
      //==================================================================================
      // Prevent repeating signals from showing up, skips every second packet!
      //==================================================================================
      unsigned long tempval=(data[4])>>1;
      tempval=((tempval)<<16)+((data[3])<<8)+data[2];
      if( (SignalHash!=SignalHashPrevious) || (RepeatingTimer<millis()) || (SignalCRC != tempval)  ){ 
         // not seen this RF packet recently
         SignalCRC=tempval;
      } else {
         return true;         // already seen the RF packet recently, but still want the humidity
      }  
      //==================================================================================
      // Only accept temp and humidity packets for now, we need test data for other packet types
      //if (data[2]!= 0x00 && data[2] != 0x0e) return false;
 	  //==================================================================================
      // now process the various sensor types      
      //==================================================================================
      // Output
      // ----------------------------------
      if (data[2] == 0x00) {
         sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
         Serial.print( pbuffer );
         Serial.print(F("LaCrosse;"));                    // Label

         //sprintf(pbuffer, "ID=%02x%02x;", data[3], (data[4])>>1); // ID    
         //Serial.print( pbuffer );
         data[4]=(data[4])>>1;
         Serial.print(F("ID="));                    // Label
         PrintHex8( data+3,1);
         PrintHex8( data+4,1);

         temperature = data[5]*100;
         temperature = temperature + data[6]*10;
         temperature = temperature + data[7];
         temperature = temperature-500;
         sprintf(pbuffer, ";TEMP=%04x;", temperature);     
         Serial.print( pbuffer );
         RawSignal.Repeats=false; 
      } else
      if (data[2] == 0x0e) {
         humidity=(data[5]*16)+data[6];
         if (data[2]==0x0e && humidity==0) return false;  // humidity should not be 0
         sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
         Serial.print( pbuffer );
         Serial.print(F("LaCrosse;"));                    // Label

         //sprintf(pbuffer, "ID=%02x%02x;", data[3], (data[4])>>1 ); // ID    
         //Serial.print( pbuffer );
         data[4]=(data[4])>>1;
         Serial.print(F("ID="));                    // Label
         PrintHex8( data+3,1);
         PrintHex8( data+4,1);

         sprintf(pbuffer, ";HUM=%02x;", (humidity)&0xff);     
         Serial.print( pbuffer );
         RawSignal.Repeats=true;
      }
      Serial.println();
      //==================================================================================
      RawSignal.Number=0;
      success=true;
#endif // PLUGIN_043_CORE
  return success;
}
