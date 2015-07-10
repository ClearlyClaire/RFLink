//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                      Plugin-15: HomeEasy EU                                       ##
//#######################################################################################################
/*********************************************************************************************\
 * Dit protocol zorgt voor ontvangst en verzending HomeEasy EU zenders
 * die werken volgens de automatische codering (Ontvangers met leer-knop)
 *
 * LET OP: GEEN SUPPORT VOOR DIRECTE DIMWAARDES!!!
 *
 * Author             : StuntTeam
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Technische informatie:
 * Analyses Home Easy Messages and convert these into an eventcode
 * Only new EU devices with automatic code system are supported
 * Only  On / Off status is decoded, no DIM values
 * Only tested with Home Easy HE300WEU transmitter, doorsensor and PIR sensor
 * Home Easy message structure, by analyzing bitpatterns so far ...
 * AAAAAAAAAAA BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB CCCC DD EE FFFFFF G
 * 11000111100 10111100011101110010001111100011 1100 10 11 000111 1  HE301EU ON
 * 11000111100 10111100011101110010001111100011 1100 01 11 000111 1  HE301EU OFF
 *  
 * A = Startbits/Preamble, 
 * B = Address, 32 bits
 * C = Unknown, Possibly: Device type 
 * D = Command, 1 bit only?
 * E = Group indicator
 * F = Channel  0-15
 * G = Stopbit
 *
 * SAMPLE:
 * Pulses=116;Pulses(uSec)=200,1175,125,1175,125,200,150,200,125,200,150,1175,150,1175,150,1175,150,1175,125,200,150,200,150,200,125,1175,150,1175,150,1175,125,1175,150,200,125,200,150,1175,125,1175,150,200,125,1175,125,1175,150,200,150,200,150,1175,150,200,150,1175,150,200,150,1175,150,200,150,200,125,1175,150,200,125,1175,150,1175,125,1175,150,200,125,200,125,200,150,200,125,1175,150,1175,150,1175,150,200,150,200,125,200,150,1175,150,1175,150,1175,150,1175,125,200,150,200,125,1175,125,200,125,1175,150,1150,125;
 * HE preamble: 11000111100 (63C) Address: 1111001101100101010010111000011 (79B2A5C3) Stopbits: 0 (0) Commands: 10001111001011 Command: 0 Channel: 1011 Group: 1
 * 20;04;HomeEasy;ID=7900b200;SWITCH=0b;CMD=ALLOFF;
 *
 * Preamble 200,1175,125,1175,125,200,150,200,125,200,150,1175,150,1175,150,1175,150,1175,125,200,150,200,
 * Address  150,200,125,1175,150,1175,150,1175,125,1175,150,200,125,200,150,1175,125,1175,150,200,125,1175,125,1175,150,200,150,200,150,1175,150,200,150,1175,150,200,150,1175,150,200,150,200,125,1175,150,200,125,1175,150,1175,125,1175,150,200,125,200,125,200,150,200,125,1175,150,1175,
 * Command  150,1175,150,200,150,200,125,200,150,1175,150,1175,150,1175,150,1175,125,200,150,200,125,1175,125,200,125,1175,150,1150,  - 125;
 \*********************************************************************************************/
 
#define HomeEasy_LongLow        0x490    // us
#define HomeEasy_ShortHigh      200      // us
#define HomeEasy_ShortLow       150      // us
#define HomeEasy_PulseLength    116

boolean Plugin_015(byte function, char *string) {
  boolean success=false;

  #ifdef PLUGIN_015_CORE
      //==================================================================================
      // valid messages are 116 pulses          
      if (RawSignal.Number != HomeEasy_PulseLength) return false;
      unsigned long preamble = 0L;
      unsigned long address = 0L;
      unsigned long bitstream = 0L;
      byte rfbit =0;
      byte command = 0;
      byte group = 0;
      byte channel = 0;
      
      // convert pulses into bit sections (preamble, address, bitstream)
      for(byte x=1;x<=HomeEasy_PulseLength;x=x+2) {
         if ((RawSignal.Pulses[x]*RawSignal.Multiply < 500) & (RawSignal.Pulses[x+1]*RawSignal.Multiply > 500)) 
            rfbit = 1;
         else
            rfbit = 0;
            
         if (x<=22) preamble = (preamble << 1) | rfbit;              // 11 bits preamble
         if ((x>=23) && (x<=86)) address = (address << 1) | rfbit;   // 32 bits address
         if ((x>=87) && (x<=114)) bitstream = (bitstream << 1) | rfbit; // 15 remaining bits
      }
      //==================================================================================
      // To prevent false positives make sure the preamble is correct, 
      // it should always be 0x63c but we compare only 10 bits to compensate for the first bit being seen incorrectly 
      if ( (preamble & 0x3ff) != 0x23c) {          // comparing 10 bits is enough to make sure the packet is valid
         return false;        
      }
      //==================================================================================
      // Prevent repeating signals from showing up
      //==================================================================================
      if(SignalHash!=SignalHashPrevious || (RepeatingTimer<millis() && SignalCRC != bitstream) || SignalCRC != bitstream ) { 
         // not seen the RF packet recently
         SignalCRC=bitstream;
      } else {
         // already seen the RF packet recently
         return true;
      }       
      //==================================================================================
      command = ((bitstream >> 9) & 0x1);      // 1=off 0=on ?
      channel = (bitstream) & 0x3f;
      group = ((bitstream >> 7) & 0x1);        // 1=group 
      // ----------------------------------
      // Output
      // ----------------------------------
      sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
      Serial.print( pbuffer );
      // ----------------------------------
      Serial.print("HomeEasy;");                  // Label
      sprintf(pbuffer, "ID=%08lx;",(address) );   // ID   
      Serial.print( pbuffer );
      sprintf(pbuffer, "SWITCH=%02x;", channel);     
      Serial.print( pbuffer );
      strcpy(pbuffer,"CMD=");
      if ( group == 1) {
         strcat(pbuffer,"ALL");
      }
      if ( command == 0) {
         strcat(pbuffer,"OFF;");
      } else {
         strcat(pbuffer,"ON;");
      }
      Serial.print( pbuffer );
      Serial.println();     
      // ----------------------------------
      RawSignal.Repeats    = true; // het is een herhalend signaal. Bij ontvangst herhalingen onderdrukken.
      success=true;
#endif // PLUGIN_015_CORE
  return success;
}

boolean PluginTX_015(byte function, char *string) {
  boolean success=false;
  unsigned long bitstream = 0L;
  unsigned long preamble = 0L;
      #ifdef PLUGIN_TX_015_CORE

        //10;HomeEasy;7900b200;b;ON;
        //10;HomeEasy;d900ba00;23;OFF;
        //10;HomeEasy;79b2a5c3;b;ON;
        //01234567890123456789012345  
        if (strncasecmp(InputBuffer_Serial+3,"HOMEEASY;",9) == 0) { // KAKU Command eg. 
           if (InputBuffer_Serial[20] != ';') return success;
           byte cmd=0;
           byte group=0;
           InputBuffer_Serial[10]=0x30;
           InputBuffer_Serial[11]=0x78;                            // Get home from hexadecimal value 
           InputBuffer_Serial[20]=0x00;                            // Get home from hexadecimal value 
           bitstream=str2int(InputBuffer_Serial+10);               // Address
           if (InputBuffer_Serial[23] == ';') {
              cmd=str2cmd(InputBuffer_Serial+24);
           } else {
              cmd=str2cmd(InputBuffer_Serial+23);
           }
           if (cmd == VALUE_OFF)    cmd = 0;   // off
           if (cmd == VALUE_ON)     cmd = 1;   // on
           if (cmd == VALUE_ALLON) {cmd = 1; group=1;}   // allon
           if (cmd == VALUE_ALLOFF){cmd = 0; group=1;}   // alloff
           // ------------------------------
           byte address = 0;
           byte channel = 0;
           byte channelcode = 0;
           byte command = 0;
           byte i=1; // bitcounter in stream
           byte y; // size of partial bitstreams
           // ------------------------------
           address = (bitstream >> 4) & 0x7;      // 3 bits address (higher bits from HomeEasy address, bit 7 not used
           channel = bitstream & 0xF;             // 4 bits channel (lower bits from HomeEasy address
           command = cmd & 0xF;                   // 1 = on, 0 = off
           // ------------------------------
           if (channel == 0) channelcode = 0x8E;
           else if (channel == 1) channelcode = 0x96;
           else if (channel == 2) channelcode = 0x9A;
           else if (channel == 3) channelcode = 0x9C;
           else if (channel == 4) channelcode = 0xA6;
           else if (channel == 5) channelcode = 0xAA;
           else if (channel == 6) channelcode = 0xAC;
           else if (channel == 7) channelcode = 0xB2;
           else if (channel == 8) channelcode = 0xB4;
           else if (channel == 9) channelcode = 0xB8;
           else if (channel == 10) channelcode = 0xC6;
           else if (channel == 11) channelcode = 0xCA;
           else if (channel == 12) channelcode = 0xCC;
           else if (channel == 13) channelcode = 0xD2;
           else if (channel == 14) channelcode = 0xD4;
           else if (channel == 15) channelcode = 0xD8;
           //--------------- HOME EASY TRANSMIT ------------
           RawSignal.Multiply=50;
           // Startbits / Preamble
           y=11; // bit position from the preamble
           preamble = 0x63C;
           for (i=1;i<=22;i=i+2) {
               RawSignal.Pulses[i] = HomeEasy_ShortHigh/RawSignal.Multiply;
               if((preamble>>(y-1))&1)          // bit 1
                  RawSignal.Pulses[i+1] = HomeEasy_LongLow/RawSignal.Multiply;
               else                              // bit 0
                  RawSignal.Pulses[i+1] = HomeEasy_ShortLow/RawSignal.Multiply;
               y--;
           }
           // ------------------------------
           // Address 
           y=32; // bit position from the bitstream
           //bitstream = 0xDAB8F56C + address;
           for (i=23;i<=86;i=i+2) {
               RawSignal.Pulses[i] = HomeEasy_ShortHigh/RawSignal.Multiply;
               if((bitstream>>(y-1))&1)          // bit 1
                  RawSignal.Pulses[i+1] = HomeEasy_LongLow/RawSignal.Multiply;
               else                              // bit 0
                  RawSignal.Pulses[i+1] = HomeEasy_ShortLow/RawSignal.Multiply;
               y--;
           }
           // ------------------------------
           // Commands etc.
           y=15; // bit position from the bitstream
           bitstream = 0x5C00;  // bit 10 on, bit 11 off indien OFF
           if (cmd==0) bitstream = 0x5A00;  // cmd = off  
           if (group==1) bitstream = bitstream | 0xc0; // group on/off
           bitstream = bitstream + channelcode;

           for (i=87;i<=116;i=i+2) {
               RawSignal.Pulses[i] = HomeEasy_ShortHigh/RawSignal.Multiply;
               if((bitstream>>(y-1))&1)          // bit 1
                  RawSignal.Pulses[i+1] = HomeEasy_LongLow/RawSignal.Multiply;
               else                              // bit 0
                  RawSignal.Pulses[i+1] = HomeEasy_ShortLow/RawSignal.Multiply;
               y--;
           }

           RawSignal.Pulses[116]=0;
           RawSignal.Number=116;                    // aantal bits*2 die zich in het opgebouwde RawSignal bevinden  unsigned long bitstream=0L;
           RawSignal.Repeats=5;                     // vijf herhalingen.
           RawSignal.Delay=20;                      // Tussen iedere pulsenreeks enige tijd rust.
           RawSendRF();
           RawSignal.Multiply=25;
           //-----------------------------------------------
           success=true;
        }
  #endif // PLUGIN_015_CORE
  return success;
}
