//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                        Plugin-03: Kaku (ARC)                                      ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of sending and receiving the ARC protocol known from Klik-Aan-Klik-Uit (KAKU) 
 * transmitter, switch, PIR, door sensor etc.
 * Devices following the ARC protocol can often be recognized by their manual switch settings for 
 * the unit and house code number. Like for example by code wheel, dip switch or mini switch.
 *
 * This plugin also works with the following devices:
 * Princeton PT2262 / MOSDESIGN M3EB / Domia Lite / Klik-Aan-Klik-Uit / Intertechno / Sartano 2606. 
 * 
 * Author             : StuntTeam & Jonas Jespersen
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 ***********************************************************************************************
 * Address = A0..P16 according to KAKU adressing/notation
 ***********************************************************************************************
 * Het signaal bestaat drie soorten reeksen van vier pulsen, te weten: 
 * 0 = T,3T,T,3T, 1 = T,3T,3T,T, short 0 = T,3T,T,T Hierbij is iedere pulse (T) 350us PWDM
 *
 * KAKU Supports:
 *        on/off, waarbij de pulsreeks er als volgt uit ziet: 000x en x staat voor Off / On
 *    all on/off, waarbij de pulsreeks er als volgt uit ziet: 001x en x staat voor All Off / All On 
 ***********************************************************************************************
 * Brand               Model                   Chipset  
 * Intertechno         ITK200                  MDT10P61S
 * Profile             PN-47N                  SC5262   
 * Elro Home Comfort   AB-600MA                PT2262
 * Eurodomest          972080                  HS2303
 * Elro Home Control   AB-440 (Home Control)   HX2262
 * Elro Home Control   AB-4xx (Home Control)   HX2272
 * Profile             PN-44N                  SC2262
 * ProMax              RSL366T                 SC5262                   
 * Phenix              YC-4000S                HX2262
 ***********************************************************************************************
 * POSSIBLE BIT VARIATIONS: (Opposite order compared to used 'bitstream' format) 
 * ------------------------
 *                    1111 1111 011 1
                      010111101111 rev:
                      1111 0111 101 0    41+f=50 7 fixed 101 ?! 0=on/off
 *                    AAAA BBBB CCC D
 * KAKU:              A bit 0-3  = address 2^4 = 16 addresses
 * Intertechno ITK200 B bit 4-7  = device  2^4 = 16 devices
 *                    C bit 8-10 = fixed (011) 
 *                    D bit 11   = on/off command
 * -------------------
 *                    0 111 0000 1 000
 *                    A BBB CCCC D EEE
 *                    A/C/E      = 0
 * Blokker:           B bit 1-3  = device number 
 *                    D bit 8    = on/off command
 * -------------------
 *                    11111 11111 11
 *                    AAAAA BBBBB CC
 * Action:            A bit 0-4  = address 2^5 = 32 addresses
 * Impuls             B bit 5-9  = device  
 *                    C bit 10-11= on/off command
 * -------------------
 * Elro Home Easy     10001 10000 10
 *                    AAAAA BBBBB CC
 * Elro Home Control  A bit 0-4  = address 2^5 = 32 addresses
 *                    B bit 5-9  = device  
 * Brennenstuhl       C bit 10-11= on/off command (only 10 or 01)
 *    Comfort
 * -------------------
 *                    10100 00010 0 0 
 *                    AAAAA BBBBB C D
 * InterTechno        A bit 0-4  = address 2^5 = 32 addresses
 * Düwi Terminal      B bit 5-9  = device  
 * Cogex              C bit 10   = always 0 
 *                    D bit 11   = on/off command 
 * -------------------
 *                    11111 11111 1 1
 *                    AAAAA BBBBB C D
 * Sartano:           A bit 0-4  = address 2^5 = 32 addresses / Housecode
 *                    B bit 5-9  = device / Unitcode   11111 each bit corresponds to a button number 
 *                    C bit 10   = off command
 *                    D bit 11   = on command
 * Address = A0..`32 according to KAKU adressing/notation (Sartano uses 5 bit for both house and unit code. Therefore the ascii notation exceeds the alphabet)
 * ------------
 *
 * 20;DB;DEBUG;Pulses=50;Pulses(uSec)=425,1050,250,1025,250,1025,250,1025,250,1025,250,1025,250,1025,250,1025,250,1050,250,1025,250,1025,250,1025,250,1025,250,1025,250,1025,250,1025,250,1050,250,1025,250,1025,950,300,250,1050,950,300,250,1025,950,300,250; 
 *
 * KAKU Doorbell          010111101111
 * 20;07;DEBUG;Pulses=50;Pulses(uSec)=300,950,250,950,250,950,950,275,250,950,250,950,250,950,950,275,250,950,950,275,250,950,950,250,250,950,950,275,250,950,250,950,250,950,950,275,250,950,950,250,250,950,950,275,250,950,950,250,250;
 * 20;09;DEBUG;Pulses=50;Pulses(uSec)=3675,950,250,950,250,950,950,250,250,950,250,950,250,950,950,275,250,950,950,250,250,950,950,275,250,950,950,250,250,950,250,950,250,950,950,250,250,950,950,275,250,950,950,275,250,950,950,275,250;
 \*********************************************************************************************/
#define KAKU_CodeLength             12              // number of data bits
#define KAKU_T                     370              // 370? 350 us
#define KAKU_R                     300              // 370? 350 us
#define Sartano_T                  360              // 360 uS

#ifdef PLUGIN_003
boolean Plugin_003(byte function, char *string) {
      if (RawSignal.Number!=(KAKU_CodeLength*4)+2) return false; // conventionele KAKU bestaat altijd uit 12 data bits plus stop. Ongelijk, dan geen KAKU!
      int i,j;
      boolean error=false; 
      word bitstream=0;
      byte command=0;
      byte housecode=0;
      byte unitcode=0;
      int PTLow=22;                                // Pulse Time - lowest found value (22 = a pulse duration of 550)
      int PTHigh=22;                               // Pulse Time - highest found value
      byte signaltype=0;                           // 0 = Original Kaku

      for (i=0; i<KAKU_CodeLength; i++) {
        j=(KAKU_R*2)/RawSignal.Multiply;   
        
        if (RawSignal.Pulses[4*i+1]<j && RawSignal.Pulses[4*i+2]>j && RawSignal.Pulses[4*i+3]<j && RawSignal.Pulses[4*i+4]>j) { // 0101
            bitstream=(bitstream >> 1);  // 0
        } else 
        if (RawSignal.Pulses[4*i+1]<j && RawSignal.Pulses[4*i+2]>j && RawSignal.Pulses[4*i+3]>j && RawSignal.Pulses[4*i+4]<j) { // 0110
           bitstream=(bitstream >> 1 | (1 << (KAKU_CodeLength-1)));  // 1
        } else 
        if (RawSignal.Pulses[4*i+1]<j && RawSignal.Pulses[4*i+2]>j && RawSignal.Pulses[4*i+3]<j && RawSignal.Pulses[4*i+4]<j) { // 0100
           bitstream=(bitstream >> 1);   // Short 0, Group command on 2nd bit.
           command=2;
        } else {
          if (i==0) {                     // are we dealing with a RTK/AB600 device? then the first bit is sometimes mistakenly seen as 1101  
             if (RawSignal.Pulses[4*i+1]>j && RawSignal.Pulses[4*i+2]>j && RawSignal.Pulses[4*i+3]<j && RawSignal.Pulses[4*i+4]>j) { // 1101
                //bitstream=(bitstream >> 1 | (1 << (KAKU_CodeLength-1)));  // 1
                bitstream=(bitstream >> 1);  // 0
                //signaltype=1;
             } else { 
                error=true;
             } 
          } else {
             error=true;
          }
        }                                 // bad signal
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
      // ==========================================================================
      // TIMING DEBUG, this will find the shortest and longest pulse
      for (i=2;i<RawSignal.Number;i++) {            // skip first pulse as it is often affected by the start bit pulse duration
          if(RawSignal.Pulses[i] < PTLow) {         // short pulse?
             PTLow=RawSignal.Pulses[i];             // new value
          } else 
          if(RawSignal.Pulses[i] > PTHigh) {        // long pulse?
             PTHigh=RawSignal.Pulses[i];            // new value
          }
      }
      
      PTLow=PTLow*RawSignal.Multiply;               // get actual value
      PTHigh=PTHigh*RawSignal.Multiply;             // get actual value

      if( (PTLow == 225 ||  PTLow == 250) && (PTHigh == 900 ||  PTHigh == 925  ||  (PTHigh >= 1000  && PTHigh <= 1050)) ) signaltype=1; // ELRO 
      else
      if( (PTLow == 250 ||  PTLow == 275)    && ((PTHigh >=1000) && (PTHigh <=1050)) ) signaltype=2; // Profile PR44N
      else
      if( (PTLow == 325 ||  PTLow == 325)    && ((PTHigh >= 800) && (PTHigh <= 850)) ) signaltype=3; // Profile PR47N
      else
      if( ((PTLow >= 125) && (PTLow <= 300)) && ((PTHigh >= 775) && (PTHigh <= 925)) ) signaltype=4; // Sartano
      //else
      //if( (PTLow >= 50 ||  PTLow <= 75) && (PTHigh >= 525 ||  PTHigh <= 575) ) signaltype=4; // Energenie5
      //else
      //if( (PTLow == 225 ) && (PTHigh == 975) ) signaltype=4; // Energenie
      //else
      //if( (PTLow == 350 ) && (PTHigh == 1375) ) signaltype=4; // Philips SBC
      //sprintf(pbuffer, "T=%d %d/%d",signaltype,PTLow,PTHigh);     
      //Serial.println( pbuffer );
      //Serial.print(" ");
      //Serial.println(bitstream,BIN);
      // END OF TIMING DEBUG
      // ==========================================================================
      if (error==true) {                            // Error means that a pattern other than 0101/0110 was found
         //Serial.print("Kaku bitstream error : "); 
         //Serial.println(bitstream, BIN);
         return false;                              // This usually means we are dealing with a semi-compatible device 
      }                                             // that might have more states than used by ARC    
      //==================================================================================
      // Determine signal type
      //==================================================================================
      if (signaltype == 4) {                        // Sartano 
         // ----------------------------------      // Sartano 
         housecode = ((bitstream) & 0x01F);         // .......11111b
         unitcode = (((bitstream) & 0x3E0) >> 5);   // ..1111100000b
         housecode = ~housecode;                    // Sartano housecode is 5 bit ('A' - '`')
         housecode &= 0x0000001FL;                  // Translate housecode so that all jumpers off = 'A' and all jumpers on = '`'
         housecode += 0x41;
         switch(unitcode) {                         // Translate unit code into button number 1 - 5
            case 0x1E:                     // E=1110
                      unitcode = 1;
                      break;
            case 0x1D:                     // D=1101
                      unitcode = 2;
                      break;
            case 0x1B:                     // B=1011
                      unitcode = 3;
                      break;
            case 0x17:                     // 7=0111
                      unitcode = 4;
                      break;
            case 0x0F:                     // f=1111 
                      unitcode = 5;
                      break;
            default:
                     //Serial.print("Sartano:");
                     //Serial.println( unitcode );
                     signaltype=3;
                     break;
                     
         }
         if (signaltype == 4)
         {
             if ( ((bitstream >> 10) & 0x03) == 2) {
                 command = 1; // On
             } else if ( ((bitstream >> 10) & 0x03) == 1){
                 command = 0;// Off
             }
         }
      }      
      if (signaltype != 4) {                        // KAKU (and some compatibles for now)
         // ----------------------------------
         housecode=(((bitstream) &0x0f) +0x41);
         unitcode=((((bitstream) &0xf0) >> 4)+1);
         // ----------------------------------
         if ((bitstream&0x600)!=0x600) {
            //Serial.println("Kaku 0/1 error");
            return false;                           // use two static bits as checksum
         }
         if ( ((bitstream >> 11) & 0x01) == 1) {
            command=1;   // ON
         } else {
            command=0;   // OFF
         }
      }
      // ==========================================================================
      // Output
      // ----------------------------------
      sprintf(pbuffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
      Serial.print( pbuffer );
      // ----------------------------------
      if (signaltype < 4) {                         // KAKU (and some compatibles for now)
        Serial.print(F("Kaku;"));                   // Label
      } else {
        Serial.print(F("AB400D;"));                 // Label
      }
      sprintf(pbuffer, "ID=%02x;", housecode);       // ID    
      Serial.print( pbuffer );
      sprintf(pbuffer, "SWITCH=%d;", unitcode);     
      Serial.print( pbuffer );
      Serial.print(F("CMD="));                    
      if ( ((bitstream >> 11) & 0x01) == 1) {
         Serial.print(F("ON;"));
      } else {
         Serial.print(F("OFF;"));
      }
      Serial.println();
      // ----------------------------------
      RawSignal.Repeats=true;                    // suppress repeats of the same RF packet 
      RawSignal.Number=0;
      return true;
}
#endif //PLUGIN_003

#ifdef PLUGIN_TX_003
void Kaku_Send(word address, byte command);
void Sartano_Send(word address);

boolean PluginTX_003(byte function, char *string) {
        boolean success=false;
        word bitstream=0;
        byte command=0;
        uint32_t housecode = 0;
        uint32_t unitcode = 0;
        byte Home=0;                             // KAKU home A..P
        byte Address=0;                          // KAKU Address 1..16
        byte c=0;
        byte x=0;
        // ==========================================================================
        //10;Kaku;00004d;1;OFF;                     
        //012345678901234567890
        // ==========================================================================
        if (strncasecmp(InputBuffer_Serial+3,"KAKU;",5) == 0) { // KAKU Command eg. Kaku;A1;On
           if (InputBuffer_Serial[14] != ';') return false;
        
           x=15;                                    // character pointer
           InputBuffer_Serial[10]=0x30;
           InputBuffer_Serial[11]=0x78;             // Get home from hexadecimal value 
           InputBuffer_Serial[14]=0x00;             // Get home from hexadecimal value 
           Home=str2int(InputBuffer_Serial+10);     // KAKU home A is intern 0  
           if (Home < 0x51)                         // take care of upper/lower case
              Home=Home - 'A';
           else 
           if (Home < 0x71)                         // take care of upper/lower case
              Home=Home - 'a';
           else {
              return false;                       // invalid value
           }
           while((c=InputBuffer_Serial[x++])!=';'){ // Address: 1 to 16/32
              if(c>='0' && c<='9'){Address=Address*10;Address=Address+c-'0';}
           }

           //if (Address==0) {                        // group command is given: 0=all 
           //   command=2;                            // Set 2nd bit for group.
           //   bitstream=Home;
           //} else {
           //   bitstream= Home | ((Address-1)<<4);        
           //}

           bitstream= Home | ((Address-1)<<4);        
           command |= str2cmd(InputBuffer_Serial+x)==VALUE_ON; // ON/OFF command
           Kaku_Send(bitstream, command);
           success=true;
        // --------------- END KAKU SEND ------------
        } else
        // ==========================================================================
        //10;AB400D;00004d;1;OFF;                     
        //012345678901234567890
        // ==========================================================================
        if (strncasecmp(InputBuffer_Serial+3,"AB400D;",7) == 0) { // KAKU Command eg. Kaku;A1;On
           if (InputBuffer_Serial[16] != ';') return false;
           x=17;                                    // character pointer
           InputBuffer_Serial[12]=0x30;
           InputBuffer_Serial[13]=0x78;             // Get home from hexadecimal value 
           InputBuffer_Serial[16]=0x00;             // Get home from hexadecimal value 
           Home=str2int(InputBuffer_Serial+12);     // KAKU home A is intern 0  
           if (Home < 0x61)                         // take care of upper/lower case
              Home=Home - 'A';
           else 
           if (Home < 0x81)                         // take care of upper/lower case
              Home=Home - 'a';
           else {
              return false;                       // invalid value
           }
           while((c=InputBuffer_Serial[x++])!=';'){ // Address: 1 to 16/32
              if(c>='0' && c<='9'){Address=Address*10;Address=Address+c-'0';}
           }
           command = str2cmd(InputBuffer_Serial+x)==VALUE_ON; // ON/OFF command
           housecode = ~Home;
           housecode &= 0x001F;
           unitcode=Address;
           if ((unitcode  >= 1) && (unitcode <= 5) ) {
              bitstream = housecode & 0x01F;
              if (unitcode == 1) bitstream |= 0x3C0;
              else if (unitcode == 2) bitstream |= 0x3A0;
              else if (unitcode == 3) bitstream |= 0x360;
              else if (unitcode == 4) bitstream |= 0x2E0;
              else if (unitcode == 5) bitstream |= 0x1E0;

              if (command) bitstream |= 0x800;
              else bitstream |= 0x400;
           }
           Sartano_Send(bitstream);
           success=true;
        }
        // --------------- END SARTANO SEND ------------
        return success;
}

void Kaku_Send(word bitstream, byte command) { 
     RawSignal.Multiply=KAKU_T/10;
     RawSignal.Repeats=7;                             // KAKU heeft minimaal vijf herhalingen nodig om te schakelen.
     RawSignal.Delay=20;                              // Tussen iedere pulsenreeks enige tijd rust.
     RawSignal.Number=KAKU_CodeLength*4+2;            // Lengte plus een stopbit

     //unsigned long Bitstream 
     bitstream = bitstream | (0x600 | ((command & 1 /*Commando*/) << 11)); // Stel een bitstream samen
     // loop de 12-bits langs en vertaal naar pulse/space signalen.  
     for (byte i=0; i<KAKU_CodeLength; i++) {
         RawSignal.Pulses[4*i+1]=10;
         RawSignal.Pulses[4*i+2]=30;
    
         if (((command >> 1)&1) /* Group */ && i>=4 && i<8) {
            RawSignal.Pulses[4*i+3]=10;
            RawSignal.Pulses[4*i+4]=10;
         } else { // short 0
            if ((bitstream>>i)&1) { // 1
                     RawSignal.Pulses[4*i+3]=30;
                     RawSignal.Pulses[4*i+4]=10;
            } else { //0
                     RawSignal.Pulses[4*i+3]=10;
                     RawSignal.Pulses[4*i+4]=30;
            }
        }
    }
    // Stopbit
    RawSignal.Pulses[4*KAKU_CodeLength+1]=KAKU_T/RawSignal.Multiply;
    RawSignal.Pulses[4*KAKU_CodeLength+2]=KAKU_T/RawSignal.Multiply;

    RawSendRF();
    RawSignal.Multiply=RAWSIGNAL_SAMPLE_RATE; 
}

void Sartano_Send(word bitstream) {
    RawSignal.Multiply = Sartano_T / 10;
    RawSignal.Repeats=7;
    RawSignal.Delay=11;
    RawSignal.Number=KAKU_CodeLength*4+2;

    // loop de 12-bits langs en vertaal naar pulse/space signalen.  
    for (byte i=0; i<KAKU_CodeLength; i++) {
        RawSignal.Pulses[4*i+1]=10;
        RawSignal.Pulses[4*i+2]=30;

        if ((bitstream >> i) & 1) { // 0
           RawSignal.Pulses[4*i+3]=30;
           RawSignal.Pulses[4*i+4]=10;
        } else { //1
           RawSignal.Pulses[4*i+3]=10;
           RawSignal.Pulses[4*i+4]=30;
        }
    }
    // Stopbit
    RawSignal.Pulses[4*KAKU_CodeLength+1]=10;

    RawSendRF();
    RawSignal.Multiply=RAWSIGNAL_SAMPLE_RATE;
}
#endif //PLUGIN_TX_003
