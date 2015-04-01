//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                        Plugin-30 AlectoV1                                         ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of decoding the protocol used for outdoor sensors of the Alecto weather stations 
 * following protocol version 1
 * This Plugin works at least with: Alecto WS3500, Silvercrest, Otio sht-10, Otio sht-20
 *                                  Auriol H13726, Ventus WS155, Hama EWS 1500, Meteoscan W155/W160 
 *                                  Alecto WS4500, Ventus W044, Balance RF-WS105 
 * 
 * Author             : StuntTeam
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Technische informatie:
 * Message Format: (9 nibbles, 36 bits):
 *
 * Format for Temperature Humidity
 *   AAAAAAAA BBBB CCCC CCCC CCCC DDDDDDDD EEEE
 *   RC       Type Temperature___ Humidity Checksum
 *   A = Rolling Code (includes channel number on low 2 bits of nibble1 (10=ch1 01=ch2 11=ch3) )
 *   B = Message type (xyyx = temp/humidity if yy <> '11')
 *       4 bits: bit 0   = battery state 0=OK, 1= below 2.6 volt       
 *               bit 1&2 = 00/01/10 = temp/hum is transmitted, 11=non temp is transmitted 
 *               bit 3   = 0=scheduled transmission, 1=requested transmission (button press)
 *   C = Temperature (two's complement)
 *   D = Humidity BCD format
 *   E = Checksum
 *
 * Format for Rain
 *   AAAAAAAA BBBB CCCC DDDD DDDD DDDD DDDD EEEE
 *   RC       Type      Rain                Checksum
 *   A = Rolling Code
 *   B = Message type (xyyx = NON temp/humidity data if yy = '11')
 *   C = fixed to 1100
 *   D = Rain (bitvalue * 0.25 mm)
 *   E = Checksum
 *
 * Format for Windspeed
 *   AAAAAAAA BBBB CCCC CCCC CCCC DDDDDDDD EEEE
 *   RC       Type                Windspd  Checksum
 *   A = Rolling Code
 *   B = Message type (xyyx = NON temp/humidity data if yy = '11')
 *   C = Fixed to 1000 0000 0000
 *   D = Windspeed  (bitvalue * 0.2 m/s, correction for webapp = 3600/1000 * 0.2 * 100 = 72)
 *   E = Checksum
 *
 * Format for Winddirection & Windgust
 *   AAAAAAAA BBBB CCCD DDDD DDDD EEEEEEEE FFFF
 *   RC       Type      Winddir   Windgust Checksum
 *   A = Rolling Code
 *   B = Message type (xyyx = NON temp/humidity data if yy = '11')
 *   C = Fixed to 111
 *   D = Wind direction
 *   E = Windgust (bitvalue * 0.2 m/s, correction for webapp = 3600/1000 * 0.2 * 100 = 72)
 *   F = Checksum
 *
 * Sample:
 * 20;53;DEBUG;Pulses=74;Pulses(uSec)=425,3800,350,1825,350,1825,325,1825,350,1825,325,3800,350,3800,350,1825,325,3800,350,1825,325,1800,350,1825,350,1825,325,1825,325,3800,325,1825,350,1800,350,1825,325,3825,325,3800,325,1825,325,1825,325,1800,325,1825,350,3800,325,1825,325,3800,350,1800,350,1800,350,3800,350,1825,325,1825,325,1825,325,1825,350,1825,325,1925,325;
 \*********************************************************************************************/
#define PLUGIN_ID 30
#define PLUGIN_NAME "AlectoV1"
#define WS3500_PULSECOUNT 74

byte Plugin_030_ProtocolAlectoCheckID(byte checkID);

//unsigned int Plugin_030_ProtocolAlectoRainBase=0;

boolean Plugin_030(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef PLUGIN_030_CORE
  case PLUGIN_RAWSIGNAL_IN:
    {
      if (RawSignal.Number != WS3500_PULSECOUNT) return false;

      RawSignal.Repeats    = false;                                            // het is een herhalend signaal. Bij ontvangst herhalingen NIET onderdrukken.

      unsigned long bitstream=0;
      byte nibble0=0;
      byte nibble1=0;
      byte nibble2=0;
      byte nibble3=0;
      byte nibble4=0;
      byte nibble5=0;
      byte nibble6=0;
      byte nibble7=0;
      byte checksum=0;
      int temperature=0;
      byte humidity=0;
      unsigned int rain=0;
      byte windspeed=0;
      byte windgust=0;
      int winddirection=0;
      byte checksumcalc = 0;
      byte rc=0;
      byte basevar=0;
      char buffer[11]=""; 

      for(byte x=2; x<=64; x=x+2) {
        if(RawSignal.Pulses[x]*RawSignal.Multiply > 2560) bitstream = ((bitstream >> 1) |(0x1L << 31)); 
        else bitstream = (bitstream >> 1);
      }

      for(byte x=66; x<=72; x=x+2) {
        if(RawSignal.Pulses[x]*RawSignal.Multiply > 2560) checksum = ((checksum >> 1) |(0x1L << 3)); 
        else checksum = (checksum >> 1);
      }

      nibble7 = (bitstream >> 28) & 0xf;
      nibble6 = (bitstream >> 24) & 0xf;
      nibble5 = (bitstream >> 20) & 0xf;
      nibble4 = (bitstream >> 16) & 0xf;
      nibble3 = (bitstream >> 12) & 0xf;
      nibble2 = (bitstream >> 8) & 0xf;
      nibble1 = (bitstream >> 4) & 0xf;
      nibble0 = bitstream & 0xf;

      // checksum calculations
      if ((nibble2 & 0x6) != 6) {
        checksumcalc = (0xf - nibble0 - nibble1 - nibble2 - nibble3 - nibble4 - nibble5 - nibble6 - nibble7) & 0xf;
      }
      else
        {
        // Alecto checksums are Rollover Checksums by design!
        if (nibble3 == 3)
          checksumcalc = (0x7 + nibble0 + nibble1 + nibble2 + nibble3 + nibble4 + nibble5 + nibble6 + nibble7) & 0xf;
        else
          checksumcalc = (0xf - nibble0 - nibble1 - nibble2 - nibble3 - nibble4 - nibble5 - nibble6 - nibble7) & 0xf;
        }

      if (checksum != checksumcalc) return false;
      rc = bitstream & 0xff;

      if ((nibble2 & 0x6) != 6) {
        temperature = (bitstream >> 12) & 0xfff;
        //fix 12 bit signed number conversion
        if ((temperature & 0x800) == 0x800) temperature = temperature - 0x1000;
        humidity = (16 * nibble7) + nibble6;
        //==================================================================================
        // Output
        // ----------------------------------
        sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
        Serial.print( buffer );
        // ----------------------------------
        Serial.print("Alecto V1;");                      // Label
        sprintf(buffer, "ID=%02x%02x;", (rc &0xfc), (rc &0x03) ); // ID split into rolling code and channel number
        Serial.print( buffer );
        sprintf(buffer, "TEMP=%04x;", temperature);     
        Serial.print( buffer );
        sprintf(buffer, "HUM=%02x;", humidity);     
        Serial.print( buffer );
        Serial.println();
        //==================================================================================
        RawSignal.Repeats=true;                          // suppress repeats of the same RF packet
        RawSignal.Number=0;
        return true;
      } else {
        // rain
        if (nibble3 == 3) {
          rain = ((bitstream >> 16) & 0xffff);
          //==================================================================================
          // Output
          // ----------------------------------
          sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
          Serial.print( buffer );
          // ----------------------------------
          Serial.print("Alecto V1;");                    // Label
          sprintf(buffer, "ID=00%02x;", rc);             // ID    
          Serial.print( buffer );
          sprintf(buffer, "RAIN=%04x;", rain);     
          Serial.print( buffer );
          Serial.println();
          //==================================================================================
          RawSignal.Repeats=true;                        // suppress repeats of the same RF packet
          RawSignal.Number=0;
          return true;
        }
        // windspeed
        if (nibble3 == 1) {
          windspeed = ((bitstream >> 24) & 0xff);
          windspeed = windspeed*72;
          //==================================================================================
          // Output
          // ----------------------------------
          sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
          Serial.print( buffer );
          // ----------------------------------
          Serial.print("Alecto V1;");                // Label
          sprintf(buffer, "ID=00%02x;", rc);         // ID    
          Serial.print( buffer );
          sprintf(buffer, "WINSP=%04x;", windspeed);     
          Serial.print( buffer );
          Serial.println();
          //==================================================================================
          RawSignal.Repeats=true;                    // suppress repeats of the same RF packet
          RawSignal.Number=0;
          return true;
        }
        // winddir
        if ((nibble3 & 0x7) == 7) {
          winddirection = ((bitstream >> 15) & 0x1ff) / 45;
          windgust = ((bitstream >> 24) & 0xff);
          windgust = windgust*72;
          //==================================================================================
          // Output
          // ----------------------------------
          sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
          Serial.print( buffer );
          // ----------------------------------
          Serial.print("Alecto V1;");                // Label
          sprintf(buffer, "ID=00%02x;", rc);         // ID    
          Serial.print( buffer );
          sprintf(buffer, "WINDIR=%04x;", winddirection);     
          Serial.print( buffer );
          sprintf(buffer, "WINGS=%04x;", windgust);     
          Serial.print( buffer );
          Serial.println();
          //==================================================================================
          RawSignal.Repeats=true;                    // suppress repeats of the same RF packet
          RawSignal.Number=0;
          return true;
        }
      }
      success = true;
      break;
    }
#endif // PLUGIN_030_CORE
  }      
  return success;
}
