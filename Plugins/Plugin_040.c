//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                        Plugin-40 Mebus                                            ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of decoding Mebus weatherstation outdoor sensors
 * It concerns Mebus sensors that are not following the Cresta (Hideki) protocol
 * Also sold as Stacja Pogody WS-9941-M
 * 
 * Author             : StuntTeam
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical information:
 * Decodes signals from a Mebus Weatherstation outdoor unit, (29 pulses, 28 bits, 433 MHz).
 * Mebus Message Format: 
 * AAAA BBBB BBBB CCCC CCCC CCCC DDEF
 *
 * A = Checksum value    AAAA=((BBBB+BBBB+CCCC+CCCC+DDEF)-1)&0x0f
 * B = Changes after each reset, no change during normal operation. (Device 'Session' ID)
 * C = Temperature (21.5 degrees is shown as decimal value 215, minus values have the high bit set and need to be subtracted from a base value of 4096)
 * D = Channel number 1/2/3
 * E = Always 1
 * F = 0 when "normal" data transmit, 1 when "requested" data transmit (TX button press)
 *
 * Needs SIGNAL_TIMEOUT=5 
 \*********************************************************************************************/
   // ==================================================================================
   // MEBUS bit packets 
   // 0000 1101 1001 0000 1100 1000 0111 
   // 0100 1101 1001 0000 1101 1100 0110 
   // 0100 1101 1001 0000 1100 1101 0110 
   // 1001 1101 1001 0000 1101 1100 1011 
   // 1011 1101 1001 0001 0000 1111 0110    27.1
   // 0010 0110 1110 0000 0000 1001 0110    0.9
   // 0011 0110 1110 1111 1000 0011 0110    -12.5 (1111 1000 0011=3971, 4096-3971=125
   //                  |----------|----------> temperature 0 - 51.1  
   //                |-|---------------------> set when minus temperatures -51.2 - 0
   // ==================================================================================
#define PLUGIN_ID 40
#define PLUGIN_NAME "Mebus"
#define MEBUS_PULSECOUNT 58

boolean Plugin_040(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef PLUGIN_040_CORE
  case PLUGIN_RAWSIGNAL_IN:
    {
      if (RawSignal.Number != MEBUS_PULSECOUNT) return false;

      char buffer[11]=""; 
      unsigned long bitstream1=0;
      byte rc=0;
      int temperature=0;
      byte minus=0;
      byte checksum=0;
      byte data[7];
      byte channel=0;
      //==================================================================================
      // get all 28 bits
      for(byte x=2;x <=56;x+=2) {
         if(RawSignal.Pulses[x]*RawSignal.Multiply > 2975) {
           bitstream1 = (bitstream1 << 1) | 0x1; 
         } else {
           bitstream1 = (bitstream1 << 1);
         }
      }
      //==================================================================================
      data[0] = (bitstream1 >> 24) & 0x0f;     // prepare nibbles from bit stream
      data[1] = (bitstream1 >> 20) & 0x0f;
      data[2] = (bitstream1 >> 16) & 0x0f;
      data[3] = (bitstream1 >> 12) & 0x0f;
      data[4] = (bitstream1 >>  8) & 0x0f;
      data[5] = (bitstream1 >>  4) & 0x0f;
      data[6] = (bitstream1 >>  0) & 0x0f;
      //==================================================================================
      // first perform a checksum check to make sure the packet is a valid mebus packet
      checksum=data[1]+data[2]+data[3]+data[4]+data[5]+data[6];
      checksum=(checksum-1)&0xf;
      if (checksum != data[0]) return false;
      //==================================================================================
      rc=(data[1]<<4) + data[2];
      channel=data[6]>>2;
      temperature=(data[3]<<8)+(data[4]<<4)+data[5];
      if (temperature > 3000) {
         temperature=4096-temperature;              // fix for minus temperatures
         //minus=1;
         temperature=temperature | 0x8000;          // turn highest bit on for minus values
      }
      //==================================================================================
      // Output
      // ----------------------------------
      sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
      Serial.print( buffer );
      Serial.print("Mebus;");                          // Label
      sprintf(buffer, "ID=%02x%02x;", rc, channel);    // ID    
      Serial.print( buffer );
      sprintf(buffer, "TEMP=%04x;", temperature);     
      Serial.print( buffer );
      Serial.println();
      //==================================================================================
      RawSignal.Repeats=true;                    // suppress repeats of the same RF packet
      RawSignal.Number=0;
      success = true;
      break;
    }
#endif // PLUGIN_040_CORE

  }      
  return success;
}
