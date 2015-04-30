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
 * Technische informatie:
 * De Flamingo FA20RF rookmelder bevat een RF zender en ontvanger. Standaard heeft elke unit een uniek ID
 * De rookmelder heeft een learn knop waardoor hij het ID van een andere unit kan overnemen
 * Daardoor kunnen ze onderling worden gekoppeld.
 * Na het koppelen hebben ze dus allemaal hetzelfde ID!
 * Je gebruikt 1 unit als master, waarvan je de code aanleert aan de andere units (slaves)
 *
 * Sample:
 * 20;32;DEBUG;Pulses=52;Pulses(uSec)=2500,800,650,1250,650,1250,650,1250,650,1250,650,1250,650,1275,650,1250,650,2550,650,1275,650,2550,650,1250,650,1250,650,2550,650,2550,650,1275,650,2550,
 650,2550,650,1275,650,2550,650,2550,650,1275,650,1275,650,2550,650,1200,650;
 000000010100110110110010 = 014DB2
 * 20;0C;DEBUG;Pulses=52;Pulses(uSec)=2500,825,625,2575,625,1275,625,1300,625,2575,625,1275,625,2575,625,2575,625,2575,625,2575,625,2575,625,2575,625,1275,625,1275,625,1275,625,2575,625,2575,
 625,2575,625,1275,625,2575,625,2575,625,1300,625,1275,625,2575,625,1225,625;
 100101111110001110110010 = 97E3B2
 * 20;0D;FA20RF;ID=e3b2;SMOKEALERT=ON;
 \*********************************************************************************************/
#define PLUGIN_ID 80
#define PLUGIN_NAME "SmokeAlert"

boolean Plugin_080(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef PLUGIN_080_CORE
  case PLUGIN_RAWSIGNAL_IN:
    {
      if (RawSignal.Number != 52) return false;

      unsigned long bitstream=0L;
      char buffer[11]=""; 
      
      for(byte x=4;x<=50;x=x+2) {
        if (RawSignal.Pulses[x-1]*RawSignal.Multiply > 1000) return false; // every preceding puls must be < 1000!
        if (RawSignal.Pulses[x]*RawSignal.Multiply > 1800) bitstream = (bitstream << 1) | 0x1; 
        else bitstream = bitstream << 1;
      }
      if (bitstream == 0) return false;
      if (bitstream == 0xFFFFFF) return false;
      if ((bitstream)&0xffff == 0xffff) return false;
      //==================================================================================
      // Output
      // ----------------------------------
      sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
      Serial.print( buffer );
      // ----------------------------------
      Serial.print("FA20RF;");                   // Label
      sprintf(buffer, "ID=%04x;", (bitstream&0xffff) );         // ID    
      Serial.print( buffer );
      Serial.print("SMOKEALERT=ON;");
      Serial.println();
      //==================================================================================
      event->Par1=0;
      event->Par2=bitstream;
      event->SourceUnit    = 0;                     // Komt niet van een Nodo unit af, dus unit op nul zetten
      event->Port          = VALUE_SOURCE_RF;
      event->Type          = NODO_TYPE_PLUGIN_EVENT;
      event->Command       = PLUGIN_ID; // Nummer van dit device

      return true;
      break;
    }
  case PLUGIN_COMMAND:
    {
      break;
    }
#endif // PLUGIN_080_CORE

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
          event->Par1=str2int(str);    
          if(GetArgv(string,str,3))
            {
              event->Par2=str2int(str);
              event->Type = NODO_TYPE_PLUGIN_EVENT;
              event->Command = PLUGIN_ID; // Plugin nummer  
              success=true;
            }
          }
        }
      }
    free(str);
    break;
    }
#endif //NODO_MEGA
  }      
  return success;
}
