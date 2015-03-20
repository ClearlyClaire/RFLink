//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                    Plugin-254: Signal Analyzer                                    ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin shows pulse lengths that have been received on RF (or IR) and have not been decoded by
 * one of the other plugins. The primary use of this plugin is to provide an easy way to debug and 
 * analyse currently unsupported RF signals
 *
 * Author             : StuntTeam
 * Support            : www.nodo-domotica.nl
 * Datum              : 9-02-2015
 * Versie             : 1.0
 * Nodo productnummer : 254
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 ***********************************************************************************************
 * Technical description:
 * This plugin just outputs unsupported  RF packets, use this plugin to find signals from new devices
 * Even if you do not know what to do with the data yourself you might want to share your data so
 * others can analyse it.
 \*********************************************************************************************/
#define PLUGIN_ID 254
#define PLUGIN_NAME "RawSignalAnalyze"

boolean Plugin_254(byte function, struct NodoEventStruct *event, char *string) {
  boolean success=false;
  
  switch(function) {
    case PLUGIN_RAWSIGNAL_IN: 
      {
        if (RFUDebug==false) break;            // debuggin is on? 
        char buffer[14]=""; 
        if(RawSignal.Number<8)break;           // make sure the packet is long enough to have a meaning 
        // ----------------------------------
        // Output
        // ----------------------------------
        sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
        Serial.print( buffer );
        // ----------------------------------
        Serial.print(F("DEBUG;Pulses="));      // debug data
        Serial.print(RawSignal.Number);
        Serial.print(F(";Pulses(uSec)="));      
        for(int x=1;x<RawSignal.Number;x++) {
            Serial.print(RawSignal.Pulses[x]*RawSignal.Multiply); 
            if (x < RawSignal.Number-1) Serial.write(',');       
        }
        Serial.println(";");
        RawSignal.Number=0;
        break;
      }      
  }      
  return false;
}
