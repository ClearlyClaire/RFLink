//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                       Plugin-44 Auriol                                            ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of decoding the Auriol protocol for sensor type Z32171A
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
 * Technical Information:
 * Decodes signals from a Auriol Weatherstation outdoor unit, (40 bits, 433 MHz).
 *
 * Auriol Message Format: 
 * 1011 1111 1001 1010 0110 0001 1011 0100 1001 0001
 * B    F    9    A    6    1    B    4    9    1
 * AAAA AAAA BBBB CCDD EEEE EEEE EEEE FFFF FFFF GGHH
 *
 * A = ID?
 * B = Rolling code?
 * C = possibly battery indicator ?
 * D = trend (2 bits) indicating temp equal/up/down ?
 * E = Temperature => 0x61b  (0x61b-0x4c4)=0x157 *5)=0x6b3 /9)=0xBE => 0xBE = 190 decimal!
 * F = humidity: 49% 
 * G = ?
 * H = channel: 1 (2 bits)
 *
 * Sample:
 * 20;C2;DEBUG;Pulses=82;Pulses(uSec)=475,3850,450,1700,450,3825,450,3900,450,3725,450,3825,450,3825,450,3900,450,3725,450,1700,450,1700,450,3900,450,3725,450,1700,450,1700,450,1800,450,1625,450,3800,450,3825,450,1800,450,1625,450,1700,450,1700,450,1800,450,3725,450,3800,450,1700,450,1800,450,1625,450,3825,450,1700,450,3900,450,1625,450,1700,450,1700,450,3900,450,1625,450,1700,450,1700,450,3825,500;
 \*********************************************************************************************/
#define PLUGIN_ID 44
#define PLUGIN_NAME "AuriolV3"

#define AURIOL_PULSECOUNT 82

boolean Plugin_044(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef PLUGIN_044_CORE
  case PLUGIN_RAWSIGNAL_IN:
    {
      if (RawSignal.Number != AURIOL_PULSECOUNT) return false;

      unsigned long bitstream1=0;
      unsigned long bitstream2=0;
      byte rc=0;
      byte channel=0;
      unsigned long temperature=0;
      int humidity=0;
	  int bitcounter=0;
      char buffer[14]=""; 
      //==================================================================================
      // get all the bits we need (40 bits)
      for(int x=2;x<RawSignal.Number;x+=2) {
         if (RawSignal.Pulses[x]*RawSignal.Multiply > 3500) {
            if (bitcounter < 16) {
               bitstream1 = (bitstream1 << 1) | 0x1;
               bitcounter++;                     // only need to count the first 10 bits
            } else {
               bitstream2 = (bitstream2 << 1) | 0x1;
            }
         } else {
            if (RawSignal.Pulses[x]*RawSignal.Multiply > 2000) return false;
            if (RawSignal.Pulses[x]*RawSignal.Multiply < 1500) return false;
            if (bitcounter < 16) {
               bitstream1 = (bitstream1 << 1); 
               bitcounter++;                     // only need to count the first 10 bits
            } else {
               bitstream2 = (bitstream2 << 1); 
            }
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
      // First perform sanity checks
      if (bitstream1 == 0) return false;
      if (bitstream2 == 0) return false;
      //==================================================================================
      rc = (bitstream1 >> 8) & 0xff ;               // get rolling code
      temperature = ((bitstream2)>>12) & 0xfff;     // get 12 temperature bits
      temperature = (temperature - 0x4c4) & 0xffff;
      temperature = (((temperature) * 5) / 9) & 0xffff;
      if (temperature > 3000) {
         temperature=4096-temperature;              // fix for minus temperatures
         temperature=temperature | 0x8000;          // turn highest bit on for minus values
      }      
      humidity = (bitstream2 >> 4) & 0xff ;         // humidity
      channel=(bitstream2) & 0x03;                  // channel number
      //==================================================================================
      // Output
      // ----------------------------------
      sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
      Serial.print( buffer );
      // ----------------------------------
      Serial.print("Auriol V3;");                   // Label
      sprintf(buffer, "ID=%02x%02x;", rc,channel);  // ID    
      Serial.print( buffer );
      sprintf(buffer, "TEMP=%04x;", temperature);   // temp  
      Serial.print( buffer );
      sprintf(buffer, "HUM=%02x;", humidity);       // hum
      Serial.print( buffer );
      Serial.println();
      //==================================================================================
      RawSignal.Repeats=true;                       // suppress repeats of the same RF packet 
      RawSignal.Number=0;
      success = true;
      break;
    }
#endif // PLUGIN_044_CORE
  }      
  return success;
}
