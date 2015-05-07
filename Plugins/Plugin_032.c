//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                        Plugin-32 AlectoV4                                         ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of decoding the protocol used for outdoor sensors of the Alecto weather stations 
 * following protocol version 4
 * This Plugin works at least with: Banggood SKU174397, Sako CH113, Homemart/Onemall FD030 and Blokker (Dake) 1730796 outdoor sensors
 * But probably with many others as the OEM sensor is sold under many brand names
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
 *   AAAAAAAA AAAA ABCC DDDD DDDD DDDD EEEEEEEE
 *
 *   A = Rolling Code
 *   B = 0=scheduled transmission, 1=requested transmission (button press)
 *   C = Channel number (00=ch1 01=ch2 10=ch3)
 *   D = Temperature (two's complement)
 *   E = Humidity BCD format
 *
 * 20;C2;DEBUG;Pulses=76;Pulses(uSec)=325,500,250,1800,375,3650,375,1775,375,3650,375,3650,375,1775,375,3650,375,1800,350,1800,375,3650,375,3650,375,3650,375,3650,375,1775,375,1775,375,1775,375,1775,375,1775,375,1775,375,1775,375,3650,375,3650,375,3650,375,1775,375,3650,375,3650,375,1775,375,1775,375,1775,375,1775,375,1775,375,1775,375,3650,375,3650,375,3650,375,3650,375;
 \*********************************************************************************************/
#define PLUGIN_ID 32
#define PLUGIN_NAME "AlectoV4"

boolean Plugin_032(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef PLUGIN_032_CORE
  case PLUGIN_RAWSIGNAL_IN:
    {
      if (RawSignal.Number < 74 || RawSignal.Number > 76 ) return false;
      unsigned long bitstream=0;
      int temperature=0;
      int humidity=0;
      byte rc=0;
      byte rc2=0;
      
      char buffer[14]=""; 

      //==================================================================================
      byte start=0;
      if (RawSignal.Number == 76) start=2;
      for(byte x=2+start; x<=58; x=x+2) {                     // Get first 28 bits
        if (RawSignal.Pulses[x+1]*RawSignal.Multiply > 550) return false;
        if (RawSignal.Pulses[x]*RawSignal.Multiply > 3000) {
           bitstream = (bitstream << 1) | 0x01;
        } else {
           if (RawSignal.Pulses[x]*RawSignal.Multiply > 1500) {
              bitstream = (bitstream << 1);
           } else {
              return false;
           }              
        }
      }
      for(byte x=60; x<=74; x=x+2) {                          // Get remaining 8 bits
        if (RawSignal.Pulses[x+1]*RawSignal.Multiply > 550) return false;
        if(RawSignal.Pulses[x]*RawSignal.Multiply > 3000) {
          humidity = (humidity << 1) | 0x01;
        } else { 
          humidity = (humidity << 1);
        }
      }
      //==================================================================================
      // Sanity checks
      if (bitstream==0) return false;
      if (humidity==0) return false;
      rc = (bitstream >> 20) & 0xff;         // max. value = 0xAF
      if (rc > 0xae) return false;           // kill "maximum" packets that seem to be transmitted occasionally
      rc2= (bitstream >> 12) & 0xfb;         
      humidity=(humidity*33)/10;             // max. value = 0x1E ?
      temperature = (bitstream) & 0xfff;
      //fix 12 bit signed number conversion
      if ((temperature & 0x800) == 0x800) temperature = temperature - 0x1000;
      //==================================================================================
      // Output
      // ----------------------------------
      sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
      Serial.print( buffer );
      // ----------------------------------
      Serial.print("Alecto V4;");                      // Label
      sprintf(buffer, "ID=%02x%02x;", rc, rc2);        // ID 
      Serial.print( buffer );
      sprintf(buffer, "TEMP=%04x;", temperature);     
      Serial.print( buffer );
      sprintf(buffer, "HUM=%02d;", humidity);     
      Serial.print( buffer );        
      Serial.println();
      //==================================================================================
      RawSignal.Repeats=true;                          // suppress repeats of the same RF packet
      RawSignal.Number=0;
      success = true;
      break;
    }
#endif // PLUGIN_032_CORE
  }      
  return success;
}
