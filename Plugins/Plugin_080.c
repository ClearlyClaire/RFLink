//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                               Plugin-80 Flamingo FA20RF Rookmelder                                ##
//#######################################################################################################
/*********************************************************************************************\
 * Dit protocol zorgt voor ontvangst van Flamingo FA20RF rookmelder
 * 
 * Author             : StuntTeam
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical information:
 * The Flamingo FA20RF/FA21RF Smokedetector contains both a RF receiver and transmitter. 
 * Every unit has a unique ID. The detector has a "learn" button to learn the ID of other units. 
 * That is what links the units to eachother. After linking multiple units, they all have the same ID!
 * Normally, one unit is used as master and the code of the master is learned to the slaves
 *
 * Attention:  The smoke detector gives an alarm as long as the message is transmitted
 *
 * Sample:
 * 20;32;DEBUG;Pulses=52;Pulses(uSec)=2500,800,650,1250,650,1250,650,1250,650,1250,650,1250,650,1275,650,1250,650,2550,650,1275,650,2550,650,1250,650,1250,650,2550,650,2550,650,1275,650,2550,
 * 650,2550,650,1275,650,2550,650,2550,650,1275,650,1275,650,2550,650,1200,650;
 * 000000010100110110110010 = 014DB2
 * 20;0C;DEBUG;Pulses=52;Pulses(uSec)=2500,825,625,2575,625,1275,625,1300,625,2575,625,1275,625,2575,625,2575,625,2575,625,2575,625,2575,625,2575,625,1275,625,1275,625,1275,625,2575,625,2575,
 * 625,2575,625,1275,625,2575,625,2575,625,1300,625,1275,625,2575,625,1225,625;
 * 100101111110001110110010 = 97E3B2
 * 20;0D;FA20RF;ID=97e3b2;SMOKEALERT=ON;
 \*********************************************************************************************/
#define FA20RFSTART                 3000    // 8000
#define FA20RFSPACE                  675    //  800
#define FA20RFLOW                   1250    // 1300
#define FA20RFHIGH                  2550    // 2600
#define FA20_PULSECOUNT             52

#ifdef PLUGIN_080
boolean Plugin_080(byte function, char *string) {
      if (RawSignal.Number != FA20_PULSECOUNT) return false;
      unsigned long bitstream=0L;
      for(byte x=4;x<=FA20_PULSECOUNT-2;x=x+2) {
        if (RawSignal.Pulses[x-1]*RAWSIGNAL_SAMPLE_RATE > 1000) return false; // every preceding puls must be < 1000!
        if (RawSignal.Pulses[x]*RAWSIGNAL_SAMPLE_RATE > 1800) bitstream = (bitstream << 1) | 0x1; 
        else bitstream = bitstream << 1;
      }
      if (bitstream == 0) return false;
      if (bitstream == 0xFFFFFF) return false;
      if (((bitstream)&0xffff) == 0xffff) return false;
      //==================================================================================
      // Output
      // ----------------------------------
      sprintf(pbuffer, "20;%02X;", PKSequenceNumber++);// Node and packet number 
      Serial.print( pbuffer );
      // ----------------------------------
      Serial.print(F("FA20RF;"));                      // Label
      sprintf(pbuffer, "ID=%06lx;", bitstream );       // ID 
      Serial.print( pbuffer );
      Serial.print(F("SMOKEALERT=ON;"));
      Serial.println();
      //==================================================================================
      RawSignal.Repeats=true;                          // suppress repeats of the same RF packet
      RawSignal.Number=0;                              // do not process the packet any further
      return true;
}
#endif // PLUGIN_080

#ifdef PLUGIN_TX_080
boolean PluginTX_080(byte function, char *string) {
      boolean success=false;
      //10;SmokeAlert;123456;ON;
      //012345678901234567890
      unsigned long bitstream=0;
      if (strncasecmp(InputBuffer_Serial+3,"SmokeAlert;",11) == 0) { // KAKU Command eg. 
         if (InputBuffer_Serial[20] != ';') return false;
         InputBuffer_Serial[12]=0x30;
         InputBuffer_Serial[13]=0x78;
         InputBuffer_Serial[20]=0;
         bitstream=str2int(InputBuffer_Serial+12); 
         // ---------- SMOKEALERT SEND -----------
         RawSignal.Multiply=50;
         RawSignal.Repeats=10;
         RawSignal.Delay=20;
         RawSignal.Pulses[1]=FA20RFSTART/RawSignal.Multiply;
         //RawSignal.Pulses[2]=FA20RFSPACE/RawSignal.Multiply;
         //RawSignal.Pulses[3]=FA20RFSPACE/RawSignal.Multiply;
         RawSignal.Pulses[2]=(FA20RFSPACE+125)/RawSignal.Multiply;
         RawSignal.Pulses[3]=(FA20RFSPACE+25)/RawSignal.Multiply;         
         for(byte x=49;x>=3;x=x-2) {
            RawSignal.Pulses[x]=FA20RFSPACE/RawSignal.Multiply;
            if ((bitstream & 1) == 1) 
               RawSignal.Pulses[x+1] = FA20RFHIGH/RawSignal.Multiply; 
            else 
               RawSignal.Pulses[x+1] = FA20RFLOW/RawSignal.Multiply;
            bitstream = bitstream >> 1;
         }
         RawSignal.Pulses[51]=FA20RFSPACE/RawSignal.Multiply;
         RawSignal.Pulses[52]=0;
         RawSignal.Number=52;
         RawSendRF();
         RawSignal.Multiply=25;                     // restore setting
         success=true;        
      }        
      return success;
}
#endif // PLUGIN_080
