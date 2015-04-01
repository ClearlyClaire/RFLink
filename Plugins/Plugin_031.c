//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                        Plugin-31 AlectoV3                                         ##
//#######################################################################################################
/*********************************************************************************************\
 * Dit protocol zorgt voor ontvangst van Alecto weerstation buitensensoren
 * WS1100, WS1200, WSD-19
 *
 * Author             : StuntTeam
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technische informatie:
 * Decodes signals from Alecto Weatherstation outdoor unit, type 3 (94/126 pulses, 47/63 bits, 433 MHz).
 * WS1100 Message Format: (7 bits preamble, 5 Bytes, 40 bits):
 * AAAAAAA AAAABBBB BBBB__CC CCCCCCCC DDDDDDDD EEEEEEEE
 *                        Temperature Humidity Checksum
 * A = start/unknown, first 8 bits are always 11111111
 * B = Rolling code
 * C = Temperature (10 bit value with -400 base)
 * D = Checksum
 * E = Humidity
 *
 * WS1200 Message Format: (7 bits preamble, 7 Bytes, 56 bits):
 * AAAAAAA AAAABBBB BBBB__CC CCCCCCCC DDDDDDDD DDDDDDDD EEEEEEEE FFFFFFFF 
 *                        Temperature Rain LSB Rain MSB ???????? Checksum
 * A = start/unknown, first 8 bits are always 11111111
 * B = Rolling code
 * C = Temperature (10 bit value with -400 base)
 * D = Rain ( * 0.3 mm)
 * E = ?
 * F = Checksum
 \*********************************************************************************************/
#define PLUGIN_ID 31
#define PLUGIN_NAME "AlectoV3"

#define WS1100_PULSECOUNT 94
#define WS1200_PULSECOUNT 126

uint8_t Plugin_031_ProtocolAlectoCRC8( uint8_t *addr, uint8_t len);
unsigned int Plugin_031_ProtocolAlectoRainBase=0;

boolean Plugin_031(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef PLUGIN_031_CORE
  case PLUGIN_RAWSIGNAL_IN:
    {
      if ((RawSignal.Number != WS1100_PULSECOUNT) && (RawSignal.Number != WS1200_PULSECOUNT)) return false;

      unsigned long bitstream1=0;
      unsigned long bitstream2=0;
      byte rc=0;
      int temperature=0;
      byte humidity=0;
      unsigned int rain=0;
      byte checksum=0;
      byte checksumcalc=0;
      byte data[6];
      char buffer[11]=""; 
      
      // get first 32 relevant bits
      for(byte x=15; x<=77; x=x+2) if(RawSignal.Pulses[x]*RawSignal.Multiply < 0x300) bitstream1 = (bitstream1 << 1) | 0x1; 
      else bitstream1 = (bitstream1 << 1);
      // get second 32 relevant bits
      for(byte x=79; x<=141; x=x+2) if(RawSignal.Pulses[x]*RawSignal.Multiply < 0x300) bitstream2 = (bitstream2 << 1) | 0x1; 
      else bitstream2 = (bitstream2 << 1);

      data[0] = (bitstream1 >> 24) & 0xff;
      data[1] = (bitstream1 >> 16) & 0xff;
      data[2] = (bitstream1 >>  8) & 0xff;
      data[3] = (bitstream1 >>  0) & 0xff;
      data[4] = (bitstream2 >> 24) & 0xff;
      data[5] = (bitstream2 >> 16) & 0xff;
      // ----------------------------------
      if (RawSignal.Number == WS1200_PULSECOUNT) {
        checksum = (bitstream2 >> 8) & 0xff;
        checksumcalc = Plugin_031_ProtocolAlectoCRC8(data, 6);
      } else {
        checksum = (bitstream2 >> 24) & 0xff;
        checksumcalc = Plugin_031_ProtocolAlectoCRC8(data, 4);
      }
      if (checksum != checksumcalc) return false;
      // ----------------------------------
      rc = (bitstream1 >> 20) & 0xff;
      temperature = ((bitstream1 >> 8) & 0x3ff) - 400;
      //==================================================================================
      // Output
      // ----------------------------------
      sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
      Serial.print( buffer );
      // ----------------------------------
      Serial.print("Alecto V3;");                // Label
      sprintf(buffer, "ID=00%02x;", rc);         // ID    
      Serial.print( buffer );
      sprintf(buffer, "TEMP=%04x;", temperature);     
      Serial.print( buffer );

      if (RawSignal.Number == WS1100_PULSECOUNT) {
         humidity = bitstream1 & 0xff;  // alleen op WS1100? 
         sprintf(buffer, "HUM=%02x;", humidity);     
         Serial.print( buffer );
      } else {
         //Serial.write(0x00);   // rain...
         rain = (((bitstream2 >> 24) & 0xff) * 256) + ((bitstream1 >> 0) & 0xff);
         // check if rain unit has been reset!
         if (rain < Plugin_031_ProtocolAlectoRainBase) Plugin_031_ProtocolAlectoRainBase=rain;
         if (Plugin_031_ProtocolAlectoRainBase > 0) {
            //UserVar[basevar+1 -1] += ((float)rain - Plugin_031_ProtocolAlectoRainBase) * 0.30;
            sprintf(buffer, "RAIN=%02x;", (rain)&0xff);     
            Serial.print( buffer );
         }
         Plugin_031_ProtocolAlectoRainBase = rain;
      }
      Serial.println();
      //==================================================================================
      RawSignal.Repeats=true;                          // suppress repeats of the same RF packet
      RawSignal.Number=0;                              // do not process the packet any further
      success = true;                                  // processing successful
      break;
    }
#endif // PLUGIN_031_CORE
  }      
  return success;
}

#ifdef PLUGIN_031_CORE
/*********************************************************************************************\
 * Calculates CRC-8 checksum
 * reference http://lucsmall.com/2012/04/29/weather-station-hacking-part-2/
 *           http://lucsmall.com/2012/04/30/weather-station-hacking-part-3/
 *           https://github.com/lucsmall/WH2-Weather-Sensor-Library-for-Arduino/blob/master/WeatherSensorWH2.cpp
 \*********************************************************************************************/
uint8_t Plugin_031_ProtocolAlectoCRC8( uint8_t *addr, uint8_t len)
{
  uint8_t crc = 0;
  // Indicated changes are from reference CRC-8 function in OneWire library
  while (len--) {
    uint8_t inbyte = *addr++;
    for (uint8_t i = 8; i; i--) {
      uint8_t mix = (crc ^ inbyte) & 0x80; // changed from & 0x01
      crc <<= 1; // changed from right shift
      if (mix) crc ^= 0x31;// changed from 0x8C;
      inbyte <<= 1; // changed from right shift
    }
  }
  return crc;
}
#endif //CORE
