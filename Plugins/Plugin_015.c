//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                      Plugin-15: HomeEasy EU                                       ##
//#######################################################################################################
/*********************************************************************************************\
 * Dit protocol zorgt voor ontvangst HomeEasy EU zenders
 * die werken volgens de automatische codering (Ontvangers met leer-knop)
 *
 * LET OP: GEEN SUPPORT VOOR DIRECTE DIMWAARDES!!!
 *
 * Auteur             : StuntTeam
 * Support            : www.nodo-domotica.nl
 * Datum              : 25 Mrt 2013
 * Versie             : 1.3
 * Syntax             : "HomeEasy <Adres>,<On|Off>
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
   11000111100 10111100011101110010001111100011 1100 10 11 000111 1  HE301EU ON
   11000111100 10111100011101110010001111100011 1100 01 11 000111 1  HE301EU OFF
   11000111100 10111101001101011011001011011100 1011 10 01 000111 1  HE300EU 1:ON, Switch:I 
   11000111100 10111101001101011011001011011100 1011 01 01 000111 1  HE300EU 1:OFF, Switch:I 
   11000111100 10111101001101011011001011011100 1011 10 01 010011 1  HE300EU1:ON, Switch:II	
   11000111100 10111011010100011110110101100011 1011 10 01 000111 1 #channel 0 on               10001110
   11000111100 10111011010100011110110101100011 1011 01 01 000111 1 #channel 0 off   0000
   11000111100 10111011010100011110110101100011 1011 10 01 001011 1 #channel 1 on
   11000111100 10111011010100011110110101100011 1011 01 01 001011 1 #channel 1 off   0001
   11000111100 10111011010100011110110101100011 1011 10 01 001101 1 #channel 2 on    
   11000111100 10111011010100011110110101100011 1011 01 01 001101 1 #channel 2 off   0010
   11000111100 10111011010100011110110101100011 1011 01 01 001110 1 #channel 3 off   0011
   11000111100 10111011010100011110110101100011 1011 01 01 010011 1 #channel 4 off   0100
   11000111100 10111011010100011110110101100011 1011 01 01 010101 1 #channel 5 off   0101
   11000111100 10111011010100011110110101100011 1011 01 01 001100 1 #channel 15 off  1111     11011000 
   11000111100 10111011010100011110110101100011 1011 01 01 001010 1 #channel 14 off  1110
   11000111100 10111101001101011011001011011100 1100 10 11 000111 1 HE300EUGroup:ON,Switch:IV
   11000111100 10111101001101011011001011011100 1100 01 11 000111 1 HE300EUGroup:OFF,Switch:IV
   11000111100 10111101001101011011001011011100 1011 01 01 101100 1 HE300EU4:OFF,Switch:IV
   11000111100 10111011010100011110110101100011 1011 01 01 000101 1 #channel 11 off
   
 * A = Startbits/Preamble, 
 * B = Address, 32 bits
 * C = Unknown, Possibly: Device type 
 * D = Command, 1 bit only?
 * E = Group indicator
 * F = Channel  0-15
 * G = Stopbit

 * SAMPLE:
 * Pulses=116;Pulses(uSec)=200,1175,125,1175,125,200,150,200,125,200,150,1175,150,1175,150,1175,150,1175,125,200,150,200,150,200,125,1175,150,1175,150,1175,125,1175,150,200,125,200,150,1175,125,1175,150,200,125,1175,125,1175,150,200,150,200,150,1175,150,200,150,1175,150,200,150,1175,150,200,150,200,125,1175,150,200,125,1175,150,1175,125,1175,150,200,125,200,125,200,150,200,125,1175,150,1175,150,1175,150,200,150,200,125,200,150,1175,150,1175,150,1175,150,1175,125,200,150,200,125,1175,125,200,125,1175,150,1150,125;
 * HE preamble: 11000111100 (63C) Address: 1111001101100101010010111000011 (79B2A5C3) Stopbits: 0 (0) Commands: 10001111001011 Command: 0 Channel: 1011 Group: 1
 * 20;04;HomeEasy;ID=7900b200;SWITCH=0b;CMD=ALLOFF;
 
   HE preamble: 11000111100 (63C) Address: 1111001101100101010010111000011 (79B2A5C3) Stopbits: 0 (0) Commands: 10010111001011 Command: 0 Channel: 1011 Group: 1

Preamble 200,1175,125,1175,125,200,150,200,125,200,150,1175,150,1175,150,1175,150,1175,125,200,150,200,
Address  150,200,125,1175,150,1175,150,1175,125,1175,150,200,125,200,150,1175,125,1175,150,200,125,1175,125,1175,150,200,150,200,150,1175,150,200,150,1175,150,200,150,1175,150,200,150,200,125,1175,150,200,125,1175,150,1175,125,1175,150,200,125,200,125,200,150,200,125,1175,150,1175,
Command  150,1175,150,200,150,200,125,200,150,1175,150,1175,150,1175,150,1175,125,200,150,200,125,1175,125,200,125,1175,150,1150,  - 125;
 \*********************************************************************************************/
 
#define HomeEasy_LongLow        0x490    // us
#define HomeEasy_ShortHigh      200      // us
#define HomeEasy_ShortLow       150      // us
#define HomeEasy_PulseLength    116

#define PLUGIN_ID 15
#define PLUGIN_NAME "HomeEasy"

boolean Plugin_015(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
  #ifdef PLUGIN_015_CORE
  case PLUGIN_RAWSIGNAL_IN:
    {
      unsigned long preamble = 0;
      unsigned long address = 0;
      unsigned long bitstream = 0;
      unsigned long stopbits = 0;
      int counter = 0;
      byte rfbit =0;
      byte command = 0;
      byte group = 0;
      unsigned long channel = 0;
      char buffer[14]=""; 
      
      //==================================================================================
      // valid messages are 116 pulses          
      if (RawSignal.Number != HomeEasy_PulseLength) return false;
      // convert pulses into bit sections (preamble, address, bitstream)
      for(byte x=1;x<=RawSignal.Number;x=x+2) {
         if ((RawSignal.Pulses[x]*RawSignal.Multiply < 500) & (RawSignal.Pulses[x+1]*RawSignal.Multiply > 500)) 
            rfbit = 1;
         else
            rfbit = 0;
            
         if (x<=22) preamble = (preamble << 1) | rfbit;
         if ((x>=23) && (x<=86)) address = (address << 1) | rfbit;
         if ((x>=87) && (x<=114)) bitstream = (bitstream << 1) | rfbit;
      }
      //==================================================================================
      // To prevent false positives make sure the preamble is correct, 
      // it should always be 0x63c but we compare only 10 bits to compensate for the first bit being seen incorrectly 
      if ( (preamble & 0x3ff) != 0x23c) {          // comparing 10 bits is enough to make sure the packet is valid
         //Serial.println("HE preamble error:");   
         //Serial.println(preamble,HEX);    
         return false;        
      }
      //==================================================================================
      command = ((bitstream >> 9) & 0x1);      // 1=off 0=on ?
      channel = (bitstream) & 0x3f;
      group = ((bitstream >> 7) & 0x1);        // 1=group 
      // -------------------------
      /*
      Serial.print("HE Address: ");    
      Serial.print(address,BIN);    
      Serial.print(" (");    
      Serial.print(address,HEX);    
      Serial.print(") Commands: ");    
      Serial.print(bitstream, BIN);    
      Serial.print(" Command: ");    
      Serial.print(command);    
      Serial.print(" Channel: ");    
      Serial.print(channel,BIN);    
      Serial.print(" Group: ");    
      Serial.println(group );    
      */
      // Add channel info to base address, first shift channel info 6 positions, so it can't interfere with bit 5
      //channel = channel << 6;
      //address = address + channel;
      //Serial.print(" ");    
      //Serial.print(channel);    
      //Serial.print(" ");    
      //Serial.println(address);    

      // ----------------------------------
      // Output
      // ----------------------------------
      sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
      Serial.print( buffer );
      // ----------------------------------
      Serial.print("HomeEasy;");                  // Label
      sprintf(buffer, "ID=%02x%02x%02x%02x;", (address>>24)&0xff  ,(address>>16)&0xff,(address>>8)&0xff, address&0xff); // ID    
      Serial.print( buffer );
      sprintf(buffer, "SWITCH=%02x;", channel);     
      Serial.print( buffer );
      Serial.print("CMD=");                    
      if ( group == 1) {
         Serial.print("ALL");
      }
      if ( command == 0) {
         Serial.print("OFF;");
      } else {
         Serial.print("ON;");
      }
      Serial.println();     
      // ----------------------------------
      //event->Par1=command?VALUE_ON:VALUE_OFF; // On/Off bit omzetten naar een Nodo waarde. 
      //event->Par2=address &0xFFFFFFCF; 
      //event->SourceUnit    = 0;                     // Komt niet van een Nodo unit af, dus unit op nul zetten
      //event->Type          = NODO_TYPE_PLUGIN_EVENT;
      //event->Command       = 15; // Nummer van dit device
      RawSignal.Repeats    = true; // het is een herhalend signaal. Bij ontvangst herhalingen onderdrukken.
      success=true;
      break;
    }

#endif // PLUGIN_015_CORE

#if NODO_MEGA
  case PLUGIN_MMI_IN:
    {
    char* str=(char*)malloc(INPUT_COMMAND_SIZE);

    if(GetArgv(string,str,1))
      {
      if(strcasecmp(str,PLUGIN_NAME)==0)
        {
        if(GetArgv(string,str,2))
          {
          event->Par2=str2int(str);    
          if(GetArgv(string,str,3))
            {
            // Vul Par1 met het HomeEasy commando. Dit kan zijn: VALUE_ON, VALUE_OFF, Andere waarden zijn ongeldig.

            // haal uit de tweede parameter een 'On' of een 'Off'.
            if(event->Par1=str2cmd(str))
              {
              event->Type  = NODO_TYPE_PLUGIN_EVENT;
              event->Command = 15; // Plugin nummer                
              success=true;
              }
            }
          }
        }
      }
    free(str);
    break;
    }

  case PLUGIN_MMI_OUT:
    {
    strcpy(string,PLUGIN_NAME);            // Eerste argument=het commando deel
    strcat(string," ");

    // In Par3 twee mogelijkheden: Het bevat een door gebruiker ingegeven adres 0..255 of een volledig HomeEasy adres.
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
#endif //NODO_MEGA
  }      
  return success;
}
