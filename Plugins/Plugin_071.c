//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                 Plugin-71 Plieger York doorbell                                   ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of decoding the Plieger York Doorbell protocol
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
 * Decodes signals from a Plieger York Doorbell, (66 pulses, 32 bits, 433 MHz).
 * Plieger Message Format: 
 * 0000000001010101 00000000 00011100    c2  0x1c
 *                           00000011    c3  0x03
 *                           11100000    c1  0xE0
 *                           --------   8 bits chime number (3 chimes, can be changed with a jumped on the transmitter) 
 *                  -------- 8 bits always 0 
 * ---------------- 16 bits code which can be changed with a button on the inside of the transmitter 
 *
 * Note: The transmitter sends two times the same packet when the bell button is pressed
 * the retransmit is killed to prevent reporting the same press twice
 *
 * Sample packet: (Nodo Pulse timing)
 * Pulses=66, Pulses(uSec)=700,250,275,725,750,250,275,725,750,250,275,725,750,250,275,725,750,250,
 * 275,725,750,250,275,725,750,250,275,725,750,250,275,725,275,725,275,725,275,725,275,725,275,725,
 * 275,725,275,725,275,725,275,725,275,725,275,725,750,250,750,250,750,250,275,725,275,725,225,
 \*********************************************************************************************/
   // ==================================================================================
#define PLUGIN_ID 71
#define PLUGIN_NAME "Plieger"
#define PLIEGER_PULSECOUNT 66

boolean Plugin_071(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef PLUGIN_071_CORE
  case PLUGIN_RAWSIGNAL_IN:
    {
      if (RawSignal.Number != PLIEGER_PULSECOUNT) return false;

      unsigned long bitstream=0;
      byte rc=0;
      int id=0;
      byte chime=0;
      byte basevar=0;
      char buffer[11]=""; 
      //==================================================================================
      // get all 32 bits
      for(byte x=1;x <=64;x+=2) {
         if(RawSignal.Pulses[x]*RawSignal.Multiply > 400) {
           if (RawSignal.Pulses[x]*RawSignal.Multiply > 1200) return false;
           bitstream = (bitstream << 1) | 0x1; 
         } else {
           bitstream = (bitstream << 1);
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
      // first perform two checks to validate the data
      if (((bitstream >> 8) &0xff) != 0x00) return false;               // these 8 bits are always 0
      chime=bitstream &0xff;
      if (chime != 0x1c && chime !=0x03 && chime != 0xE0) return false; // the chime number can only have 3 values
      //==================================================================================
      id=(bitstream >> 16) & 0xffff;                   // get 16 bits unique address
      if (chime == 0xE0) chime =1;
      if (chime == 0x1C) chime =2;
      //==================================================================================
      // Output
      // ----------------------------------
      sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
      Serial.print( buffer );
      // ----------------------------------
      Serial.print("Plieger York;");                   // Label
      sprintf(buffer, "ID=%04x;", id);                 // ID      
      Serial.print( buffer );
      Serial.print("SWITCH=1;CMD=ON;");  
      sprintf(buffer, "CHIME=%02x;", chime);           // chime number
      Serial.print( buffer );
      Serial.println();
      //==================================================================================
      RawSignal.Repeats=true;                          // suppress repeats of the same RF packet
      RawSignal.Number=0;                              // do not process the packet any further
      success = true;                                  // processing successful
      break;
    }
#endif // PLUGIN_071_CORE
  }      
  return success;
}
