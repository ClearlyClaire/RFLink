//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                       Plugin-46 Auriol                                            ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of decoding the Auriol protocol for sensor type Z31055A-TX
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
 * Decodes signals from a Auriol Weatherstation outdoor unit, (36 bits, 433 MHz).
 * Auriol Message Format: 
 * 1101 0110 1000 0000 1101 1111 1111 0000
 * 1010 0100 1000 0000 1111 1011 1111 0000
 * AAAA AAAA BCCC DDDD DDDD DDDD EEEE FFFF
 *
 * A = Rolling Code, no change during normal operation. (Device 'Session' ID) (Might also be 4 bits RC and 4 bits for channel number)
 * B = Battery status indicator, 1=OK 0=LOW
 * C = Always 000
 * D = Temperature (21.5 degrees is shown as decimal value 215, minus values have the high bit set and need to be subtracted from a base value of 4096)
 * E = Always 1111 ?
 * F = Always 0000 ?
 * 
 * Sample debug data: 
 * Auriol ID=0, A480F5F0- 1010 0100 1000 0000 1111 0101 1111 0000, Temperature= 24.50
 * Auriol ID=0, A480FBF0- 1010 0100 1000 0000 1111 1011 1111 0000, Temperature= 25.10
 \*********************************************************************************************/
#define PLUGIN_ID 46
#define PLUGIN_NAME "Auriol"

#define AURIOL_PULSECOUNT 255

boolean Plugin_046(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef PLUGIN_046_CORE
  case PLUGIN_RAWSIGNAL_IN:
    {
      if (RawSignal.Number != AURIOL_PULSECOUNT) return false;

      unsigned long bitstream1=0;
      byte rc=0;
      byte bat=0;
      int temperature=0;
      byte checksum=0;
      byte start=0;
      byte basevar;
      char buffer[11]=""; 
      
      byte temp=0;
      byte checksumcalc=0;
      byte type=0;
      //==================================================================================
      // get all the bits we need (32 bits)
      for(int x=1;x <=255;x++) {
         if(RawSignal.Pulses[x]*RawSignal.Multiply > 3000) { // find first byte
            start=x+2; // start position is 2nd byte after the long pulse (which is too short for Nodo to see as a packet break)
            break;
         }
      }
      for(int x=start;x <start+64;x+=2) {
         if(RawSignal.Pulses[x]*RawSignal.Multiply > 1200) {
           bitstream1 = (bitstream1 << 1) | 0x1; 
         } else {
           bitstream1 = (bitstream1 << 1);
         }
      }
      //==================================================================================
      // First perform a sanity check
      if (bitstream1 == 0) return false;
      // ------------------------
      // Perform a checksum calculation to make sure the received packet is a valid Auriol packet
      for (int i=1;i<32;i++) {
          checksumcalc=checksumcalc^ ((bitstream1>>i)&0x01);
      }
      // differentiate between auriol models
      if ((bitstream1 & 0xff) == 0xF0) { // 
         if (checksumcalc== (bitstream1&0x01) ) {
            //Serial.println("CRC 1 Error");
            return false;
         }
      } else {
        if (checksumcalc!= (bitstream1&0x01) ) {
            //Serial.println("CRC 2 Error");
            return false;
        }
      }
      // ----------------------------------
      // After the checksum check, do another sanity check on some selected bits
      rc = (bitstream1 >> 20) & 0x07;            // get 3 bits, should always be 000
      if (rc != 0) return false; 
      //==================================================================================
      bat= (bitstream1 >> 23) & 0x01;            // get battery strength indicator
      rc = (bitstream1 >> 24) & 0xff;            // get rolling code
      temperature = (bitstream1 >> 8) & 0xfff;   // get 12 temperature bits
      if (temperature > 3000) {
         temperature=4096-temperature;           // fix for minus temperatures
         temperature=temperature | 0x8000;       // turn highest bit on for minus values
      }      
      //==================================================================================
      // Output
      // ----------------------------------
      sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
      Serial.print( buffer );
      // ----------------------------------
      Serial.print("Auriol V2;");                   // Label
      sprintf(buffer, "ID=00%02x;", rc);         // ID    
      Serial.print( buffer );
      sprintf(buffer, "TEMP=%04x;", temperature);     
      Serial.print( buffer );
      if (bat==0) {                              // battery status
         Serial.print("BAT=LOW;");
      } else {
         Serial.print("BAT=OK;");
      }
      Serial.print( bitstream1,HEX );
      Serial.print(";");
      Serial.println();
      //==================================================================================
      RawSignal.Number=0;
      success = true;
      break;
    }
#endif // PLUGIN_046_CORE
  }      
  return success;
}
