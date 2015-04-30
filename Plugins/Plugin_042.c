//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                      Plugin-42 UPM/Esic                                           ##
//#######################################################################################################
/*********************************************************************************************\
 * Dit protocol zorgt voor ontvangst van UPM/Esic weerstation buitensensoren 
 * Tevens alle sensoren die het UPM/Esic (W.H. Mandolyn International Ltd) protocol volgen waaronder:
 * UPM, Esic, Emos, DVM, Clas Ohlson, Dickson 
 * WT260,WT260H,WT440H,WT450,WT450H,WDS500,RG700
 *
 * Author             : StuntTeam
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical information:
 * Decodes signals from a UPM/Esic Weatherstation outdoor unit, (52/54 pulses, 36 bits, 433 MHz).
 * Supports two packet formats
 * --------------------------------------------------------------------------------------------
 * FORMAT 1:
 *
 * ____Byte 0_____  ____Byte 1_____  ____Byte 2_____  ____Byte 3_____  _Nib4__
 * 7 6 5 4 3 2 1 0  7 6 5 4 3 2 1 0  7 6 5 4 3 2 1 0  7 6 5 4 3 2 1 0  3 2 1 0
 * x x x x c c c c  d d y y b S S S  s s s s s P P P  p p p p p p p p  z z C C
 *
 * x = Constant, 1100, probably preamble
 * c = House Code (0 - 15)
 * d = Device Code (1 - 4) ?
 * y = ?
 * b = Low battery indication
 * S = Secondary value - Humidity/Wind direction (high bits)
 * s = Secondary value - Humidity/Wind direction (low bits)
 * P = Primary value - Temperature/Rain/Wind speed value (high bits)
 * p = Primary value - Temperature/Rain/Wind speed value (low bits)
 * z = Sequence number 0 - 2. Messages are sent in bursts of 3. For some senders this is always 0
 * C = Checksum. bit 1 is XOR of odd bits, bit 0 XOR of even bits in message
 * 
 * If HouseCode = 10 and deviceCode = 2, then p and P is Wind speed
 * and h and H is Wind direction
 * 
 * If HouseCode = 10 and deviceCode = 3, then p and P is rain
 *
 * Temp (C) = RawValue / 16 - 50
 * Rain (total mm) = RawValue * 0,7
 * Wind Speed (mph)= RawValue (* 1/3,6 for km/h)
 * Humidity (%) = RawValue / 2
 * Wind direction (deg) = RawValue * 22,5
 * --------------------------------------------------------------------------------------------
 * FORMAT 2:
 *
 * ____Byte 0_____  ____Byte 1_____  ____Byte 2_____  ____Byte 3_____  _Nib4__
 * 7 6 5 4 3 2 1 0  7 6 5 4 3 2 1 0  7 6 5 4 3 2 1 0  7 6 5 4 3 2 1 0  3 2 1 0
 * x x x x c c c c  d d y y b h h h  h h h h T T T T  T T T T t t t t  t t t p
 *
 * x = Constant, 1100, probably preamble
 * c = House Code (0 - 15)
 * d = Device (Channel) Code (1 - 4) ?
 * y = ? 
 * b = ? 
 * h = Humidity (7 bits) (0111011 = 59 %) 
 * T = Temperature (8 bits) (see below) 
 * t = Temperature (7 bits) (see below)       
 * p = Parity (xor of all bits should give 0)
 *
 * The temperature is transmitted as (temp + 50.0) * 128, which equals (temp * 128) + 6400. Adding 50.0 °C makes
 * all values positive, an unsigned 15 bit integer where the first 8 bits correspond to the whole part of the temperature
 * (here 01001001, decimal 73, substract 50 = 23). Remaining 7 bits correspond to the fractional part.
 * Sample:
 * 20;64;DEBUG;Pulses=52;Pulses(uSec)=875,875,825,875,1725,1800,1725,1800,1725,850,825,1800,1725,875,800,850,825,1800,1725,1800,800,875,800,850,1725,1800,825,850,1725,850,825,1800,1725,1800,825,850,1725,875,800,1800,800,875,800,850,825,850,1725,1800,1750,1800,475;
 * 11000001 00110000 11100100 01100000 0010
 \*********************************************************************************************/
#define PLUGIN_ID 42
#define PLUGIN_NAME "UPM"

#define UPM_MIN_PULSECOUNT 48
#define UPM_MAX_PULSECOUNT 56

boolean Plugin_042(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef PLUGIN_042_CORE
  case PLUGIN_RAWSIGNAL_IN:
    {
      if (RawSignal.Number < UPM_MIN_PULSECOUNT || RawSignal.Number > UPM_MAX_PULSECOUNT) return false; 

      byte rc=0;
      byte basevar=0;
      unsigned long bitstream1=0;                   // holds first 10 bits 
      unsigned long bitstream2=0;                   // holds last 26 bits

      int temperature=0;
      int humidity=0;
      int rain=0;
      int winds=0;
      int windd=0;
      byte battery=0;
      byte units=0;
      byte devicecode=0;
      byte checksum=0;
      byte bitcounter=0;                            // counts number of received bits (converted from pulses)
      byte halfbit=0;                               // high pulse = 1, 2 low pulses = 0, halfbit keeps track of low pulses
      byte msgformat=0;
      char buffer[14]=""; 
      //==================================================================================
      for(byte x=1;x <RawSignal.Number;x++) {       // get bytes
         if (RawSignal.Pulses[x]*RawSignal.Multiply > 1500 && RawSignal.Pulses[x]*RawSignal.Multiply < 2000) {
            if (halfbit==1) {                       // UPM cant receive a 1 bit after a single low value
               return false;                        // pulse error, must not be a UPM packet or reception error
            }
            if (bitcounter < 10) {
                bitstream1 = (bitstream1 << 1);
                bitcounter++;                       // only need to count the first 10 bits
            } else {
                bitstream2 = (bitstream2 << 1);
            }
            halfbit=0;                              // wait for next first low or high pulse     
         } else {
            if (RawSignal.Pulses[x]*RawSignal.Multiply > 1000 && RawSignal.Pulses[x]*RawSignal.Multiply < 500) return false; // Not a valid UPM pulse length
            if (halfbit == 0) {                     // 2 times a low value = 0 bit
               halfbit=1;                           // first half received   
            } else {
               if (bitcounter < 10) {
                  bitstream1 = (bitstream1 << 1) | 0x1; 
                  bitcounter++;                     // only need to count the first 10 bits
               } else {
                  bitstream2 = (bitstream2 << 1) | 0x1; 
               }
               halfbit=0;                           // wait for next first low or high pulse     
            }
         }
      }
      //==================================================================================
      // first perform a check to make sure the packet is a valid UPM/Esic packet 
      // by comparing the first 4 bits which are always? 1100
      if ( (bitstream1 >> 6 ) != 0x0c ) return false;  
      //==================================================================================
      // perform a checksum check to make sure the packet is a valid UPM/Esic packet
      // Checksum - xor all odd and all even bits should match the last two bits
      for (byte i=0;i<9;i=i+2){ 
          checksum=checksum ^ ((bitstream1 >> i) &3);
      }
      for (byte i=2;i<25;i=i+2){ 
          checksum=checksum ^ ((bitstream2 >> i) &3);
      }
      if (checksum == (bitstream2 &3 )) {   // did the format 1 checksum calculation match?
         msgformat=1;                       // Yes, set it
      } else {                              // else perform a bit parity check to see if we have format 2
         checksum=checksum ^ ((bitstream2) &3); // xor the last 2 bits
         units = (checksum >> 1) & 0x01;    // get the odd bit of the checksum result
         checksum=(checksum & 0x01) ^ units;// xor the odd with the even bit of the checksum result
         if (checksum == 0) {               // did the format 2 parity checksum calculation match?
             msgformat=2;
         } else {
             return false;
         }
      }
      //==================================================================================
      // now process the various sensor types      
      //==================================================================================
      // housecode
      units=bitstream1 & 0x03;              // format 1&2
      rc=units;                             // format 1&2
      // devicecode
      devicecode=(bitstream1 >> 2) & 0x0f;  // format 1&2
      if (msgformat==1) {
         battery = (bitstream2 >> 23) & 1;     // battery state 1=low 0=ok
         if (rc==10 && devicecode==2) {               // wind
            units = (bitstream2 >> 4) & 0x0f;    
            winds = (bitstream2 >> 8) & 0x7f;
            windd = (bitstream2 >> 15) & 0xff;        // wind direction
            windd = windd * 225;
            //==================================================================================
            // Output
            // ----------------------------------
            sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
            Serial.print( buffer );
            Serial.print("UPM/Esic;");                       // Label
            sprintf(buffer, "ID=%02x%02x;", rc, devicecode); // ID    
            Serial.print( buffer );
            sprintf(buffer, "WINSP=%02x;", winds);     
            Serial.print( buffer );
            sprintf(buffer, "WINDIR=%04x;", windd);     
            Serial.print( buffer );
            if (battery==1) {                                // battery status
               Serial.print("BAT=LOW;");               
            } else {
               Serial.print("BAT=OK;");                
            }
            Serial.println();
            //==================================================================================
         } else
         if (rc==10 && devicecode==3) {               // rain
            units = (bitstream2 >> 4) & 0x0f;         
            rain = (bitstream2 >> 8) & 0x7f;
            rain = rain * 7;                        // Serial.print( (float)rain * 0.7 );
            //==================================================================================
            // Output
            // ----------------------------------
            sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
            Serial.print( buffer );
            Serial.print("UPM/Esic;");              // Label
            sprintf(buffer, "ID=%02x%02x;", rc, devicecode); // ID    
            Serial.print( buffer );
            sprintf(buffer, "RAIN=%04x;", rain);     
            Serial.print( buffer );
            if (battery==1) {                       // battery status
               Serial.print("BAT=LOW;");               
            } else {
               Serial.print("BAT=OK;");                
            }
            Serial.println();
            //==================================================================================
         } else {                                   // temperature & Humidity
            units = (bitstream2 >> 4) & 0x0f;       // temperature
            temperature = (bitstream2 >> 8) & 0x7f;
            temperature = temperature-50;
            temperature = (temperature*10) + units;
            humidity = (bitstream2 >> 15) & 0xff;   // humidity
            humidity = humidity / 2;
            //if (humidity==0) return false;            // dont accept Bad humidity status
            //if (temperature > 1000) return false;     // dont accept bad temperature
            //==================================================================================
            // Output
            // ----------------------------------
            sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
            Serial.print( buffer );
            Serial.print("UPM/Esic;");              // Label
            sprintf(buffer, "ID=%02x%02x;", rc, devicecode); // ID    
            Serial.print( buffer );
            sprintf(buffer, "TEMP=%04x;", temperature);     
            Serial.print( buffer );
            sprintf(buffer, "HUM=%02d;", humidity); // Humidity 0x15 = 21% decimal 
            Serial.print( buffer );
            if (battery==1) {                       // battery status
               Serial.print("BAT=LOW;");               
            } else {
               Serial.print("BAT=OK;");                
            }
            Serial.println();
            //==================================================================================
         }
      } else {
         units = (bitstream2 >> 1) & 0x7f;          // temperature
         temperature = (bitstream2 >> 8) & 0xff;
         temperature = temperature-50;
         temperature = (temperature*100) + units;
         temperature = temperature/10;
         humidity = (bitstream2 >> 16) & 0x7f;      // humidity
         //if (humidity==0) return false;             // dont accept Bad humidity status
         //if (temperature > 1000) return false;      // dont accept bad temperature
         //==================================================================================
         // Output
         // ----------------------------------
         sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
         Serial.print( buffer );
         Serial.print("UPM/Esic F2;");                    // Label
         sprintf(buffer, "ID=%02x%02x;", rc, devicecode); // ID    
         Serial.print( buffer );
         sprintf(buffer, "TEMP=%04x;", temperature);     
         Serial.print( buffer );
         sprintf(buffer, "HUM=%02x;", humidity);     
         Serial.print( buffer );
         Serial.println();
         //==================================================================================
      }
      //==================================================================================
      RawSignal.Repeats=true;                    // suppress repeats of the same RF packet
      RawSignal.Number=0;
      success = true;
      break;
    }
#endif // PLUGIN_042_CORE
  }      
  return success;
}
