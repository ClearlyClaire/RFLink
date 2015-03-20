//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                      Plugin-060 AlarmSensor                                       ##
//#######################################################################################################
/*********************************************************************************************\
 * This protocol provides support for some Alarm sensors that are part of a Varel alarm
 * 
 * Author             : StuntTeam
 * Support            : www.nodo-domotica.nl
 * Date               : 9-02-2015
 * Version            : 1.0
 * Compatibility      : RFLink 1.0
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical data:
 * Devices send 26 pulses. Manchester encoded, 26 bits
 * No further data, might be just a 25 bits ID
 *
 * Sample:
 * 20;74;DEBUG;Pulses=26;Pulses(uSec)=425,425,800,875,350,875,350,875,350,875,350,875,350,875,350,875,350,400,800,875,350,400,825,875,350;
 \*********************************************************************************************/
#define PLUGIN_ID 060
#define PLUGIN_NAME "AlarmPIR"

boolean Plugin_060(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef PLUGIN_060_CORE
  case PLUGIN_RAWSIGNAL_IN:
    {
      if (RawSignal.Number != 26) return false;

      unsigned long bitstream=0L;
      byte data[3];
      char buffer[11]=""; 
      
      for(byte x=1;x<RawSignal.Number;x++) {
          if (RawSignal.Pulses[x]*RawSignal.Multiply > 600) {
             if (RawSignal.Pulses[x]*RawSignal.Multiply > 1000) return false;
             bitstream = (bitstream << 1) | 0x1; 
          } else {
             bitstream = bitstream << 1;
          }
      }

      if (bitstream == 0) return false;
      //==================================================================================
      // Output
      // ----------------------------------
      sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
      Serial.print( buffer );
      // ----------------------------------
      Serial.print("Pir;");                            // Label
      sprintf(buffer, "ID=%04x;", bitstream &0xffff);  // ID    
      Serial.print( buffer );
      Serial.print("PIR=ON;");                // this device does report movement only
      Serial.println();
      //==================================================================================
      RawSignal.Repeats=true;                    // suppress repeats of the same RF packet
      RawSignal.Number=0;
      return true;
      break;
    }
#endif // PLUGIN_060_CORE
  }      
  return success;
}
