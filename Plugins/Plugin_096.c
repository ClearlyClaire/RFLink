//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ## 
//##                                          Plugin-96 Xiron                                          ##
//#######################################################################################################
/*********************************************************************************************\
 * This protocol provides reception/decoding of Xiron weatherstation outdoorsensors
 * 
 * Author             : Martinus van den Broek
 * Support            : None!
 * Date               : 29 Sep 2014 (debugging code removed)
 * Version            : 0.2
 * Compatibility      : R744
 * Syntax             : "XironV1 <Par1:Sensor ID>, <Par2:Basis Variabele>"
 *********************************************************************************************
 * Technical data:
 * Decodes signals from Xiron Weatherstation outdoor unit
 * Message Format: (84 pulses, 40 bits of data)
 * AAAAAAAA BB CC DDDDDDDDDDDD EEEE FFFFFFFF GGGG
 * ID       ?? Ch Temperature  ?    Humidity ?
 * A = ID (code changes after battery replacement)
 * B = ?
 * C = Channel (1,2,3)
 * D = Temperature (12 bit value)
 * E = ?
 * F = Humidity
 * G = ?
 *
 * RF Message is repeated 5 times, delay between messages is approx 3600 uSec
 * Current Nodo release (R744) cannot receive this because this delay is too short!!!
 * Two possible workarounds:
 *   1) Ajdust SIGNAL_TIMEOUT from default 5 mSec to 3 mSec (side effects unknown !!)
 *   2) Adjust RAW_BUFFER_SIZE from 256 to 512 so entire message burst can be processed
 * The current plugin version can handle both workarounds.
 * Workaround (1) seems to receive better in case of longer distance between Nodo and sensor.
 *
 * This plugin generates too many false positives. It needs a checksum calculation.
 \*********************************************************************************************/
 
#define PLUGIN_ID 96
#define PLUGIN_NAME "XironV1"

#define XIRON_PULSECOUNT_SINGLE 84 
#define XIRON_PULSECOUNT_REPEATED 508 

byte Plugin_096_ProtocolXironCheckID(byte checkID);

byte Plugin_096_ProtocolXironValidID[5];
byte Plugin_096_ProtocolXironVar[5];

boolean Plugin_096(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef PLUGIN_096_CORE
  case PLUGIN_RAWSIGNAL_IN:
    {
      if ((RawSignal.Number != XIRON_PULSECOUNT_SINGLE) && (RawSignal.Number != XIRON_PULSECOUNT_REPEATED)) return false;

      unsigned long bitstream1=0;
      unsigned long bitstream2=0;
      byte rc=0;
      int temperature=0;
      byte humidity=0;
      byte offset=2;
      byte channel=0;
      char buffer[11]=""; 
      
      if (RawSignal.Number == XIRON_PULSECOUNT_SINGLE) offset=0;
      // get first 28 bits
      for(byte x=(2+offset); x<=(56+offset); x=x+2) if(RawSignal.Pulses[x]*RawSignal.Multiply > 1250) bitstream1 = (bitstream1 << 1) | 0x1; 
      else bitstream1 = (bitstream1 << 1);
      // get remaining 12 bits
      for(byte x=(58+offset); x<=(80+offset); x=x+2) if(RawSignal.Pulses[x]*RawSignal.Multiply > 1250) bitstream2 = (bitstream2 << 1) | 0x1; 
      else bitstream2 = (bitstream2 << 1);
      //==================================================================================
      // should perform a checksum / validity operation here.
      if (bitstream1==0) return false;
      
      rc = (bitstream1 >> 20) & 0xff;
      channel = (bitstream1 >> 16) & 0x3;
      temperature = ((bitstream1 >> 4) & 0x3ff);
      humidity = ((bitstream2 >> 4) & 0xff);
      //==================================================================================
      // Output
      // ----------------------------------
      sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
      Serial.print( buffer );
      // ----------------------------------
      Serial.print("Xiron;");                          // Label
      sprintf(buffer, "ID=%02x%02x;", rc, channel);    // ID (rc+channel)
      Serial.print( buffer );
      sprintf(buffer, "TEMP=%04x;", temperature);     
      Serial.print( buffer );
      sprintf(buffer, "HUM=%02x;", humidity);     
      Serial.print( buffer );
      Serial.println();
      //Serial.write( ((temperature >> 8)&0xff) );// temp data
      //Serial.write( (temperature &0xff) );      // temp data
      //==================================================================================
      RawSignal.Repeats=true;                    // suppress repeats of the same RF packet
      RawSignal.Number=0;
      success = true;
      break;
    }
#endif // PLUGIN_096_CORE
  }      
  return success;
}
