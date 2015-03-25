//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                       Plugin-04: NewKAKU                                          ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of receiving from and transmitting to "Klik-Aan-Klik-Uit" devices
 * working according to the learning code system. This protocol is also used by the Home Easy devices.
 * It includes direct DIM functionality.
 * 
 * Auteur             : StuntTeam
 * Support            : www.nodo-domotica.nl
 * Versie             : 18-01-2015, Versie 1.0
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 ***********************************************************************************************
 * Incoming event: "NewKaku     <adres>,<On|Off|AllOn|AllOff|Dimlevel 1..15>
 * Send          : "NewKakuSend <adres>,<On|Off|AllOn|AllOff|Dimlevel 1..15> 
 ***********************************************************************************************
 * Pulse (T) is 275us PDM
 * 0 = T,T,T,4T, 1 = T,4T,T,T, dim = T,T,T,T op bit 27
 *
 * NewKAKU supports:
 *   on/off       ---- 000x Off/On
 *   all on/off   ---- 001x AllOff/AllOn
 *   dim absolute xxxx 0110 Dim16        // dim on bit 27 + 4 extra bits for dim level
 *
 *  NewKAKU bitstream= (First sent) AAAAAAAAAAAAAAAAAAAAAAAAAACCUUUU(LLLL) -> A=KAKU_address, C=command, U=KAKU-Unit, L=extra dimlevel bits (optional)
 *
 * Internal use of the paramters in the Nodo event: 
 * 
 * Cmd  : contains the command SendNewKAKU or the event NewKAKU. Unused.
 * Par1 : Command VALUE_ON, VALUE_OFF or dim level [1..15]
 * Par2 : Address
 *
 * Sample RF packet:
 * 20;B8;NewKaku;ID=00c142;SWITCH=1;CMD=OFF;
 * 20;B9;DEBUG;Pulses=132;Pulses(uSec)=200,2550,150,200,125,1200,150,200,150,1200,125,1200,150,225,125,1200,125,225,125,200,150,1200,150,200,150,1200,150,1200,125,200,125,200,125,1225,125,1200,125,225,150,200,150,1200,150,1200,150,200,150,1200,150,225,125,200,150,1200,125,200,150,1200,125,200,150,1200,150,200,125,1225,150,200,125,1200,150,1200,125,225,125,200,125,1200,150,1200,125,225,125,200,125,1225,125,200,125,1225,125,200,125,1200,125,200,150,1225,125,1200,150,200,125,200,125,1200,125,200,150,1200,125,200,125,1200,150,200,125,1200,125,200,125,1200,150,200,125,1200,150,200,150,1200,125;
 * 20;06;DEBUG;Pulses=132;Pulses(uSec)=175,2575,150,200,150,1200,150,200,150,1200,150,1200,150,200,125,1200,150,200,125,200,150,1200,125,200,150,1200,150,1200,150,200,150,200,150,1225,150,1200,125,225,150,200,125,1200,150,1200,150,200,150,1200,150,200,150,200,125,1225,125,200,150,1200,125,200,150,1200,125,200,150,1200,150,200,150,1200,150,1200,125,200,150,200,125,1200,150,1200,125,225,150,200,125,1200,150,200,150,1200,150,200,150,1200,150,200,150,1225,125,1200,150,200,125,200,150,1200,150,200,125,1200,150,200,150,1200,150,200,150,1200,150,200,125,1225,125,200,125,1200,150,200,150,1200,150
 \*********************************************************************************************/
#define PLUGIN_ID 4
#define PLUGIN_004_EVENT       "NewKAKU"
#define PLUGIN_004_COMMAND "NewKAKUSend"
#define NewKAKU_RawSignalLength      132        // regular KAKU packet length
#define NewKAKUdim_RawSignalLength   148        // KAKU packet length including DIM bits
#define NewKAKU_1T                   275        // us
#define NewKAKU_mT                   650        // us, ergens ongeveer midden tussen 1T en 4T 
#define NewKAKU_4T                  1100        // us
#define NewKAKU_8T                  2200        // us, Tijd van de space na de startbit

boolean Plugin_004(byte function, struct NodoEventStruct *event, char *string)
  {
  boolean success=false;
  
  switch(function)
    {
    #ifdef PLUGIN_004_CORE
    case PLUGIN_RAWSIGNAL_IN:
      {
      unsigned long bitstream=0L;
      boolean Bit;
      int i;
      int P0,P1,P2,P3;
      event->Par1=0;
      char buffer[14]=""; 
      
      // nieuwe KAKU bestaat altijd uit start bit + 32 bits + evt 4 dim bits. Ongelijk, dan geen NewKAKU
      //if (RawSignal.Number==NewKAKU_RawSignalLength || RawSignal.Number==NewKAKUdim_RawSignalLength || RawSignal.Number==(NewKAKU_RawSignalLength-2) || RawSignal.Number==(NewKAKUdim_RawSignalLength-2) )
      if (RawSignal.Number==NewKAKU_RawSignalLength || RawSignal.Number==NewKAKUdim_RawSignalLength )
        {
        // RawSignal.Number bevat aantal pulsRawSignal.Multiplyen * 2  => negeren
        // RawSignal.Pulses[1] bevat startbit met tijdsduur van 1T => negeren
        // RawSignal.Pulses[2] bevat lange space na startbit met tijdsduur van 8T => negeren
        i=3; // RawSignal.Pulses[3] is de eerste van een T,xT,T,xT combinatie
        //if ( RawSignal.Number==(NewKAKU_RawSignalLength-2) || RawSignal.Number==(NewKAKUdim_RawSignalLength-2) ) i=1;

        do {
          P0=RawSignal.Pulses[i]    * RawSignal.Multiply;
          P1=RawSignal.Pulses[i+1]  * RawSignal.Multiply;
          P2=RawSignal.Pulses[i+2]  * RawSignal.Multiply;
          P3=RawSignal.Pulses[i+3]  * RawSignal.Multiply;
          
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
            event->Par1=(event->Par1<<1) | Bit;
            //Serial.print(" ");
            //Serial.print(i);
            //Serial.print("=");
            //Serial.print(Bit,BIN);
          }       
          i+=4;                                                    // volgende pulsenquartet
        } while(i<RawSignal.Number-2);                             //-2 omdat de space/pulse van de stopbit geen deel meer van signaal uit maakt.
        byte hibyte=(bitstream)>> 22;
        // ----------------------------------
        // Output
        // ----------------------------------
        sprintf(buffer, "20;%02X;", PKSequenceNumber++);           // Node and packet number 
        Serial.print( buffer );
        // ----------------------------------
        Serial.print("NewKaku;");                                  // Label
        sprintf(buffer, "ID=%02x%04x;",hibyte, (((bitstream) >> 6) & 0xffff) );   // ID    
        Serial.print( buffer );
        sprintf(buffer, "SWITCH=%d;", ((bitstream)&0x0f)+1 );     
        Serial.print( buffer );
        Serial.print("CMD=");                    

        int command = (bitstream >> 4) & 0x03;
        if (command > 1) command ++;
        if (i>140) {                                                 // Commando en Dim deel
            event->Par1++;                                           // Dim level. +1 omdat gebruiker dim level begint bij één.
            sprintf(buffer, "SET_LEVEL=%d;", event->Par1 );     
            Serial.print( buffer );
        } else {
            if ( command == 0 ) Serial.print("OFF;");
            if ( command == 1 ) Serial.print("ON;");
            if ( command == 3 ) Serial.print("ALLOFF;");
            if ( command == 4 ) Serial.print("ALLON;");
        }
        Serial.println();
        // ----------------------------------
        //if(bitstream>0xffff)                                       // Adres-deel: Is het signaal van een originele KAKU zender afkomstig, of van een Nodo ingegeven door de gebruiker ?
        //  event->Par2=bitstream &0x0FFFFFCF;                       // Op hoogste nibble zat vroeger het signaaltype. & is t.b.v. compatibiliteit 
        //else                                                       // Het is van een andere Nodo afkomstig. 
        //  event->Par2=(bitstream>>6)&0xff;                         // Neem dan alleen 8bit v/h adresdeel van KAKU signaal over
        //if(i>140)                                                  // Commando en Dim deel
        //  event->Par1++;                                           // Dim level. +1 omdat gebruiker dim level begint bij één.
        //else
        //  event->Par1=((bitstream>>4)&0x01)?VALUE_ON:VALUE_OFF;    // On/Off bit omzetten naar een Nodo waarde. 
        //event->SourceUnit    = 0;                                  // Komt niet van een Nodo unit af, dus unit op nul zetten
        //event->Type          = NODO_TYPE_PLUGIN_EVENT;
        //event->Command       = PLUGIN_ID;                          // nummer van dit device
        RawSignal.Repeats    = true;                               // het is een herhalend signaal. Bij ontvangst herhalingen onderdrukken.
        success=true;
        }   
      break;
      }
      
    case PLUGIN_COMMAND:
      {
      unsigned long bitstream=0L;
      byte i=1;
      byte x;                                                      // aantal posities voor pulsen/spaces in RawSignal
        
      // bouw het KAKU adres op. Er zijn twee mogelijkheden: Een adres door de gebruiker opgegeven binnen het bereik van 0..255 of een lange hex-waarde
      if(event->Par2<=255)
        bitstream=1|(event->Par2<<6);                              // Door gebruiker gekozen adres uit de Nodo_code toevoegen aan adres deel van de KAKU code. 
      else
        bitstream=event->Par2 & 0xFFFFFFCF;                        // adres geheel over nemen behalve de twee bits 5 en 6 die het schakel commando bevatten.
    
      event->Port=VALUE_ALL;                                       // Signaal mag naar alle door de gebruiker met [Output] ingestelde poorten worden verzonden.
      RawSignal.Repeats=7;                                         // Aantal herhalingen van het signaal.
      RawSignal.Delay=20;                                          // Tussen iedere pulsenreeks enige tijd rust.
      RawSignal.Multiply=25;

      if(event->Par1==VALUE_ON || event->Par1==VALUE_OFF)
        {
        bitstream|=(event->Par1==VALUE_ON)<<4;                     // bit-5 is het on/off commando in KAKU signaal
        x=130;                                                     // verzend startbit + 32-bits = 130
        }
      else
        x=146;                                                     // verzend startbit + 32-bits = 130 + 4dimbits = 146
     
      // bitstream bevat nu de KAKU-bits die verzonden moeten worden.
    
      for(i=3;i<=x;i++)RawSignal.Pulses[i]=NewKAKU_1T/RawSignal.Multiply;  // De meeste tijden in signaal zijn T. Vul alle pulstijden met deze waarde. Later worden de 4T waarden op hun plek gezet
      
      i=1;
      RawSignal.Pulses[i++]=NewKAKU_1T/RawSignal.Multiply;         // pulse van de startbit
      RawSignal.Pulses[i++]=NewKAKU_8T/RawSignal.Multiply;         // space na de startbit
      
      byte y=31;                                                   // bit uit de bitstream
      while(i<x)
        {
        if((bitstream>>(y--))&1)
          RawSignal.Pulses[i+1]=NewKAKU_4T/RawSignal.Multiply;     // Bit=1; // T,4T,T,T
        else
          RawSignal.Pulses[i+3]=NewKAKU_4T/RawSignal.Multiply;     // Bit=0; // T,T,T,4T
    
        if(x==146)                                                 // als het een dim opdracht betreft
          {
          if(i==111)                                               // Plaats van de Commando-bit uit KAKU 
            RawSignal.Pulses[i+3]=NewKAKU_1T/RawSignal.Multiply;   // moet een T,T,T,T zijn bij een dim commando.
          if(i==127)                                               // als alle pulsen van de 32-bits weggeschreven zijn
            {
            bitstream=(unsigned long)event->Par1-1;                //  nog vier extra dim-bits om te verzenden. -1 omdat dim niveau voor gebruiker begint bij 1
            y=3;
            }
          }
        i+=4;
        }
      RawSignal.Pulses[i++]=NewKAKU_1T/RawSignal.Multiply;         // pulse van de stopbit
      RawSignal.Pulses[i]=0;                                       // space van de stopbit
      RawSignal.Number=i;                                          // aantal bits*2 die zich in het opgebouwde RawSignal bevinden
      SendEvent(event,true,true,Settings.WaitFree==VALUE_ON);
      success=true;
      break;
      }
    #endif // CORE
      
    #if NODO_MEGA
    case PLUGIN_MMI_IN:
      {
      char* str=(char*)malloc(INPUT_COMMAND_SIZE);
    
      if(GetArgv(string,str,1))
        {
        event->Type=0;

        if(strcasecmp(str,PLUGIN_004_EVENT)==0)
          event->Type=NODO_TYPE_PLUGIN_EVENT;

        if(strcasecmp(str,PLUGIN_004_COMMAND)==0)
          event->Type=NODO_TYPE_PLUGIN_COMMAND;
        
        if(event->Type)
          {
          if(GetArgv(string,str,2))
            {
            event->Par2=str2int(str); 

            //Serial.print("Par2:");
            //Serial.println( event->Par2,HEX );
                        
            if(GetArgv(string,str,3)) {
              // Vul Par1 met het KAKU commando. Dit kan zijn: VALUE_ON, VALUE_OFF, 1..16. Andere waarden zijn ongeldig.
              
              // haal uit de tweede parameter een 'On' of een 'Off'.
              if(event->Par1=str2cmd(str))
                success=true;
                
              // als dit niet is gelukt, dan uit de tweede parameter de dimwaarde halen.
              else {
                event->Par1=str2int(str);                          // zet string om in integer waarde
                if(event->Par1>=1 && event->Par1<=16)              // geldig dim bereik 1..16 ?
                   success=true;
                }
              event->Command = PLUGIN_ID;                          // Plugin nummer  
              }
            }
          }
        }
      free(str);
      break;
      }

    case PLUGIN_MMI_OUT:
      {
      if(event->Type==NODO_TYPE_PLUGIN_EVENT)
        strcpy(string,PLUGIN_004_EVENT);                           // Eerste argument=het commando deel

      if(event->Type==NODO_TYPE_PLUGIN_COMMAND)
        strcpy(string,PLUGIN_004_COMMAND);                         // Eerste argument=het commando deel

      strcat(string," ");
    
      // In Par3 twee mogelijkheden: Het bevat een door gebruiker ingegeven adres 0..255 of een volledig NewKAKU adres.
      if(event->Par2>=0x0ff)
        strcat(string,int2strhex(event->Par2)); 
      else
        strcat(string,int2str(event->Par2)); 
      
      strcat(string,",");
      
      if(event->Par1==VALUE_ON)
        strcat(string,"On");  
      else if(event->Par1==VALUE_OFF)
        strcat(string,"Off");
      else
        strcat(string,int2str(event->Par1));

      break;
      }
    #endif //MMI
    }      
  return success;
  }
