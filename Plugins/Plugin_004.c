//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                       Plugin-04: NewKAKU                                          ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of receiving from and transmitting to "Klik-Aan-Klik-Uit" devices
 * working according to the learning code system. This protocol is also used by the Home Easy devices.
 * It includes direct DIM functionality.
 * 
 * Author             : StuntTeam
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 ***********************************************************************************************
 * Pulse (T) is 275us PDM
 * 0 = T,T,T,4T, 1 = T,4T,T,T, dim = T,T,T,T op bit 27
 *
 * NewKAKU supports:
 *   on/off       ---- 000x Off/On
 *   all on/off   ---- 001x AllOff/AllOn
 *   dim absolute xxxx 0110 Dim16        // dim on bit 27 + 4 extra bits for dim level
 *
 *  NewKAKU bitstream= (First sent) AA AAAAAAAA AAAAAAAA AAAAAAAACCUUUU(LLLL) -> A=KAKU_address, C=command, U=KAKU-Unit, L=extra dimlevel bits (optional)
 *
 * Sample RF packet:
 * 20;B8;NewKaku;ID=00c142;SWITCH=1;CMD=OFF;
 * 20;B9;DEBUG;Pulses=132;Pulses(uSec)=200,2550,150,200,125,1200,150,200,150,1200,125,1200,150,225,125,1200,125,225,125,200,150,1200,150,200,150,1200,150,1200,125,200,125,200,125,1225,125,1200,125,225,150,200,150,1200,150,1200,150,200,150,1200,150,225,125,200,150,1200,125,200,150,1200,125,200,150,1200,150,200,125,1225,150,200,125,1200,150,1200,125,225,125,200,125,1200,150,1200,125,225,125,200,125,1225,125,200,125,1225,125,200,125,1200,125,200,150,1225,125,1200,150,200,125,200,125,1200,125,200,150,1200,125,200,125,1200,150,200,125,1200,125,200,125,1200,150,200,125,1200,150,200,150,1200,125;
 * 20;06;DEBUG;Pulses=132;Pulses(uSec)=175,2575,150,200,150,1200,150,200,150,1200,150,1200,150,200,125,1200,150,200,125,200,150,1200,125,200,150,1200,150,1200,150,200,150,200,150,1225,150,1200,125,225,150,200,125,1200,150,1200,150,200,150,1200,150,200,150,200,125,1225,125,200,150,1200,125,200,150,1200,125,200,150,1200,150,200,150,1200,150,1200,125,200,150,200,125,1200,150,1200,125,225,150,200,125,1200,150,200,150,1200,150,200,150,1200,150,200,150,1225,125,1200,150,200,125,200,150,1200,150,200,125,1200,150,200,150,1200,150,200,150,1200,150,200,125,1225,125,200,125,1200,150,200,150,1200,150
 \*********************************************************************************************/
#define NewKAKU_RawSignalLength      132        // regular KAKU packet length
#define NewKAKUdim_RawSignalLength   148        // KAKU packet length including DIM bits
#define NewKAKU_1T                   225        // 275        // us
#define NewKAKU_mT                   650/RAWSIGNAL_SAMPLE_RATE // us, approx. in between 1T and 4T 
#define NewKAKU_4T                  1225        // 1100       // us
#define NewKAKU_8T                  2600        // 2200       // us, Duration of the space after the start bit

#ifdef PLUGIN_004
boolean Plugin_004(byte function, char *string) {
      // nieuwe KAKU bestaat altijd uit start bit + 32 bits + evt 4 dim bits. Ongelijk, dan geen NewKAKU
      if ( (RawSignal.Number != NewKAKU_RawSignalLength) && (RawSignal.Number != NewKAKUdim_RawSignalLength) ) return false;
      boolean Bit;
      int i;
      //int P0,P1,P2,P3;
      byte P0,P1,P2,P3;
      byte dim=0;
      unsigned long bitstream=0L;
      
      // RawSignal.Number bevat aantal pulsRawSignal.Multiplyen * 2  => negeren
      // RawSignal.Pulses[1] bevat startbit met tijdsduur van 1T => negeren
      // RawSignal.Pulses[2] bevat lange space na startbit met tijdsduur van 8T => negeren
      i=3; // RawSignal.Pulses[3] is de eerste van een T,xT,T,xT combinatie
      //if ( RawSignal.Number==(NewKAKU_RawSignalLength-2) || RawSignal.Number==(NewKAKUdim_RawSignalLength-2) ) i=1;
      do {
          P0=RawSignal.Pulses[i]  ; // * RawSignal.Multiply;
          P1=RawSignal.Pulses[i+1]; // * RawSignal.Multiply;
          P2=RawSignal.Pulses[i+2]; // * RawSignal.Multiply;
          P3=RawSignal.Pulses[i+3]; // * RawSignal.Multiply;
          
          if (P0<NewKAKU_mT && P1<NewKAKU_mT && P2<NewKAKU_mT && P3>NewKAKU_mT) { 
              Bit=0; // T,T,T,4T
              //Serial.print("0");
          } else 
          if (P0<NewKAKU_mT && P1>NewKAKU_mT && P2<NewKAKU_mT && P3<NewKAKU_mT) {
              Bit=1; // T,4T,T,T
              //Serial.print("1");
          } else 
          if (P0<NewKAKU_mT && P1<NewKAKU_mT && P2<NewKAKU_mT && P3<NewKAKU_mT) {      // T,T,T,T Deze hoort te zitten op i=111 want: 27e NewKAKU bit maal 4 plus 2 posities voor startbit
             if(RawSignal.Number!=NewKAKUdim_RawSignalLength) {    // als de dim-bits er niet zijn
               //Serial.println("No dim bits");
               return false;
             }
          } else {
            //Serial.println("Unknown pattern");
            return false;                                          // andere mogelijkheden zijn niet geldig in NewKAKU signaal.  
          }
          
          if(i<130) {                                              // alle bits die tot de 32-bit pulstrein behoren 32bits * 4posities per bit + pulse/space voor startbit
            bitstream=(bitstream<<1) | Bit;
          } else {                                                 // de resterende vier bits die tot het dimlevel behoren 
            dim=(dim<<1) | Bit;
          }       
          i+=4;                                                    // volgende pulsenquartet
      } while(i<RawSignal.Number-2);                             //-2 omdat de space/pulse van de stopbit geen deel meer van signaal uit maakt.
      //==================================================================================
      // Prevent repeating signals from showing up
      //==================================================================================
      if(SignalHash!=SignalHashPrevious || (RepeatingTimer+700<millis() ) || SignalCRC != bitstream ) { // 1000
         // not seen the RF packet recently
         SignalCRC=bitstream;
      } else {
         // already seen the RF packet recently
         //Serial.println("Skip");
         return true;
      } 
      //==================================================================================
      // Output
      // ----------------------------------
      sprintf(pbuffer, "20;%02X;", PKSequenceNumber++);           // Node and packet number 
      Serial.print( pbuffer );
      // ----------------------------------
      Serial.print(F("NewKaku;"));                               // Label
      sprintf(pbuffer, "ID=%08lx;",((bitstream) >> 6) );   // ID   
      Serial.print( pbuffer );
      sprintf(pbuffer, "SWITCH=%d;", ((bitstream)&0x0f)+1 );     
      Serial.print( pbuffer );
      Serial.print(F("CMD="));                    

      int command = (bitstream >> 4) & 0x03;
      if (command > 1) command ++;
      if (i>140) {                                               // Commando en Dim deel
          sprintf(pbuffer, "SET_LEVEL=%d;", dim );     
          Serial.print( pbuffer );
      } else {
          if ( command == 0 ) Serial.print(F("OFF;"));
          if ( command == 1 ) Serial.print(F("ON;"));
          if ( command == 3 ) Serial.print(F("ALLOFF;"));
          if ( command == 4 ) Serial.print(F("ALLON;"));
      }
      Serial.println();
      // ----------------------------------
      RawSignal.Repeats=true;                    // suppress repeats of the same RF packet         
      RawSignal.Number=0;
      return true;
}
#endif // Plugin_004

#ifdef PLUGIN_TX_004
boolean PluginTX_004(byte function, char *string) {
        boolean success=false;
        //10;NewKaku;123456;3;ON;                   // ON, OFF, ALLON, ALLOFF, ALL 99, 99      
        //10;NewKaku;306070b;f;ON;
        //01234567890123456789012
        if (strncasecmp(InputBuffer_Serial+3,"NEWKAKU;",8) == 0) { 
           byte x=18;                               // pointer to the switch number
           if (InputBuffer_Serial[17] != ';') {
              if (InputBuffer_Serial[18] != ';') {
                 return false;
              } else {
                 x=19;
                 if (InputBuffer_Serial[20] != ';') return false;
              }
           } else {
              if (InputBuffer_Serial[19] != ';') return false;
              
           }
           
           unsigned long bitstream=0L;
           unsigned long tempaddress=0L;
           byte cmd=0;
           byte c=0;
           byte Address=0;                          // Address 1..16
            
           InputBuffer_Serial[ 9]=0x30;
           InputBuffer_Serial[10]=0x78;             // Get address from hexadecimal value 
           if (x==19) {
              InputBuffer_Serial[18]=0x00;          // Get address from hexadecimal value 
           } else {
              InputBuffer_Serial[17]=0x00;          // Get address from hexadecimal value 
           }
           tempaddress=str2int(InputBuffer_Serial+9);  // NewKAKU address

           while((c=InputBuffer_Serial[x++])!=';'){ // Address: 1 to 16
              if(c>='0' && c<='9'){Address=Address*10;Address=Address+c-'0';}
           }
           if (Address > 16) return false;          // invalid address
           Address--;                               // 1 to 16 -> 0 to 15
           tempaddress=(tempaddress <<6) + Address;

           cmd=str2cmd(InputBuffer_Serial+x);       // Get ON/OFF etc. command
           if (cmd == false) {                      // Not a valid command received? ON/OFF/ALLON/ALLOFF
              cmd=str2int(InputBuffer_Serial+x);    // get DIM value
           }
           // --------------- NEWKAKU SEND ------------
           //unsigned long bitstream=0L;
           byte i=1;
           x=0;                                                         // aantal posities voor pulsen/spaces in RawSignal
        
           // bouw het KAKU adres op. Er zijn twee mogelijkheden: Een adres door de gebruiker opgegeven binnen het bereik van 0..255 of een lange hex-waarde
           if (tempaddress<=255)
              bitstream=1|(tempaddress<<6);                             // Door gebruiker gekozen adres uit de Nodo_code toevoegen aan adres deel van de KAKU code. 
           else
              bitstream=tempaddress & 0xFFFFFFCF;                       // adres geheel over nemen behalve de twee bits 5 en 6 die het schakel commando bevatten.
    
           RawSignal.Repeats=7;                                         // Aantal herhalingen van het signaal.
           RawSignal.Delay=20;                                          // Tussen iedere pulsenreeks enige tijd rust.

           if (cmd == VALUE_ON || cmd == VALUE_OFF) {
              bitstream|=(cmd == VALUE_ON)<<4;                          // bit-5 is het on/off commando in KAKU signaal
              x=130;                                                    // verzend startbit + 32-bits = 130
           } else
              x=146;                                                    // verzend startbit + 32-bits = 130 + 4dimbits = 146
     
           // bitstream bevat nu de KAKU-bits die verzonden moeten worden.
           for(i=3;i<=x;i++)RawSignal.Pulses[i]=NewKAKU_1T/RawSignal.Multiply;  // De meeste tijden in signaal zijn T. Vul alle pulstijden met deze waarde. Later worden de 4T waarden op hun plek gezet
      
           i=1;
           RawSignal.Pulses[i++]=NewKAKU_1T/RawSignal.Multiply;         // pulse van de startbit
           RawSignal.Pulses[i++]=NewKAKU_8T/RawSignal.Multiply;         // space na de startbit
      
           byte y=31;                                                   // bit uit de bitstream
           while(i<x) {
             if ((bitstream>>(y--))&1)
                RawSignal.Pulses[i+1]=NewKAKU_4T/RawSignal.Multiply;    // Bit=1; // T,4T,T,T
             else
                RawSignal.Pulses[i+3]=NewKAKU_4T/RawSignal.Multiply;    // Bit=0; // T,T,T,4T
    
             if (x==146) {                                              // als het een dim opdracht betreft
                if (i==111)                                             // Plaats van de Commando-bit uit KAKU 
                   RawSignal.Pulses[i+3]=NewKAKU_1T/RawSignal.Multiply; // moet een T,T,T,T zijn bij een dim commando.
                if (i==127) {                                           // als alle pulsen van de 32-bits weggeschreven zijn
                   bitstream=(unsigned long)cmd;                        //  nog vier extra dim-bits om te verzenden. 
                   y=3;
                }
             }
             i+=4;
           }
           RawSignal.Pulses[i++]=NewKAKU_1T/RawSignal.Multiply;         // pulse van de stopbit
           RawSignal.Pulses[i]=0;                                       // space van de stopbit
           RawSignal.Number=i;                                          // aantal bits*2 die zich in het opgebouwde RawSignal bevinden
           RawSendRF();
           success=true;
        }
        // --------------------------------------
        return success;
}
#endif // Plugin_TX_004
