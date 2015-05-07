//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                      Plugin-060 AlarmSensor                                       ##
//#######################################################################################################
/*********************************************************************************************\
 * This protocol provides support for some Alarm sensors that are part of a Varel alarm
 * 
 * Author             : StuntTeam
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical data:
 * Devices send 25 pulses, first pulse is part of the start bit. Remaining bits are Manchester encoded, 24 bits
 *
 * The PCB contains a Holtec HT12E Encoder chip
 * The PCB has two switch blocks: SW1 with switches 1-8  (Device code?)
 *                                SW2 with switches 1-4  (House code?)
 *
 * Sample:
 * 20;74;DEBUG;Pulses=26;Pulses(uSec)=425,425,800,875,350,875,350,875,350,875,350,875,350,875,350,875,350,400,800,875,350,400,825,875,350;
 * 1001101010101010 01100110
 * 10000000 1010
 \*********************************************************************************************/
#define PLUGIN_ID 060
#define PLUGIN_NAME "AlarmPIR"
#define ALARMPIRV2_PULSECOUNT 26

boolean Plugin_060(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef PLUGIN_060_CORE
  case PLUGIN_RAWSIGNAL_IN:
    {
      unsigned long bitstream=0L;
      byte data[3];
      char buffer[11]=""; 
      //==================================================================================
      if (RawSignal.Number != ALARMPIRV2_PULSECOUNT) return false;
      if (RawSignal.Pulses[1]*RawSignal.Multiply > 550) return false;    // First pulse is start bit and should be short!
      for(byte x=2;x<RawSignal.Number;x=x+2) {
          if (RawSignal.Pulses[x]*RawSignal.Multiply > 700) { // long pulse
             if (RawSignal.Pulses[x]*RawSignal.Multiply > 1000) return false;
             if (RawSignal.Pulses[x+1]*RawSignal.Multiply > 700) return false; // invalid manchestercode
             bitstream = bitstream << 1;
          } else { // short pulse
             if (RawSignal.Pulses[x]*RawSignal.Multiply < 250) return false;  // too short
             if (RawSignal.Pulses[x+1]*RawSignal.Multiply < 700) return false; // invalid manchestercode
             bitstream = (bitstream << 1) | 0x1; 
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
      if (bitstream == 0) return false;
      //==================================================================================
      // Output
      // ----------------------------------
      sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
      Serial.print( buffer );
      // ----------------------------------
      Serial.print("X10;");                         // Label
      sprintf(buffer, "ID=%04x;", bitstream);
      Serial.print( buffer );
      Serial.print("SWITCH=01;");
      Serial.print("CMD=ON;");                      // this device does report movement only
      Serial.println();
      //==================================================================================
      RawSignal.Repeats=true;                       // suppress repeats of the same RF packet
      RawSignal.Number=0;
      return true;
      break;
    }
#endif // PLUGIN_060_CORE
  }      
  return success;
}
