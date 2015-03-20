//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                      Plugin-061 AlarmSensor                                       ##
//#######################################################################################################
/*********************************************************************************************\
 * This protocol provides support for some chinese Alarm "gadgets"
 * 
 * Author             : Martinus van den Broek
 * Support            : geen
 * Date               : 28 Okt 2013
 * Versie             : 0.1
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Technical data:
 * Devices send 50 pulses. Manchester encoded, 25 bits
 * No further data, might be just a 25 bits ID
 \*********************************************************************************************/
#define PLUGIN_ID 061
#define PLUGIN_NAME "AlarmSensor"

boolean Plugin_061(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef PLUGIN_061_CORE
  case PLUGIN_RAWSIGNAL_IN:
    {
      char buffer[14]=""; 
      RawSignal.Multiply=50;
      if (RawSignal.Number != 50) return false;

      unsigned long bitstream=0L;
      for(byte x=2;x<=48;x=x+2)
      {
        if ((RawSignal.Pulses[x-1]*RawSignal.Multiply < 600) && (RawSignal.Pulses[x]*RawSignal.Multiply < 600)) return false; // invalid manchestercode
        if ((RawSignal.Pulses[x-1]*RawSignal.Multiply > 600) && (RawSignal.Pulses[x]*RawSignal.Multiply > 600)) return false; // invalid manchestercode
        if (RawSignal.Pulses[x]*RawSignal.Multiply > 600) bitstream = (bitstream << 1) | 0x1; 
        else bitstream = bitstream << 1;
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
      Serial.print("PIR=ON;");                         // this device does report movement only
      Serial.println();
      //==================================================================================
      RawSignal.Repeats=true;                          // suppress repeats of the same RF packet
      RawSignal.Number=0;
      return true;
      break;
    }
#endif // Plugin_061_CORE
  }      
  return success;
}

