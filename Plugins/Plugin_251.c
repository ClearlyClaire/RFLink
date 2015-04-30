//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ## 
//##                                        Plugin-251: UserEvent                                      ##
//#######################################################################################################
/*********************************************************************************************\
* Funktionele beschrijving: Dit device makt dat de Nodo compatibel is voor ontvangst 
*                           van UserEvents volgens het oude signaalformat van de Nodo Due.
*
 * Author             : StuntTeam
 * Support            : http://sourceforge.net/projects/rflink/
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
***********************************************************************************************
* Technische beschrijving:
* Oude Nodo versies maakten gebruik van een 32-bit signaal. UserEvents werden eveneens in dit format
* verzonden. Vanaf de 3.6 versie is gekozen voor een technisch geavanceerdere opbouw van RF en IR codes 
* Consequentie hiervan is dat het oude formaat niet meer werd herkend door de Nodo. Dit device zorgt er
* voor dat de oude UserEvents nog kunnen worden ingelezen en verwerkt.
* 
* LET OP: In het oude format wordt geen gebruik gemaakt van het Home adres van
*         een Nodo. Events komen ongeacht de home definitie altijd binnen.
\*********************************************************************************************/
 
#define PLUGIN_NAME "UserEvent"
#define PLUGIN_ID   251

boolean Plugin_251(byte function, struct NodoEventStruct *event, char *string)
  {
  boolean success=false;
  unsigned long bitstream=0L;
  int x,z;

  switch(function)
    {    
    #ifdef PLUGIN_251_CORE
    case PLUGIN_RAWSIGNAL_IN:
      {
      // NODO signaal bestaat uit start bit + 32 bits. Als ongelijk aan 66, dan geen Nodo signaal
      if(RawSignal.Number==66)
        {
        // 0=aantal, 1=startpuls, 2=space na startpuls, 3=1e pulslengte. Dus start loop met drie.
        z=0;
        for(x=3;x<=RawSignal.Number;x+=2)
          {
          if((RawSignal.Pulses[x]*RawSignal.Multiply)>1000) //??? NODO_PULSE_MID      
            bitstream|=(long)(1L<<z); //LSB in signaal wordt  als eerste verzonden
          z++;
          }
  
        // We hoeven alleen maar compatible te zijn met de userevents van de oude Nodo.
        // in code 100 heeft in de vorige versies altijd het userevent gezeten.
        if(((bitstream>>16)&0xff)==100)
          {
          RawSignal.Repeats    = true; // het is een herhalend signaal. Bij ontvangst herhalingen onderdrukken.
          ClearEvent(event);
          event->SourceUnit=(bitstream>>24)&0xf;
          event->DestinationUnit=0;
          event->Type=NODO_TYPE_EVENT;
          event->Command=EVENT_USEREVENT;
          event->Par1=(bitstream>>8)&0xff;
          event->Par2=bitstream&0xff;
          success=true;
          }
        }
      }
    #endif // CORE
    
    #if NODO_MEGA // alleen relevant voor een Nodo Mega want de Small heeft geen MMI!
    case PLUGIN_MMI_IN:
      {
      char *TempStr=(char*)malloc(INPUT_COMMAND_SIZE);

      if(GetArgv(string,TempStr,1))
        {
        if(strcasecmp(TempStr,PLUGIN_NAME)==0)
          {
          event->Type    = NODO_TYPE_EVENT;
          event->Command = PLUGIN_ID;
          success=true;
          }
        }
      free(TempStr);
      break;
      }
    #endif //MMI
    }    
  return success;
  }

