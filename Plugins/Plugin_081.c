//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                               Plugin-14 Flamingo FA20RF Rookmelder                                ##
//#######################################################################################################
/*********************************************************************************************\
 * Dit protocol zorgt voor aansturen van Flamingo FA20RF rookmelder
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
 * Je gebruikt 1 unit als master, waarvan de je code aanleert aan de andere units (slaves)
 *
 * Let op: De rookmelder geeft alarm zolang dit bericht wordt verzonden en stopt daarna automatisch
 \*********************************************************************************************/
#define FA20RFSTART                 8000
#define FA20RFSPACE                  800
#define FA20RFLOW                   1300
#define FA20RFHIGH                  2600

#define PLUGIN_ID 81
#define PLUGIN_NAME "SmokeAlertSend"

boolean Plugin_081(byte function, struct NodoEventStruct *event, char *string)
  {
  boolean success=false;

  switch(function)
  {
#ifdef PLUGIN_081_CORE
  case PLUGIN_RAWSIGNAL_IN:
    {
      break;
    }

  case PLUGIN_COMMAND:
    {
    unsigned long bitstream=event->Par2;
    RawSignal.Multiply=50;
    if (event->Par1 == 0) event->Par1=50;
    RawSignal.Repeats=event->Par1;
    RawSignal.Delay=20;
    RawSignal.Pulses[1]=FA20RFSTART/RawSignal.Multiply;
    RawSignal.Pulses[2]=FA20RFSPACE/RawSignal.Multiply;
    RawSignal.Pulses[3]=FA20RFSPACE/RawSignal.Multiply;
    for(byte x=49;x>=3;x=x-2)
      {
      RawSignal.Pulses[x]=FA20RFSPACE/RawSignal.Multiply;
      if ((bitstream & 1) == 1) RawSignal.Pulses[x+1] = FA20RFHIGH/RawSignal.Multiply; 
      else RawSignal.Pulses[x+1] = FA20RFLOW/RawSignal.Multiply;
      bitstream = bitstream >> 1;
      }

    RawSignal.Pulses[51]=FA20RFSPACE/RawSignal.Multiply;
    RawSignal.Pulses[52]=0;
    RawSignal.Number=52;

    RawSendRF();
    success=true;
    break;
    } 
#endif // PLUGIN_081_CORE

#if NODO_MEGA
  case PLUGIN_MMI_IN:
    {
    char *TempStr=(char*)malloc(INPUT_COMMAND_SIZE);

    if(GetArgv(string,TempStr,1))
      {
      if(strcasecmp(TempStr,PLUGIN_NAME)==0)
        {
        event->Type = NODO_TYPE_PLUGIN_COMMAND;
        event->Command = PLUGIN_ID; // Plugin nummer  
        success=true;
        }
      }
    free(TempStr);
    break;
    }

  case PLUGIN_MMI_OUT:
    {
    strcpy(string,PLUGIN_NAME);            // Eerste argument=het commando deel
    strcat(string," ");
    strcat(string,int2str(event->Par1));
    strcat(string,",");
    strcat(string,int2str(event->Par2));
    break;
    }
#endif //NODO_MEGA
  }      
  return success;
}

