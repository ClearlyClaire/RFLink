//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                        Plugin-02: Kaku                                            ##
//#######################################################################################################
/*********************************************************************************************\
 * Dit protocol zorgt voor ontvangst van Klik-Aan-Klik-Uit zenders (Handzender, PIR, etc.)
 * een aansturing van ontvangers die werken volgens de handmatige codering (draaiwiel met adres- en huiscodering). 
 * Dit protocol kan eveneens worden ontvangen door Klik-Aan-Klik-Uit apparaten die werken met automatische
 * code programmering. Dimmen wordt niet ondersteund. Coding/Encoding principe is in de markt bekend
 * onder de namen: Princeton PT2262 / MOSDESIGN M3EB / Domia Lite / Klik-Aan-Klik-Uit / Intertechno
 * 
 * Author             : StuntTeam
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 ***********************************************************************************************
 * Incoming event: "Kaku <adres>,<On | Off>
 * Send          : "KakuSend <Adres>, <On | Off> 
 *
 * Address = A0..P16 according to KAKU adressing/notation
 ***********************************************************************************************
 * Het signaal bestaat drie soorten reeksen van vier pulsen, te weten: 
 * 0 = T,3T,T,3T, 1 = T,3T,3T,T, short 0 = T,3T,T,T Hierbij is iedere pulse (T) 350us PWDM
 *
 * KAKU ondersteund:
 *        on/off, waarbij de pulsreeks er als volgt uit ziet: 000x en x staat voor Off / On
 *    all on/off, waarbij de pulsreeks er als volgt uit ziet: 001x en x staat voor All Off / All On 
 *
 * Interne gebruik van de parameters in het Nodo event:
 * 
 * Cmd  : Hier zit het commando SendKAKU of het event KAKU in. Deze gebruiken we verder niet.
 * Par1 : Groep commando (true of false)
 * Par2 : Hier zit het KAKU commando (On/Off) in. True of false
 * Par3 : Adres en Home code. Acht bits AAAAHHHH
 *
 * POSSIBLE BIT VARIATIONS:
 * ------------
 *              1111 1111 111 1
 *              0110 0000 000 0
 *              AAAA BBBB CCC D
 * KAKU:        A bit 0-3  = address 2^4 = 16 addresses
 *              B bit 4-7  = device  2^4 = 16 devices
 *              C bit 8-10 = fixed
 *              D bit 11   = on/off command
 * ------------
 *              0 111 0000 1 000
 *              A BBB CCCC D EEE
 * Blokker:     B bit 1-3  = device  
 *              D bit 8    = on/off command
 * ------------
 *              11111 11111 11
 *              AAAAA BBBBB CC
 * Action:      A bit 0-4  = address 2^5 = 32 addresses
 *              B bit 5-9  = device  
 *              C bit 10-11= on/off command
 * ------------
 * Elro Home    10001 10000 10
 *    Easy      AAAAA BBBBB CC
 * Elro Home    A bit 0-4  = address 2^5 = 32 addresses
 *    Control   B bit 5-9  = device  
 * Brennenstuhl C bit 10-11= on/off command (only 10 or 01)
 *    Comfort
 * ------------
 *              10100 00010 0 0 
 *              AAAAA BBBBB C D
 * InterTechno  A bit 0-4  = address 2^5 = 32 addresses
 *Düwi Terminal B bit 5-9  = device  
 * Cogex        C bit 10 = always 0 
 *              D bit 11 = on/off command 
 *
 *20;DB;DEBUG;Pulses=48;Pulses(uSec)=100,875,275,875,300,875,275,875,275,875,275,875,275,875,275,875,275,875,275,875,275,875,275,875,275,875,275,875,275,875,275,875,275,875,275,875,925,200,275,875,925,200,275,875,925,225,250;
 \*********************************************************************************************/
#define KAKU_CodeLength             12                                          // aantal data bits
#define KAKU_T                     300                                          // 350 us
#define PLUGIN_002_EVENT        "Kaku"
#define PLUGIN_002_COMMAND  "KakuSend"

boolean Plugin_002(byte function, struct NodoEventStruct *event, char *string)
  {
  boolean success=false;
      unsigned long bitstream=0;
  
  switch(function)
    {
    #ifdef PLUGIN_002_CORE

    case PLUGIN_RAWSIGNAL_IN:
      {
      int i,j;
      boolean error=false; 
      byte housecode=0;
      byte unitcode=0;
      char buffer[14]=""; 
      // ==========================================================================
      if (RawSignal.Number!=(KAKU_CodeLength*4)+2)return false; // conventionele KAKU bestaat altijd uit 12 data bits plus stop. Ongelijk, dan geen KAKU!
      
      for (i=0; i<KAKU_CodeLength; i++) {
        j=(KAKU_T*2)/RawSignal.Multiply;   
        
        if (RawSignal.Pulses[4*i+1]<j && RawSignal.Pulses[4*i+2]>j && RawSignal.Pulses[4*i+3]<j && RawSignal.Pulses[4*i+4]>j) { // 0101
            bitstream=(bitstream >> 1);  // 0
        } else 
        if (RawSignal.Pulses[4*i+1]<j && RawSignal.Pulses[4*i+2]>j && RawSignal.Pulses[4*i+3]>j && RawSignal.Pulses[4*i+4]<j) { // 0110
           bitstream=(bitstream >> 1 | (1 << (KAKU_CodeLength-1)));  // 1
        } else 
        if (RawSignal.Pulses[4*i+1]<j && RawSignal.Pulses[4*i+2]>j && RawSignal.Pulses[4*i+3]<j && RawSignal.Pulses[4*i+4]<j) { // 0100
           bitstream=(bitstream >> 1);   // Short 0, Group command on 2nd bit.
           event->Par1=2;
        } else {
          if (i==0) {                     // are we dealing with a RTK/AB600 device? then the first bit is sometimes mistakenly seen as 1101  
             if (RawSignal.Pulses[4*i+1]>j && RawSignal.Pulses[4*i+2]>j && RawSignal.Pulses[4*i+3]<j && RawSignal.Pulses[4*i+4]>j) { // 1101
                bitstream=(bitstream >> 1 | (1 << (KAKU_CodeLength-1)));  // 1
                //bitstream=(bitstream >> 1);  // 0
             } else { 
                error=true;
             } 
          } else {
             error=true;
          }
        }                                 // bad signal
      }
      // ==========================================================================
      if (error==true) {
         //Serial.print("Kaku bitstream error : ");
         //Serial.println(bitstream, BIN);
         return false;
      }     
      // ----------------------------------
      housecode=(((bitstream) &0x0f) +0x41);
      unitcode=((((bitstream) &0xf0) >> 4)+1);
      // ----------------------------------
      if ((bitstream&0x600)!=0x600) {
         //Serial.println("Kaku 0/1 error");
         return false;                              // twee vaste bits van KAKU gebruiken als checksum
      }
      // ==========================================================================
      // ----------------------------------
      // Alles is in orde, bouw event op           
      // ----------------------------------
      // Output
      // ----------------------------------
      sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
      Serial.print( buffer );
      // ----------------------------------
      Serial.print("Kaku;");                           // Label
      sprintf(buffer, "ID=%02x;", housecode);          // ID    
      Serial.print( buffer );
      sprintf(buffer, "SWITCH=%d;", unitcode);     
      Serial.print( buffer );
      Serial.print("CMD=");                    
      if ( ((bitstream >> 11) & 0x01) == 1) {
         Serial.print("ON;");
      } else {
         Serial.print("OFF;");
      }
      Serial.println();
      // ----------------------------------
      RawSignal.Repeats    = true;                     // het is een herhalend signaal. Bij ontvangst herhalingen onderdrukken.
      //event->Par2          = bitstream & 0xFF;
      //event->Par1         |= (bitstream >> 11) & 0x01;
      //event->SourceUnit    = 0;                        // Komt niet van een Nodo unit af.
      //event->Type          = NODO_TYPE_PLUGIN_EVENT;
      //event->Command       = 2;                        // nummer van deze plugin
      RawSignal.Repeats    = true;                     // Hiermee geven we aan dat het om een herhalend signaal gaat en vervolg pulsen NIET tot een event moeten leiden.
      success=true;
      break;
      }
      
    case PLUGIN_COMMAND:
      {

      RawSignal.Multiply=50;
      RawSignal.Repeats=7;                             // KAKU heeft minimaal vijf herhalingen nodig om te schakelen.
      RawSignal.Delay=20;                              // Tussen iedere pulsenreeks enige tijd rust.
      RawSignal.Number=KAKU_CodeLength*4+2;            // Lengte plus een stopbit
      event->Port=VALUE_ALL;                           // Signaal mag naar alle door de gebruiker met [Output] ingestelde poorten worden verzonden.
     
      unsigned long Bitstream = event->Par2 | (0x600 | ((event->Par1&1 /*Commando*/) << 11)); // Stel een bitstream samen
      
      // loop de 12-bits langs en vertaal naar pulse/space signalen.  
      for (byte i=0; i<KAKU_CodeLength; i++) {
        RawSignal.Pulses[4*i+1]=KAKU_T/RawSignal.Multiply;
        RawSignal.Pulses[4*i+2]=(KAKU_T*3)/RawSignal.Multiply;
    
        if (((event->Par1>>1)&1) /* Group */ && i>=4 && i<8) {
          RawSignal.Pulses[4*i+3]=KAKU_T/RawSignal.Multiply;
          RawSignal.Pulses[4*i+4]=KAKU_T/RawSignal.Multiply;
        } else { // short 0
          if((Bitstream>>i)&1) { // 1
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
        SendEvent(event,true,true,Settings.WaitFree==VALUE_ON);
        success=true;
        break;
      }
    #endif //PLUGIN_CORE_002
      
    #if NODO_MEGA
    case PLUGIN_MMI_IN:
      {
      // Reserveer een kleine string en kap voor de zekerheid de inputstring af om te voorkomen dat er
      // buiten gereserveerde geheugen gewerkt wordt.
      char *TempStr=(char*)malloc(INPUT_COMMAND_SIZE);

      // Hier aangekomen bevat string het volledige commando. Test als eerste of het opgegeven commando overeen komt met "Kaku"
      // Dit is het eerste argument in het commando.
      if(GetArgv(string,TempStr,1))
        {
        event->Type  = 0;
        
        if(strcasecmp(TempStr,PLUGIN_002_COMMAND)==0) {
          event->Type  = NODO_TYPE_PLUGIN_COMMAND;
        }

        if(event->Type) {
          byte c;
          byte x=0;                                                             // teller die wijst naar het te behandelen teken
          byte Home=0;                                                          // KAKU home A..P
          byte Address=0;                                                       // KAKU Address 1..16
          event->Command = 2;                                                   // Plugin nummer  
        
          if(GetArgv(string,TempStr,2))                                         // eerste parameter bevat adres volgens codering A0..P16 
            {
            while((c=tolower(TempStr[x++]))!=0)
              {
              if(c>='0' && c<='9'){Address=Address*10;Address=Address+c-'0';}
              if(c>='a' && c<='p'){Home=c-'a';}                                 // KAKU home A is intern 0
              }
          
            if(Address==0)
              {                                                                 // groep commando is opgegeven: 0=alle adressen
              event->Par1=2;                                                    // 2e bit setten voor groep.
              event->Par2=Home;
              }
            else
              event->Par2= Home | ((Address-1)<<4);        
   
            if(GetArgv(string,TempStr,3))                                       // Het door de gebruiker ingegeven tweede parameter bevat het on/off commando
              {
              event->Par1 |= str2cmd(TempStr)==VALUE_ON; 
              success=true;
              }
            }
          }
        }
      free(TempStr);
      break;
      }
    #endif //MMI
    }      
  return success;
  }
