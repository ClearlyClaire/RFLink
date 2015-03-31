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
 *
 * PCB contains 2 chips: biss0001 & ev1527
 * BISS0001 = Micro Power PIR Motion Detector IC  
 * EV1527   = OTP Encoder, max. of 20 bits providing up to 1 million codes.
 *
 * Sample:
 * Pulses=50, Pulses(uSec)=900,225,200,850,225,850,825,225,225,850,225,850,825,225,225,850,200,850,225,850,825,225,225,850,825,225,225,850,825,225,825,225,825,225,825,225,825,225,825,225,825,225,225,850,225,850,825,225,225,
 * 100101100101100101011001100110101010101010010110 
 * 01101101 1101010000000110  6D D406
 \*********************************************************************************************/
#define PLUGIN_ID 061
#define PLUGIN_NAME "AlarmSensor"
#define ALARMPIRV1_PULSECOUNT 50

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

      unsigned long bitstream=0L;
      unsigned long bitstream2=0L;
      //==================================================================================
      if (RawSignal.Number != ALARMPIRV1_PULSECOUNT) return false;
      for(byte x=2;x<=48;x=x+2) {
        if ((RawSignal.Pulses[x-1]*RawSignal.Multiply < 600) && (RawSignal.Pulses[x]*RawSignal.Multiply < 600)) return false; // invalid manchestercode
        if ((RawSignal.Pulses[x-1]*RawSignal.Multiply > 600) && (RawSignal.Pulses[x]*RawSignal.Multiply > 600)) return false; // invalid manchestercode
        if (RawSignal.Pulses[x]*RawSignal.Multiply > 600) bitstream = (bitstream << 1) | 0x1; 
        else bitstream = bitstream << 1;
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
      bitstream2=(bitstream)>>16;
      //==================================================================================
      // Output
      // ----------------------------------
      sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
      Serial.print( buffer );
      // ----------------------------------
      Serial.print("X10;");                         // Label
      sprintf(buffer, "ID=%04x;", bitstream &0xffff);  // ID    
      Serial.print( buffer );
      sprintf(buffer, "SWITCH=%02x;", bitstream2 &0xff);  // ID    
      Serial.print( buffer );
      Serial.print("CMD=ON;");                      // this device reports movement only
      Serial.println();
      //==================================================================================
      RawSignal.Repeats=true;                       // suppress repeats of the same RF packet
      RawSignal.Number=0;
      return true;
      break;
    }
#endif // Plugin_061_CORE
  }      
  return success;
}

