//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                        Plugin-02: Kaku (ARC)                                      ##
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
 ***********************************************************************************************
 * Sartano 2606: L=150-300(325)  H=775-925
 * 20;3F;DEBUG;Pulses=50;Pulses(uSec)=350,825,900,200,275,825,900,200,250,825,900,200,250,850,875,200,250,850,875,200,250,850,875,225,250,850,875,225,225,850,875,225,225,875,225,850,225,850,875,225,225,850,250,850,250,850,875,225,225;

 20;02;DEBUG;Pulses=50;Pulses(uSec)=325,825,875,225,225,875,850,225,225,875,225,875,225,875,225,850,225,875,225,875,225,850,225,850,225,875,825,250,225,875,850,225,225,875,825,250,200,900,825,275,200,900,200,900,200,900,825,250,200;
 20;25;DEBUG;Pulses=50;Pulses(uSec)=350,800,900,175,275,825,875,200,250,825,250,825,250,825,275,825,275,825,250,825,275,825,275,825,250,825,875,200,250,825,875,200,250,850,875,200,250,850,875,200,250,850,875,225,225,850,250,850,225;
 20;1D;DEBUG;Pulses=50;Pulses(uSec)=375,800,925,150,275,800,900,175,275,825,275,825,275,825,275,825,275,825,275,800,275,800,900,175,275,825,275,825,275,825,900,200,250,825,875,200,250,850,875,200,250,850,250,850,250,850,875,200,250;
 20;46;DEBUG;Pulses=50;Pulses(uSec)=375,775,925,150,300,800,925,150,300,800,300,800,275,800,275,800,300,800,300,800,275,800,925,175,275,825,275,800,275,800,900,175,275,825,900,200,275,825,875,200,250,825,875,200,250,850,250,850,250;
 20;89;DEBUG;Pulses=50;Pulses(uSec)=375,800,925,175,275,800,900,175,275,825,275,825,275,825,275,825,275,825,275,825,275,825,900,175,275,825,900,200,250,850,250,825,275,825,875,200,250,850,875,200,250,850,250,850,250,850,875,200,250;
 20;B6;DEBUG;Pulses=50;Pulses(uSec)=375,800,925,150,275,800,925,175,275,825,275,825,275,825,275,825,275,825,275,800,275,800,900,175,275,825,900,200,250,825,250,825,250,825,900,200,250,825,875,200,250,850,875,200,250,850,250,850,250;
 20;F2;DEBUG;Pulses=50;Pulses(uSec)=375,775,925,150,300,800,925,175,275,825,275,825,275,825,275,825,275,825,275,800,275,800,900,175,275,825,900,200,275,825,875,200,250,850,250,825,250,825,875,200,250,850,250,850,250,825,875,200,250;
 20;1E;DEBUG;Pulses=50;Pulses(uSec)=375,775,925,150,300,800,925,175,275,825,275,825,275,825,275,825,275,800,275,825,275,800,900,175,275,825,900,200,250,825,875,200,250,850,250,825,250,825,875,200,250,850,875,200,250,850,250,850,250;
 20;F9;DEBUG;Pulses=50;Pulses(uSec)=350,800,900,175,275,825,900,200,250,825,900,200,250,850,250,825,250,825,250,825,250,825,275,825,250,825,875,200,250,850,875,200,250,850,875,200,250,850,875,200,250,850,250,850,225,850,875,225,225;
 20;16;DEBUG;Pulses=50;Pulses(uSec)=350,800,900,175,275,825,900,200,250,850,875,200,250,850,250,825,250,825,250,825,250,825,250,825,275,825,900,200,250,850,875,200,250,850,875,200,225,850,875,225,250,850,875,200,225,850,250,850,225;
 20;29;DEBUG;Pulses=50;Pulses(uSec)=350,800,900,175,275,825,900,175,275,825,900,200,250,850,250,825,250,825,250,850,250,825,875,200,250,850,250,825,250,825,875,200,250,850,875,200,250,850,875,200,250,850,250,850,250,850,875,200,250;
 20;55;DEBUG;Pulses=50;Pulses(uSec)=350,800,900,175,275,825,900,200,250,825,875,200,250,850,250,825,250,825,250,850,250,825,875,200,250,850,250,850,250,825,875,200,250,850,875,225,250,850,875,200,250,850,875,225,225,850,225,850,250;
 20;80;DEBUG;Pulses=50;Pulses(uSec)=350,825,900,200,275,825,900,200,250,825,875,200,250,850,250,850,250,825,250,825,300,800,875,200,275,800,875,200,250,850,250,850,250,850,875,200,250,850,875,225,225,850,250,850,250,850,875,225,225;
 20;B6;DEBUG;Pulses=50;Pulses(uSec)=350,800,900,175,275,825,900,200,275,825,875,200,250,825,250,825,250,825,250,825,250,825,875,200,250,850,875,200,250,850,250,850,250,850,875,200,250,850,875,225,250,850,875,225,225,850,225,850,250;
 20;CD;DEBUG;Pulses=50;Pulses(uSec)=350,800,900,175,275,825,900,200,250,825,900,200,250,850,250,850,250,825,250,825,250,825,900,200,250,850,875,200,250,850,875,200,225,850,250,850,250,850,875,225,225,850,250,850,250,850,875,200,250;
 20;F0;DEBUG;Pulses=50;Pulses(uSec)=350,800,900,175,275,825,900,200,275,825,900,200,250,825,250,850,250,825,275,825,275,825,900,200,250,850,875,200,250,850,875,200,250,850,250,850,250,850,875,225,250,850,875,200,250,850,250,850,225;  
 
 * 0110 0110 0110 0110 0110 0110 0110 0110 0101 0110 0101 0110 0     11111 11101 01
 * Promax RSL366T: L=325-375(425) H=1225-1275   [325/1275] [325/350/1275/1300]
 * 20;0A;DEBUG;Pulses=50;Pulses(uSec)=425,1275,325,1275,350,1275,1225,375,325,1275,1225,350,350,1275,1225,375,350,1275,350,1275,350,1275,1225,350,350,1275,1225,375,325,1275,1225,350,325,1275,1225,350,350,1275,1225,375,325,1275,1225,350,350,1275,1225,375,350;
   20;A5;DEBUG;Pulses=50;Pulses(uSec)=400,1275,350,1275,325,1275,1225,350,350,1275,1225,350,325,1275,1225,375,325,1275,325,1275,350,1250,1225,375,325,1275,1225,350,350,1275,1225,350,350,1275,1225,375,325,1275,1225,350,350,1275,1225,375,325,1275,350,1275,350;
   20;A6;Kaku;ID=4f;SWITCH=15;CMD=OFF;

 * FA500/Elro   [250/925]
 * 20;40;DEBUG;Pulses=50;Pulses(uSec)=350,850,275,850,275,850,275,850,275,850,275,850,275,850,275,875,300,875,275,900,275,875,300,875,300,875,275,900,300,875,900,250,300,875,275,875,300,875,900,250,275,875,900,250,300,875,300,875,275;

 * 0101 0110 0110 0110 0101 0110 0110 0110 0110 0110 0110 0110 0     0111 0111 1111
 * profile 44N: L=275(350) H=1025-1050   [275/1050]
 * 20;14;DEBUG;Pulses=50;Pulses(uSec)=350,1050,1025,275,275,1050,1025,275,275,1050,1025,275,275,1050,1025,275,275,1050,275,1050,275,1050,275,1050,275,1050,1025,275,275,1050,1025,275,275,1050,275,1050,275,1050,1025,275,275,1050,1025,275,275,1050,275,1050,275;
   20;04;DEBUG;Pulses=50;Pulses(uSec)=350,1050,1025,275,275,1050,1025,275,275,1050,1025,275,275,1050,1025,275,275,1050,275,1050,275,1050,275,1050,275,1050,1025,275,275,1050,1025,275,275,1050,275,1050,275,1050,1025,275,275,1050,1025,275,275,1050,275,1050,275;
   20;05;Kaku;ID=50;SWITCH=13;CMD=OFF;
 
 * 0110 0110 0110 0110 0101 0101 0110 0110 0101 0110 0110 0101 0     1111 0011 0110
 * profile 47N  L=150-250 H=750-850(1725)
 * 20;6B;DEBUG;Pulses=50;Pulses(uSec)=1725,850,750,225,150,825,750,250,150,825,750,225,150,850,750,225,150,850,150,825,150,825,150,825,175,825,750,225,150,825,750,225,150,825,150,825,175,825,750,225,175,825,750,225,150,825,150,850,150;
 * 20;1B;DEBUG;Pulses=50;Pulses(uSec)=225,825,750,225,175,825,750,225,150,825,750,225,175,825,750,225,175,825,175,825,175,825,175,825,175,825,750,225,175,825,750,225,175,825,175,825,175,825,750,225,175,825,750,225,150,825,175,825,175;
   20;72;DEBUG;Pulses=50;Pulses(uSec)=250,850,750,225,150,825,750,225,175,825,750,225,175,825,750,225,175,825,175,825,175,825,150,825,150,825,175,825,175,825,175,825,175,825,175,825,175,825,750,225,150,825,750,225,175,825,175,825,175;
   20;73;Eurodomest;ID=0aaffe;SWITCH=05;CMD=ALLON;

 * 0110 0110 0110 0110 0101 0101 0110 0110 0101 0110 0110 0101 0     1111 0011 0110 
 * Intertechno ITK200: L=250-300(350) H=950-1050     [250/1050]
 * 20;74;DEBUG;Pulses=50;Pulses(uSec)=350,1050,250,1025,250,1025,250,1025,250,1025,250,1025,250,1025,250,1025,250,1050,250,1025,250,1025,250,1025,250,1050,250,1025,250,1025,250,1025,250,1025,250,1025,250,1025,950,300,250,1050,950,300,250,1025,250,1025,250;
 * 0101 0101 0101 0101 0101 0101 0101 0101 0101 0110 0110 0101 0     0000 0000 0110 
 * Elro AB600: L=225-250(325) H= 900-925
 * 20;81;DEBUG;Pulses=50;Pulses(uSec)=325,925,225,925,225,925,225,925,225,925,225,925,225,925,225,925,225,925,900,250,225,900,900,250,225,925,900,250,225,925,900,250,225,925,225,925,225,900,900,250,225,900,900,250,225,900,900,225,225;
 * 0101 0101 0101 0101 0110 0110 0110 0110 0101 0110 0110 0110 0     0000 1111 0111 
 * Eurodomest (Non-ARC) [200/825]
 * 20;9D;DEBUG;Pulses=50;Pulses(uSec)=1250,200,750,175,200,750,200,750,750,200,200,750,200,750,750,200,200,750,750,200,750,200,200,750,750,200,200,750,200,750,750,200,200,750,750,200,200,750,200,750,750,200,750,200,750,200,750,200,200;
 * 1010 0101 1001 0110 0110 1001 1001 0110 0110 0101 1010 1010 0     2031 1331 1022 
 * KAKU original: L=225-375   H=950-1050
 * 20;39;DEBUG;Pulses=50;Pulses(uSec)=375,975,1000,250,300,975,1000,250,300,1000,300,975,300,975,300,1000,300,1000,300,975,275,975,300,975,275,1000,275,1000,300,975,300,975,275,1000,300,975,275,1000,975,275,275,1000,1000,275,275,1000,975,275,275;
 * 20;3A;DEBUG;Pulses=50;Pulses(uSec)=325,1025,975,300,250,1025,950,300,250,1025,250,1025,250,1025,250,1025,250,1025,275,1025,275,1000,275,1025,275,1025,250,1025,275,1025,250,1025,250,1025,275,1025,250,1025,975,300,250,1025,975,300,250,1025,975,300,250;
 * 20;3B;DEBUG;Pulses=50;Pulses(uSec)=325,1025,950,300,250,1025,975,300,250,1050,250,1025,250,1025,250,1050,250,1025,250,1025,225,1025,250,1025,250,1050,225,1050,250,1050,250,1025,250,1050,250,1050,250,1025,950,325,250,1050,950,300,250,1050,950,300,225;
 * 20;3C;DEBUG;Pulses=50;Pulses(uSec)=350,1025,975,300,275,1000,950,300,250,1025,250,1025,275,1025,250,1025,275,1025,250,1025,275,1025,250,1025,250,1025,250,1025,275,1025,250,1025,275,1025,250,1025,275,1025,975,300,250,1025,975,300,250,1025,975,300,275;
 * 20;3D;DEBUG;Pulses=50;Pulses(uSec)=325,1025,975,300,250,1025,975,300,250,1025,250,1025,250,1025,250,1025,250,1025,250,1025,275,1000,275,1025,250,1025,275,1025,275,1025,250,1025,250,1025,250,1025,275,1025,975,300,250,1025,975,300,275,1000,975,275,275;
 * 20;3E;DEBUG;Pulses=50;Pulses(uSec)=350,1025,975,300,250,1025,950,300,250,1050,250,1025,250,1025,250,1025,250,1025,250,1025,250,1025,250,1025,250,1025,275,1025,250,1025,275,1025,250,1025,250,1025,250,1025,975,300,250,1025,975,300,250,1025,975,300,250;
 * 20;06;DEBUG;Pulses=50;Pulses(uSec)=275,875,200,850,200,850,200,850,200,850,200,850,200,850,200,850,200,850,200,850,200,850,200,850,200,850,200,850,200,850,200,850,200,850,200,850,200,850,825,225,200,850,825,225,200,850,200,850,200;
 \*********************************************************************************************/
#define KAKU_CodeLength             12              // number of data bits
#define KAKU_T                     370              // 370? 350 us
#define KAKU_R                     300              // 370? 350 us
#define Sartano_T                  300              // 300 uS

void Kaku_Send(unsigned long address, byte command);
void Sartano_Send(unsigned long address);

boolean Plugin_002(byte function, char *string) {
  boolean success=false;

    #ifdef PLUGIN_002_CORE
      // ==========================================================================
      if (RawSignal.Number!=(KAKU_CodeLength*4)+2) return false; // conventionele KAKU bestaat altijd uit 12 data bits plus stop. Ongelijk, dan geen KAKU!
      int i,j;
      boolean error=false; 
      unsigned long bitstream=0L;
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
      // TIMING DEBUG
      for (i=2;i<RawSignal.Number;i++) {            // skip first pulse as it is affected by the start bit
          if(RawSignal.Pulses[i] < PTLow) {         // short pulse?
             PTLow=RawSignal.Pulses[i];             // new value
          } else 
          if(RawSignal.Pulses[i] > PTHigh) {        // long pulse?
             PTHigh=RawSignal.Pulses[i];            // new value
          }
      }
      // seen as eurodomest ?!
      //250-300 850-900 
      //20;40;DEBUG;Pulses=50;Pulses(uSec)=350,850,275,850,275,850,275,850,275,850,275,850,275,850,275,875,300,875,275,900,275,875,300,875,300,875,275,900,300,875,900,250,300,875,275,875,300,875,900,250,275,875,900,250,300,875,300,875,275;

     // 150-250 750-850  150-850
     //20;7C;DEBUG;Pulses=50;Pulses(uSec)=275,850,750,250,150,825,750,250,150,850,750,225,150,850,750,225,150,850,150,850,150,850,150,850,150,850,150,850,150,850,150,850,150,850,150,825,150,850,750,250,150,850,750,250,150,850,150,850,150;
     //20;7D;Eurodomest;ID=0aaffe;SWITCH=05;CMD=ALLON;

     // 275 1025-1050
     //20;A7;DEBUG;Pulses=50;Pulses(uSec)=350,1050,1025,275,275,1050,1025,275,275,1050,1025,275,275,1050,1025,275,275,1050,1025,275,275,1050,1025,275,275,1050,1025,275,275,1050,1025,275,275,1050,275,1050,275,1050,1025,275,275,1050,1025,275,275,1050,1025,275,275;
     //20;A8;AB400D;ID=41;SWITCH=4;CMD=ON;

      // profile not seen at all
      // 275-300 1025-1050   > 275 - 1050
      //20;1B;DEBUG;Pulses=50;Pulses(uSec)=350,1050,1025,275,275,1050,1025,275,275,1050,1025,275,275,1050,1025,275,275,1050,275,1050,275,1050,300,1050,275,1050,1025,275,300,1050,1025,275,300,1050,275,1050,275,1050,1025,275,275,1050,1025,275,275,1050,1025,275,275;
      
      
      PTLow=PTLow*RawSignal.Multiply;               // get actual value
      PTHigh=PTHigh*RawSignal.Multiply;             // get actual value

      if( (PTLow == 225 ||  PTLow == 250) && (PTHigh == 900 ||  PTHigh == 925) ) signaltype=1; // ELRO 
      else
      if( (PTLow == 225 ||  PTLow == 250) && (PTHigh == 1050) ) signaltype=0; // ITK200
      else
      if( (PTLow == 275 ||  PTLow == 275) && (PTHigh == 1050 ||  PTHigh == 1050) ) signaltype=2; // Profile PR44N
      else
      if( (PTLow == 325 ||  PTLow == 325) && (PTHigh >= 800 ||  PTHigh <= 850) ) signaltype=3; // Profile PR47N
      else
      if( (PTLow >= 150 ||  PTLow <= 300) && (PTHigh >= 775 ||  PTHigh <= 925) ) signaltype=4; // Sartano
      
      //sprintf(pbuffer, "T=%d %d/%d",signaltype,PTLow,PTHigh);     
      //Serial.println( pbuffer );
      //Serial.print(" ");
      //Serial.println(bitstream,BIN);
      // END OF TIMING DEBUG
      // ==========================================================================
      if (error==true) {                            // Error means that a pattern other than 0101/0110 was found
      
/*      
20;30;DEBUG;Pulses=50;Pulses(uSec)=
200,800,725,200,175,775,725,200,175,775,725,200,175,775,750,225,175,825,175,825,175,825,175,825,175,825,175,825,175,825,175,825,175,825,175,825,175,825,775,225,175,825,775,225,175,825,775,225,175;
T=4 175/825
Kaku bitstream error : 111000001000
20;31;DEBUG;Pulses=50;Pulses(uSec)=300,800,725,200,175,775,725,200,175,800,725,200,175,800,775,225,175,825,175,825,175,825,175,825,175,825,175,825,175,825,175,825,175,825,175,825,175,825,775,225,175,825,775,225,175,825,775,200,175;
20;32;DEBUG;Pulses=50;Pulses(uSec)=
225,825,775,200,175,825,775,225,175,825,750,225,175,825,750,225,750,225,775,225,775,200,775,200,175,825,175,825,175,800,175,825,775,225,775,225,175,825,775,200,175,825,750,225,175,825,775,225,175;

200,800,725,200, 0110
175,775,725,200,
175,775,725,200,
175,775,750,225,
175,825,175,825, 0101
175,825,175,825,
175,825,175,825,
175,825,175,825,
175,825,175,825,
175,825,775,225,
175,825,775,225,
175,825,775,225, 0110
175;
111100000111


225,825,775,200, 0110
175,825,775,225,
175,825,750,225,
175,825,750,225,
750,225,775,225,
775,200,775,200, 1010
175,825,175,825, 0101
175,800,175,825, 
775,225,775,225, 
175,825,775,200,
175,825,750,225,
175,825,775,225,
175;
T=4 175/825
Kaku bitstream error : 111001111000

      
Kaku bitstream error : 11000001100
T=4 175/825
sig=3
20;03;Kaku;ID=50;SWITCH=1;CMD=OFF;
T=4 175/800
Kaku bitstream error : 111001111000


20;AF;DEBUG;Pulses=50;Pulses(uSec)=325,825,750,225,175,825,750,225,175,825,750,225,175,825,750,225,750,225,750,225,750,225,750,225,175,825,750,225,175,825,750,225,775,225,775,225,175,825,750,225,175,825,750,225,175,825,750,225,175;
T=4 175/825
Kaku bitstream error : 111111111000
20;B0;Eurodomest;ID=0aa0a2;SWITCH=05;CMD=ALLOFF;


325,825,750,225, 0110
175,825,750,225,
175,825,750,225,
175,825,750,225,
750,225,750,225,1010
750,225,750,225,
175,825,750,225,0110
175,825,750,225,
775,225,775,225,1010
175,825,750,225,0110
175,825,750,225,
175,825,750,225,
175;

1111xx11x111

*/      
         //Serial.print("Kaku bitstream error : "); 
         //Serial.println(bitstream, BIN);
         return false;                              // This usually means we are dealing with a semi-compatible device 
      }                                             // that might have more states than used by ARC    
      //==================================================================================
      // Determine signal type
      //==================================================================================
      if (signaltype == 4) {                        // Sartano 
         // ----------------------------------      // Sartano 
         housecode = ((bitstream) & 0x0000001FL);       // .......11111b
         unitcode = (((bitstream) & 0x000003E0L) >> 5); // ..1111100000b
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
                     //return false;
                     signaltype=3;
                     //Serial.println("sig=3");
                     break;
                     
         }	  
         if (signaltype == 4) {                        // Sartano 
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
      success=true;
    #endif //PLUGIN_CORE_002
  return success;
}

boolean PluginTX_002(byte function, char *string) {
    boolean success=false;
    unsigned long bitstream=0L;
    byte command=0;
    uint32_t housecode = 0;
    uint32_t unitcode = 0;
    byte Home=0;                             // KAKU home A..P
    byte Address=0;                          // KAKU Address 1..16
    byte c=0;
    byte x=0;

    #ifdef PLUGIN_TX_002_CORE
        // ==========================================================================
        //10;Kaku;00004d;1;OFF;                     
        //012345678901234567890
        // ==========================================================================
        if (strncasecmp(InputBuffer_Serial+3,"KAKU;",5) == 0) { // KAKU Command eg. Kaku;A1;On
           if (InputBuffer_Serial[14] != ';') return success;
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
              return success;                       // invalid value
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
           if (InputBuffer_Serial[16] != ';') return success;
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
              return success;                       // invalid value
           }
           while((c=InputBuffer_Serial[x++])!=';'){ // Address: 1 to 16/32
              if(c>='0' && c<='9'){Address=Address*10;Address=Address+c-'0';}
           }
           command = str2cmd(InputBuffer_Serial+x)==VALUE_ON; // ON/OFF command
           housecode = ~Home;
           housecode &= 0x0000001FL;
           unitcode=Address;
           if ((unitcode  >= 1) && (unitcode <= 5) ) {
              bitstream = housecode & 0x0000001FL;
              if (unitcode == 1) bitstream |= 0x000003C0L;
              else if (unitcode == 2) bitstream |= 0x000003A0L;
              else if (unitcode == 3) bitstream |= 0x00000360L;
              else if (unitcode == 4) bitstream |= 0x000002E0L;
              else if (unitcode == 5) bitstream |= 0x000001E0L;

              if (command) bitstream |= 0x00000800L;
              else bitstream |= 0x00000400L;					
           }
           Sartano_Send(bitstream);
           success=true;
        }
        // --------------- END SARTANO SEND ------------
    #endif //PLUGIN_TX_CORE_002
  return success;
}

#ifdef PLUGIN_TX_002_CORE
void Kaku_Send(unsigned long bitstream, byte command) { 
     RawSignal.Multiply=50;
     RawSignal.Repeats=7;                             // KAKU heeft minimaal vijf herhalingen nodig om te schakelen.
     RawSignal.Delay=20;                              // Tussen iedere pulsenreeks enige tijd rust.
     RawSignal.Number=KAKU_CodeLength*4+2;            // Lengte plus een stopbit

     //unsigned long Bitstream 
     bitstream = bitstream | (0x600 | ((command & 1 /*Commando*/) << 11)); // Stel een bitstream samen
     // loop de 12-bits langs en vertaal naar pulse/space signalen.  
     for (byte i=0; i<KAKU_CodeLength; i++) {
         RawSignal.Pulses[4*i+1]=KAKU_T/RawSignal.Multiply;
         RawSignal.Pulses[4*i+2]=(KAKU_T*3)/RawSignal.Multiply;
    
         if (((command >> 1)&1) /* Group */ && i>=4 && i<8) {
            RawSignal.Pulses[4*i+3]=KAKU_T/RawSignal.Multiply;
            RawSignal.Pulses[4*i+4]=KAKU_T/RawSignal.Multiply;
         } else { // short 0
            if ((bitstream>>i)&1) { // 1
                     RawSignal.Pulses[4*i+3]=(KAKU_T*3)/RawSignal.Multiply;
                     RawSignal.Pulses[4*i+4]=KAKU_T/RawSignal.Multiply;
            } else { //0
                     RawSignal.Pulses[4*i+3]=KAKU_T/RawSignal.Multiply;
                     RawSignal.Pulses[4*i+4]=(KAKU_T*3)/RawSignal.Multiply;          
            }          
        }
        // Stopbit
        RawSignal.Pulses[4*KAKU_CodeLength+1]=KAKU_T/RawSignal.Multiply;
        RawSignal.Pulses[4*KAKU_CodeLength+2]=KAKU_T/RawSignal.Multiply;
    }
    RawSendRF();
    RawSignal.Multiply=25;
}

void Sartano_Send(unsigned long address) { 
    int fpulse = 360;                               // Pulse witdh in microseconds
    int fretrans = 7;                               // Number of code retransmissions
    uint32_t fdatabit;
    uint32_t fdatamask = 0x00000001;
    uint32_t fsendbuff;
	
    digitalWrite(PIN_RF_RX_VCC,LOW);                // Turn off power to the RF receiver 
    digitalWrite(PIN_RF_TX_VCC,HIGH);               // Enable the 433Mhz transmitter
    delayMicroseconds(TRANSMITTER_STABLE_DELAY);    // short delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms)

    for (int nRepeat = 0; nRepeat <= fretrans; nRepeat++) {
        fsendbuff = address;
        // Send command
        
		for (int i = 0; i < 12; i++) {              // Sartano packet is 12 bits 
            // read data bit
            fdatabit = fsendbuff & fdatamask;       // Get most right bit
            fsendbuff = (fsendbuff >> 1);           // Shift right

			// PT2262 data can be 0, 1 or float. Only 0 and float is used by Sartano
            if (fdatabit != fdatamask) {            // Write 0
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse);
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse * 3);
				digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse);
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse * 3);
            } else {                                // Write float
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse * 1);
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse * 3);
				digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse * 3);
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse * 1);
            }
        }
		// Send sync bit
        digitalWrite(PIN_RF_TX_DATA, HIGH);         
        delayMicroseconds(fpulse * 1);
        digitalWrite(PIN_RF_TX_DATA, LOW);          // and lower the signal
        delayMicroseconds(fpulse * 31);
    }
    delayMicroseconds(TRANSMITTER_STABLE_DELAY);    // short delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms)
    digitalWrite(PIN_RF_TX_VCC,LOW);                // Turn thew 433Mhz transmitter off
    digitalWrite(PIN_RF_RX_VCC,HIGH);               // Turn the 433Mhz receiver on
    RFLinkHW();
}
#endif //PLUGIN_CORE_002
