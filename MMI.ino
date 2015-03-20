
#if NODO_MEGA

 /*******************************************************************************************************\
 * Deze funktie parsed een string zoals die door de gebruiker wordt opgegeven. De commando's
 * worden gechecked op geldigheid en ter uitvoering doorgegeven.
 * Commando's en events worden gescheiden met een puntkomma.
 \*******************************************************************************************************/
int ExecuteLine(char *Line, byte Port)
  {
  ProcessingStatus(true);

  char *Command=(char*)malloc(INPUT_COMMAND_SIZE);
  char *TmpStr1=(char*)malloc(INPUT_COMMAND_SIZE);
  char *TmpStr2=(char*)malloc(INPUT_LINE_SIZE);
  int CommandPos;
  int LinePos;
  int w,x,y;
  int EventlistWriteLine=0;
  byte error=0, State_EventlistWrite=0;
  unsigned long a;
  struct NodoEventStruct EventToExecute,TempEvent;


  Transmission_SendToUnit=Transmission_SendToAll;                               // Als de SendTo wel/niet permanent, dan unitnummer overnemen of uitschakelen.

  if(Transmission_SendToAll)                                                    // Als SendTo wel permanent ingeschakeld, dan queue leegmaken anders eventuele oude queue inhoud meeverzonden met SendTo
    QueuePosition=0;

  if(FileWriteMode>0)                                                           // verwerking van commando's is door gebruiker tijdelijk geblokkeerd door FileWrite commando
    {
    if(StringFind(Line,cmd2str(CMD_FILE_WRITE))!=-1)                            // string gevonden!
      {
      FileWriteMode=0;                                                          // Stop de FileWrite modus
      TempLogFile[0]=0;
      PrintString(ProgmemString(Text_22),Port);
      }
    if(TempLogFile[0]!=0)
      FileWriteLine("",TempLogFile,"DAT",Line, false);
    }
  else
    {
    CommandPos=0;
    LinePos=0;    
    int LineLength=strlen(Line);

    while(LinePos<=LineLength && error==0)
      {
      char LineChar=Line[LinePos];
  
      if(LineChar=='!')                                                         // Comment teken. hierna verder niets meer doen. 
        {
        LinePos=LineLength+1;                                                   // ga direct naar einde van de regel.
        }

      // Commando compleet als puntkomma (scheidt opdrachten) of einde string.
      if((LineChar=='!' || LineChar==';' || LineChar==0) && CommandPos>0 || CommandPos==(INPUT_COMMAND_SIZE-1))
        {
        Command[CommandPos]=0;
        CommandPos=0;

        if(Substitute(Command)!=0)                                              // De Nodo kan berekeningen maken en variabelen vullen. Voer deze taak uit.
          error=MESSAGE_INVALID_PARAMETER;

        Alias(Command,true);                                                    // check of ingevoerde commando een alias is. Is dit het geval, dan wordt Command vervangen door de alias.

        error=Str2Event(Command, &EventToExecute);                              // Commando's in tekst format moeten worden omgezet naar een Nodo event.
        EventToExecute.Port=Port;
        
        // Enkele comando's kennen een afwijkende behandeling. Dit zijn commando's die niet uitgevoerd
        // kunnen worden door ExecuteCommand() omdat we in de Nodo geen strings kunnen doorgeven in de
        // eventstruct, Deze commando's separaat parsen en direct hier uitvoeren.
        if(error==MESSAGE_UNKNOWN_COMMAND)
          {
          error=0;                                                              // nieuwe poging.

          ClearEvent(&EventToExecute);
          GetArgv(Command,TmpStr1,1);
          EventToExecute.Command=str2cmd(TmpStr1);                              // Bouw een nieuw event op. 

          if(GetArgv(Command,TmpStr1,2))                                        // Haal Par1 uit het commando.
            {
            EventToExecute.Par1=str2cmd(TmpStr1);
            if(!EventToExecute.Par1)
              EventToExecute.Par1=str2int(TmpStr1);
            }
          
          if(GetArgv(Command,TmpStr1,3))                                        // Haal Par2 uit het commando.
            {
            EventToExecute.Par2=str2cmd(TmpStr1);
            if(!EventToExecute.Par2)
              EventToExecute.Par2=str2int(TmpStr1);
            }        
          x=EventToExecute.Command;
          EventToExecute.Command=0;                                             // Toegevoegd voor de werking van Sendto.          

          switch(x)
            {
            case CMD_FILE_EXECUTE:
              if(EventToExecute.Par2==VALUE_ON)
                EventToExecute.Par1=VALUE_ON;
              else
                EventToExecute.Par1=VALUE_OFF;
                
              if(GetArgv(Command,TmpStr1,2))
                EventToExecute.Par2=str2int(TmpStr1);

              if(EventToExecute.Par2==0)
                  error=FileExecute("",TmpStr1,"DAT", EventToExecute.Par1==VALUE_ON, VALUE_ALL);
              else
                {
                EventToExecute.Type=NODO_TYPE_COMMAND;
                EventToExecute.Command=CMD_FILE_EXECUTE;
                }
              break;                                                                             
        
            case CMD_SENDTO:
              QueuePosition=0;                                                  //We gebruiken de Queue voor verwerking van de commando's die middels SendTo naar de Slave moeten  
              Transmission_SendToUnit=EventToExecute.Par1;
              Transmission_SendToAll=0;  
              Transmission_SendToFast=false;
                            
              if(StringFind(Command,cmd2str(VALUE_OFF))!=-1)
                {
                Transmission_SendToUnit=0;
                }
              else
                {
                if(EventToExecute.Par1>UNIT_MAX)
                  error=MESSAGE_INVALID_PARAMETER;
                else
                  {
                  if(StringFind(Command, cmd2str(VALUE_ALL))!=-1)               // Zoek of in een van de parameters All staat
                    Transmission_SendToAll=Transmission_SendToUnit;  
                    
                  if(StringFind(Command, cmd2str(VALUE_FAST))!=-1)              // Zoek of in een van de parameters Fast staat
                    Transmission_SendToFast=true;
                  }
                }
              EventToExecute.Command=0;      
              break;    
        
            case CMD_EVENTLIST_WRITE:
              EventToExecute.Type=NODO_TYPE_COMMAND;
              EventToExecute.Command=CMD_EVENTLIST_WRITE;      
              if(Transmission_SendToUnit==Settings.Unit || Transmission_SendToUnit==0)// Als geen SendTo actief
                {                          
                if(EventToExecute.Par1<=EventlistMax)
                  {
                  EventlistWriteLine=EventToExecute.Par1;
                  State_EventlistWrite=1;
                  }
                else
                  error=MESSAGE_INVALID_PARAMETER;
                }
              else
                {
                // Het EventlistWrite commando moet naar de Slave worden verzonden. In dit geval maken we er een systeem commando
                // van omdat deze bijzondere behandeling aan de slave zijde nodig heeft.
                EventToExecute.Command=SYSTEM_COMMAND_QUEUE_EVENTLIST_WRITE;
                EventToExecute.Type=NODO_TYPE_SYSTEM;
                QueueAdd(&EventToExecute);
                }
              
              EventToExecute.Command=0;      
              break;
        
            case CMD_PASSWORD:
              {
              EventToExecute.Type=NODO_TYPE_COMMAND;
              TmpStr1[0]=0;
              GetArgv(Command,TmpStr1,2);
              TmpStr1[25]=0;                                                    // voor geval de string te lang is.
              strcpy(Settings.Password,TmpStr1);
        
              if(Settings.Lock)                                                 // Als een lock actief, dan lock op basis van nieuwe password instellen
                {
                Settings.Lock=0L;
                for(x=0;x<strlen(Settings.Password);x++)
                  {
                  Settings.Lock=Settings.Lock<<5;
                  Settings.Lock^=Settings.Password[x];
                  }
                }
              break;
              }  
        
            case CMD_ID:
              {
              EventToExecute.Type=NODO_TYPE_COMMAND;
              TmpStr1[0]=0;
              GetArgv(Command,TmpStr1,2);
              TmpStr1[9]=0;                                                     // voor geval de string te lang is.
              strcpy(Settings.ID,TmpStr1);
              break;
              }  
        
            case CMD_TEMP:
              {
              EventToExecute.Type=NODO_TYPE_COMMAND;
              x=StringFind(Command,cmd2str(CMD_TEMP))+strlen(cmd2str(CMD_TEMP));
              while(Command[x]==' ')x++;                                        // eventuele spaties verwijderen
              strcpy(TmpStr1,Command+x);                                        // Alles na de "temp" hoort bij de variabele
              TmpStr1[25]=0;                                                    // voor geval de string te lang is.
              strcpy(Settings.Temp,TmpStr1);
              break;
              }  
            
            case CMD_FILE_ERASE:      
              if(GetArgv(Command,TmpStr1,2))
                FileErase("",TmpStr1,"DAT");
              break;
        
            case CMD_RAWSIGNAL_DELAY:
              RawSignal.Delay=EventToExecute.Par1;
              break;
        
            case CMD_RAWSIGNAL_SAMPLE:
              if(EventToExecute.Par1==0)
                Settings.RawSignalSample=RAWSIGNAL_SAMPLE_DEFAULT;
              else
                Settings.RawSignalSample=EventToExecute.Par1;
              RawSignal.Multiply=Settings.RawSignalSample;
              break;
        
            case CMD_RAWSIGNAL_REPEATS:
              RawSignal.Repeats=EventToExecute.Par1;
              EventToExecute.Command=0;                                         // Geen verdere verwerking meer nodig.
              break;
            
            case CMD_RAWSIGNAL_PULSES:
              x=StringFind(Line,cmd2str(CMD_RAWSIGNAL_PULSES))+strlen(cmd2str(CMD_RAWSIGNAL_PULSES));
              while(Line[x]==' ')x++;                                           // eventuele spaties verwijderen
              error=RawSignalPulses(Line+x);
              LinePos=LineLength+1;                                             // ga direct naar einde van de regel.
              break;
            
            case CMD_RAWSIGNAL_ERASE:      
              if(GetArgv(Command,TmpStr1,2))
                {
                if(TmpStr1[0]!='*')
                  x=2;
                else
                  x=0;
                FileErase(ProgmemString(Text_08),TmpStr1+x,"DAT");              // +2 om zo de "0x" van de string te strippen.
                }
              break;
                  
            #ifdef ethernetserver_h
            case CMD_FILE_GET_HTTP:
              EventToExecute.Type=NODO_TYPE_COMMAND;
              if(GetArgv(Command,TmpStr1,2))
                {
                Led(BLUE);
                GetHTTPFile(TmpStr1);
                }
              else
                error=MESSAGE_INVALID_PARAMETER;            
              break;
            #endif 
              
            case CMD_FILE_SHOW:
              EventToExecute.Type=NODO_TYPE_COMMAND;
              if(GetArgv(Command,TmpStr1,2))
                {
                PrintString(ProgmemString(Text_22), Port);
                error=FileShow("",TmpStr1,"DAT",Port);
                PrintString(ProgmemString(Text_22), Port);
                }
              break;
                        
            case CMD_NODO_IP:
              EventToExecute.Type=NODO_TYPE_COMMAND;
              if(GetArgv(Command,TmpStr1,2))
                if(!str2ip(TmpStr1,Settings.Nodo_IP))
                  error=MESSAGE_INVALID_PARAMETER;
              break;
              
            case CMD_CLIENT_IP:
              EventToExecute.Type=NODO_TYPE_COMMAND;
              if(GetArgv(Command,TmpStr1,2))
                if(!str2ip(TmpStr1,Settings.Client_IP))
                  error=MESSAGE_INVALID_PARAMETER;
              break;
              
            case CMD_SUBNET:
              EventToExecute.Type=NODO_TYPE_COMMAND;
              if(GetArgv(Command,TmpStr1,2))
                if(!str2ip(TmpStr1,Settings.Subnet))
                  error=MESSAGE_INVALID_PARAMETER;
              break;
              
            case CMD_DNS_SERVER:
              EventToExecute.Type=NODO_TYPE_COMMAND;
              if(GetArgv(Command,TmpStr1,2))
                if(!str2ip(TmpStr1,Settings.DnsServer))
                  error=MESSAGE_INVALID_PARAMETER;
              break;
              
            case CMD_GATEWAY:
              EventToExecute.Type=NODO_TYPE_COMMAND;
              if(GetArgv(Command,TmpStr1,2))
                if(!str2ip(TmpStr1,Settings.Gateway))
                  error=MESSAGE_INVALID_PARAMETER;
              break;
              
            case CMD_EVENTLIST_FILE:
              EventToExecute.Type=NODO_TYPE_COMMAND;
              if(GetArgv(Command,TmpStr1,2))
                {
                if(!SaveEventlistSDCard("",TmpStr1,"DAT"))
                  {
                  error=MESSAGE_SDCARD_ERROR;
                  break;
                  }
                }
              break;
        
            case CMD_IF:
              EventToExecute.Type=NODO_TYPE_COMMAND;
              x=StringFind(Command," ") ;                                       // laat x wijzen direct NA het if commando.
              strcpy(TmpStr1,Command+x);                                        // Alles na de "if" hoort bij de voorwaarde
        
              // eventuele spaties er uithalen
              y=0;
              for(x=0;x<strlen(TmpStr1);x++)
                {
                if(TmpStr1[x]!=' ')
                  TmpStr1[y++]=TmpStr1[x];
                }
              TmpStr1[y]=0;
              
              // Zoek '=' teken op en splitst naar linker en rechter operand.
              x=StringFind(TmpStr1,"<>");
              if(x!=-1)
                {
                strcpy(TmpStr2,TmpStr1+x+2);
                TmpStr1[x]=0;
                if(strcasecmp(TmpStr1,TmpStr2)==0)
                  LinePos=LineLength+1; // ga direct naar einde van de regel.
                }
                
              x=StringFind(TmpStr1,"=");
              if(x!=-1)
                {
                strcpy(TmpStr2,TmpStr1+x+1);
                TmpStr1[x]=0;
                if(strcasecmp(TmpStr1,TmpStr2)!=0)
                  LinePos=LineLength+1; // ga direct naar einde van de regel.
                }
              else
                error=MESSAGE_INVALID_PARAMETER;
              break;
        
            case CMD_RAWSIGNAL_LIST:
              FileList(ProgmemString(Text_08),Port);
              break;
        
            case CMD_FILE_LIST:
              FileList("",Port);
              break;
        
            case CMD_FILE_WRITE:
              if(GetArgv(Command,TmpStr1,2) && strlen(TmpStr1)<=8)
                {
                PrintString(ProgmemString(Text_22),Port);
                TmpStr1[8]=0;//voor de zekerheid een te lange string afkappen
                strcpy(TempLogFile,TmpStr1);
                FileWriteMode=120;
                }
              else
                error=MESSAGE_INVALID_PARAMETER;
              break;
            
            case CMD_FILE_WRITE_LINE:
              if(GetArgv(Command,TmpStr1,2) && strlen(TmpStr1)<=8)
                {
                if(Substitute(Line)!=0)
                  error=MESSAGE_INVALID_PARAMETER;

                x=StringFind(Line,TmpStr1)+strlen(TmpStr1);
                while(Line[x]==32 || Line[x]==' ;')x++;


                FileWriteLine("",TmpStr1, "DAT", Line+x, false);
                LinePos=LineLength+1; // ga direct naar einde van de regel.
                }
              else
                error=MESSAGE_INVALID_PARAMETER;
              break;
        
            case CMD_ALIAS_WRITE:
              x=StringFind(Line,cmd2str(CMD_ALIAS_WRITE))+strlen(cmd2str(CMD_ALIAS_WRITE));
              while(Command[x]==' ')x++;             // eventuele spaties verwijderen
              error=AliasWrite(Line+x);
              break;
        
            case CMD_ALIAS_LIST:
              x=StringFind(Line,cmd2str(CMD_ALIAS_LIST))+strlen(cmd2str(CMD_ALIAS_LIST));
              while(Command[x]==' ')x++;             // eventuele spaties verwijderen
              PrintString(ProgmemString(Text_22), Port);
              AliasList(Line+x, Port);
              PrintString(ProgmemString(Text_22), Port);
              break;
        
            case CMD_ALIAS_ERASE:
              x=StringFind(Line,cmd2str(CMD_ALIAS_ERASE))+strlen(cmd2str(CMD_ALIAS_ERASE));
              while(Command[x]==' ')x++;             // eventuele spaties verwijderen
              error=AliasErase(Line+x);
              break;
        
            case CMD_HTTP_REQUEST:
              // zoek in de regel waar de string met het http request begint.
              x=StringFind(Line,cmd2str(CMD_HTTP_REQUEST))+strlen(cmd2str(CMD_HTTP_REQUEST));
              while(Line[x]==32)x++;
              strcpy(Settings.HTTPRequest,&Line[0]+x);
              break;

            default:
              {                 
              // Ingevoerde commando is niet gevonden. 
              // Loop de devices langs om te checken if er een hit is. Zo ja, dan de struct
              // met de juiste waarden gevuld. Is er geen hit, dan keert PluginCall() terug met een false.
              // in dat geval kijken of er een commando op SDCard staat
              if(!PluginCall(PLUGIN_MMI_IN,&EventToExecute,Command))
                {
                // Als het geen regulier commando was EN geen commando met afwijkende MMI en geen Plugin en geen alias, dan kijken of file op SDCard staat)
                // Voer bestand uit en verwerking afbreken als er een foutmelding is.
                error=MESSAGE_UNKNOWN_COMMAND;
                if(strlen(Command)<=8)
                  error=FileExecute("",Command,"DAT", false, VALUE_ALL);
                  
                // als script niet te openen, dan is het ingevoerde commando ongeldig.
  
                EventToExecute.Command=0;      
                }

              if(error)
                {
                strcpy(TmpStr2,Command);
                strcat(TmpStr2,"?");
                PrintString(TmpStr2,VALUE_ALL);
                // Answer that an invalid command was received?
                char buffer[11]=""; 
                sprintf(buffer, "20;%02X;CMD UNKNOWN;", PKSequenceNumber++); // Node and packet number 
                Serial.println( buffer );
                //Serial.println("20;CMD UNKNOWN;");
                }
              }                          
            }// switch(command...@2
          }            

        if(EventToExecute.Command && error==0)
          {            
          // Er kunnen zich twee situaties voordoen:
          //
          // A: Event is voor deze Nodo en moet gewoon worden uitgevoerd;
          // B: SendTo is actief. Event moet worden verzonden naar een andere Nodo. Hier wordt de Queue voor gebruikt.
          
          if(State_EventlistWrite==0)// Gewoon uitvoeren
            {
            if(Transmission_SendToUnit==Settings.Unit || Transmission_SendToUnit==0)
              {
              EventToExecute.Port=Port;
              EventToExecute.Direction=VALUE_DIRECTION_INPUT;
              error=ProcessEvent(&EventToExecute);  
              }
            else
              {
              if(EventToExecute.Command)                                        // geen lege events in de queue plaatsen
                {
                QueueAdd(&EventToExecute);                                      // Plaats in queue voor latere verzending.
                }
              }
            continue;
            }
    
          if(State_EventlistWrite==2)
            {            
            UndoNewNodo();// Status NewNodo verwijderen indien van toepassing
            if(!Eventlist_Write(EventlistWriteLine,&TempEvent,&EventToExecute))
              {
              RaiseMessage(MESSAGE_EVENTLIST_FAILED,EventlistWriteLine);
              break;
              }
            State_EventlistWrite=0;
            continue;
            }  
            
          if(State_EventlistWrite==1)
            {
            TempEvent=EventToExecute; // TempEvent = >Event< dat moet worden weggeschreven in de eventlist;
            State_EventlistWrite=2;
            }
          }
        }// if(LineChar.

      // Tekens toevoegen aan commando zolang er nog ruimte is in de string
      if(LineChar!=';' && CommandPos<(INPUT_COMMAND_SIZE-1) )
        Command[CommandPos++]=LineChar;      
      LinePos++;
      }// while(LinePos...
  
    // Verzend de inhoud van de queue naar de slave Nodo
    if(Transmission_SendToUnit!=Settings.Unit && Transmission_SendToUnit!=0 && error==0 && QueuePosition>0)
      {
      error=QueueSend(Transmission_SendToFast);
      if(error)
        {
        CommandPos=0;
        LinePos=0;
        }
      }
    }// einde regel behandeling

  free(TmpStr2);
  free(TmpStr1);
  free(Command);

  // Verwerk eventuele events die in de queue zijn geplaatst.
  if(error==0)
    QueueProcess();
    
  return error;
  }


/*********************************************************************************************\
 * Print een event naar de opgegeven poort. Dit kan zijn:
 * 
 * VALUE_ALL, VALUE_SOURCE_SERIAL, VALUE_SOURCE_TELNET, VALUE_SOURCE_FILE
 * 
 \*********************************************************************************************/
void PrintEvent(struct NodoEventStruct *Event, byte Port)
  {

  // Systeem events niet weergeven.
  if(Event->Type==NODO_TYPE_SYSTEM || Event->Type==0)
    return;
  
  char* StringToPrint=(char*)malloc(128);
  char* TmpStr=(char*)malloc(INPUT_LINE_SIZE);

  StringToPrint[0]=0; // als start een lege string

  // Direction
  if(Event->Direction)
    {
    strcat(StringToPrint,cmd2str(Event->Direction));      
    strcat(StringToPrint,"=");
  
    // Poort
    strcat(StringToPrint, cmd2str(Event->Port));
    
    #ifdef ethernetserver_h
    if(Event->Port==VALUE_SOURCE_HTTP || Event->Port==VALUE_SOURCE_TELNET)
      {
      strcat(StringToPrint, "(");
      strcat(StringToPrint, ip2str(ClientIPAddress));
      strcat(StringToPrint, ")");
      }
    #endif
      
    if(Event->Port==VALUE_SOURCE_EVENTLIST)
      {
      // print de nesting diepte van de eventlist en de regel uit de eventlist.
      strcat(StringToPrint, "(");
      strcat(StringToPrint, int2str(ExecutionDepth-1));
      strcat(StringToPrint, ".");
      strcat(StringToPrint, int2str(ExecutionLine));
      strcat(StringToPrint, ")");
      }
    strcat(StringToPrint, "; "); 
    }

  // Unit 
  strcat(StringToPrint, cmd2str(VALUE_UNIT));
  strcat(StringToPrint, "=");  
  if(Event->Direction==VALUE_DIRECTION_OUTPUT && Event->Port!=VALUE_SOURCE_HTTP)
    strcat(StringToPrint, int2str(Event->DestinationUnit));
  else
    strcat(StringToPrint, int2str(Event->SourceUnit)); 

  // Event
  strcat(StringToPrint, "; ");
  strcat(StringToPrint, ProgmemString(Text_14));
  Event2str(Event,TmpStr);
  if(Settings.Alias==VALUE_ON)
    Alias(TmpStr,false);

  strcat(StringToPrint, TmpStr);

  // WEERGEVEN OP TERMINAL
  PrintString(StringToPrint,Port);   // stuur de regel naar Serial en/of naar Ethernet

  // LOG OP SDCARD
  if(bitRead(HW_Config,HW_SDCARD) && Settings.Log==VALUE_ON) 
    {
    TmpStr[0]='!';
    TmpStr[1]=' ';
    TmpStr[2]=0;

    // datum en tijd weergeven
    #if CLOCK
    if(bitRead(HW_Config,HW_CLOCK)) // alleen als er een RTC aanwezig is.
      {   
      strcat(TmpStr,DateTimeString());
      strcat(TmpStr,"; ");
      }
    #endif clock

    strcat(TmpStr,StringToPrint);
    FileWriteLine("",ProgmemString(Text_23),"DAT", TmpStr, false);
    }

  free(TmpStr);
  free(StringToPrint);
  } 


/**********************************************************************************************\
 * Print actuele dag, datum, tijd.
 \*********************************************************************************************/
#if CLOCK
char* DateTimeString(void)
  {
  int x;
  static char dt[40];
  char s[5];

  // Print de dag. 1=zondag, 0=geen RTC aanwezig
  for(x=0;x<=2;x++)
    s[x]=(*(ProgmemString(Text_04)+(Time.Day-1)*3+x));
  s[x]=0;

  sprintf(dt,ProgmemString(Text_17),Time.Date,Time.Month,Time.Year, s, Time.Hour, Time.Minutes);

  return dt;
  }
#endif clock

/**********************************************************************************************\
 * Print de welkomsttekst van de Nodo.
 \*********************************************************************************************/
prog_char PROGMEM Text_welcome[] = "Nodo-Mega V3.7, Product=SWACNC-MEGA-R%03d, Home=%d, ThisUnit=%d";
void PrintWelcome(void)
  {
  char *TempString=(char*)malloc(128);

  // Print Welkomsttekst
  PrintString("",VALUE_ALL);
  PrintString(ProgmemString(Text_22),VALUE_ALL);
  PrintString(ProgmemString(Text_01),VALUE_ALL);
  PrintString(ProgmemString(Text_02),VALUE_ALL);

  // print versienummer, unit en indien gevuld het ID
  sprintf(TempString,ProgmemString(Text_welcome), NODO_BUILD, HOME_NODO, Settings.Unit);
  if(Settings.ID[0])
    {
    strcat(TempString,", ID=");
    strcat(TempString,Settings.ID);
    }

  PrintString(TempString,VALUE_ALL);

  #if CLOCK
 // Geef datum en tijd weer.
  if(bitRead(HW_Config,HW_CLOCK))
    {
    sprintf(TempString,"%s %s",DateTimeString(), cmd2str(Time.DaylightSaving?VALUE_DLS:0));
    PrintString(TempString,VALUE_ALL);
    }
  #endif
  
  // print IP adres van de Nodo
  #ifdef ethernetserver_h
  if(bitRead(HW_Config,HW_ETHERNET))
    {
    sprintf(TempString,"IP=%u.%u.%u.%u, ", EthernetNodo.localIP()[0],EthernetNodo.localIP()[1],EthernetNodo.localIP()[2],EthernetNodo.localIP()[3]);

    strcat(TempString,cmd2str(CMD_PORT_OUTPUT));
    strcat(TempString,"=");        
    strcat(TempString,int2str(Settings.PortOutput));

    strcat(TempString,", ");        

    strcat(TempString,cmd2str(CMD_PORT_INPUT));
    strcat(TempString,"=");        
    strcat(TempString,int2str(Settings.PortInput));

    PrintString(TempString,VALUE_ALL);
    
    if(Settings.HTTPRequest[0])
      {
      strcpy(TempString,"Host=");        
      strcat(TempString,Settings.HTTPRequest);
      int x=StringFind(TempString,"/");
      TempString[x]=0;    
      strcat(TempString,", HostIP=");        
  
      if((IPClientIP[0] + IPClientIP[1] + IPClientIP[2] + IPClientIP[3]) > 0)
        strcat(TempString,ip2str(IPClientIP));
      else
        strcat(TempString,"?");        
  
      PrintString(TempString,VALUE_ALL);
      }
    }
  #endif
    
  PrintString(ProgmemString(Text_22),VALUE_ALL);
  free(TempString);
  }


 /**********************************************************************************************\
 * Verzend teken(s) naar de Terminal
 \*********************************************************************************************/
void PrintString(char* LineToPrint, byte Port)
  {  
  //if((Port==VALUE_SOURCE_SERIAL || Port==VALUE_ALL) && bitRead(HW_Config,HW_SERIAL))
  //  Serial.println(LineToPrint);
  // unwanted for serial communication

  #ifdef ethernetserver_h
  if(bitRead(HW_Config,HW_ETHERNET))
    {
    if((Port==VALUE_SOURCE_TELNET || Port==VALUE_ALL) && TerminalClient.connected() && TerminalConnected>0 && TerminalLocked==0)
      TerminalClient.println(LineToPrint);
      
    if((Port==VALUE_SOURCE_HTTP || Port==VALUE_ALL) && IPClient.connected())
      {
      IPClient.print(LineToPrint);
      IPClient.println("<br>");
      }
    }
  #endif
  }
  

#define PAR1_INT           1
#define PAR1_TEXT          2
#define PAR1_MESSAGE       3
#define PAR2_INT           4
#define PAR2_TEXT          5
#define PAR2_FLOAT         6
#define PAR2_INT_HEX       7
#define PAR2_DIM           8
#define PAR2_WDAY          9
#define PAR2_TIME         10
#define PAR2_DATE         11
#define PAR2_ALARMENABLED 12
#define PAR2_INT8         13
#define PAR3_INT          14
#define PAR4_INT          15
#define PAR5_INT          16
#define PAR3_TEXT         17

void Event2str(struct NodoEventStruct *Event, char* EventString)
  {
  int x;
  EventString[0]=0;
  char* str=(char*)malloc(INPUT_COMMAND_SIZE);
  str[0]=0;// als er geen gevonden wordt, dan is de string leeg

  // Er kunnen een aantal parameters worden weergegeven. In een kleine tabel wordt aangegeven op welke wijze de parameters aan de gebruiker
  // moeten worden getoond. Het is niet per defiitie zo dat de interne Par1, Par2 en Par3 ook dezelfe parameters zijn die aan de gebruiker
  // worden getoond.   
  byte ParameterToView[8]={0,0,0,0,0,0,0,0};

  // Plugins hebben een eigen MMI, Roep het device aan voor verwerking van de parameter PLUGIN_MMI_OUT.
  // zoek het device op in de devices tabel en laat de string vullen. Als het niet gelukt is om de string te
  // vullen dan behandelen als een regulier event/commando
  if(Event->Type == NODO_TYPE_PLUGIN_COMMAND || Event->Type == NODO_TYPE_PLUGIN_EVENT)
    {
    strcpy(EventString,"?");
    PluginCall(PLUGIN_MMI_OUT, Event,EventString);
    }
    
  if(EventString[0]==0)
    {
    strcat(EventString,cmd2str(Event->Command));
    strcat(EventString," ");      
    switch(Event->Command)
      {
      case EVENT_TIME:
        ParameterToView[0]=PAR2_TIME;         // tijd
        ParameterToView[1]=PAR2_WDAY;         // dag van de week of een wildcard
        break;

      case CMD_ALARM_SET:
        ParameterToView[0]=PAR1_INT;          // Alarmnummer
        ParameterToView[1]=PAR2_ALARMENABLED; // On / Off en de tijd
        ParameterToView[2]=PAR2_TIME;         // tijd
        ParameterToView[3]=PAR2_WDAY;         // dag van de week of een wildcard
        break;

      case CMD_BREAK_ON_TIME_LATER:
      case CMD_BREAK_ON_TIME_EARLIER:
      case CMD_CLOCK_TIME:
        ParameterToView[0]=PAR2_TIME;         // tijd
        break;

      case CMD_CLOCK_DATE:
        ParameterToView[0]=PAR2_DATE;         // datum
        break;

      // Par1 en Par2 samengesteld voor weergave van COMMAND <nummer> , <analoge waarde>
      case CMD_BREAK_ON_VAR_EQU:
      case CMD_BREAK_ON_VAR_LESS:
      case CMD_BREAK_ON_VAR_MORE:
      case CMD_BREAK_ON_VAR_NEQU:
      case CMD_VARIABLE_SET:
      case CMD_VARIABLE_INC:
      case CMD_VARIABLE_DEC:
      case EVENT_VARIABLE:
        ParameterToView[0]=PAR1_INT;
        ParameterToView[1]=PAR2_FLOAT;
        break;

      // Par2 als hex waarde, par2 als tekst.
      case CMD_RAWSIGNAL_SHOW:
      case CMD_RAWSIGNAL_SEND:
        ParameterToView[0]=PAR2_INT_HEX;
        ParameterToView[1]=PAR1_TEXT;
        break;

      // Par2 als hex, geen andere parameters.
      case VALUE_HWCONFIG:
      case EVENT_RAWSIGNAL:
        ParameterToView[0]=PAR2_INT_HEX;
        break;


      case CMD_WIRED_SMITTTRIGGER:
      case CMD_WIRED_THRESHOLD:
      case VALUE_WIRED_ANALOG:
        ParameterToView[0]=PAR1_INT;
        ParameterToView[1]=PAR2_INT;
        break;


      // Par2 als decimale int.
      case VALUE_BUILD:
      case VALUE_FREEMEM:
      case CMD_PORT_INPUT:
      case CMD_PORT_OUTPUT:
        ParameterToView[0]=PAR2_INT;
        break;
        
      // Par1 als waarde en par2 als tekst
      case CMD_VARIABLE_SEND:
      case CMD_WIRED_PULLUP:
      case CMD_WIRED_OUT:
      case VALUE_UNIT:
      case EVENT_WIRED_IN:
        ParameterToView[0]=PAR1_INT;
        ParameterToView[1]=PAR2_TEXT;
        break;

        // Par1 als tekst en par2 als tekst
      case CMD_OUTPUT:
      case CMD_RAWSIGNAL_RECEIVE:
        ParameterToView[0]=PAR1_TEXT;
        ParameterToView[1]=PAR2_TEXT;
        break;


      case CMD_VARIABLE_GET:
        ParameterToView[0]=PAR1_INT;
        ParameterToView[1]=PAR2_INT8;
        ParameterToView[2]=PAR3_INT;
      
        break;

      case EVENT_WILDCARD:
        ParameterToView[0]=PAR1_TEXT;
        ParameterToView[1]=PAR2_TEXT;
        ParameterToView[2]=PAR3_INT;
        break;

      // Par1 als tekst en par2 als getal
      case CMD_RAWSIGNAL_SAVE:
      case CMD_STATUS:
        ParameterToView[0]=PAR1_TEXT;
        ParameterToView[1]=PAR2_INT;
        break;

      // Par1 als tekst en par2 niet
      case CMD_SEND_EVENT:
      case CMD_LOCK:
      case CMD_DEBUG:
      case CMD_WAIT_FREE_NODO:
      case CMD_LOG:
      case CMD_ECHO:
      case CMD_WAIT_FREE_RX:
      case CMD_ALIAS_SHOW:
      case CMD_BREAK_ON_DAYLIGHT:
        ParameterToView[0]=PAR1_TEXT;
        break;

      // Par1 als waarde en par2 niet
      case CMD_RAWSIGNAL_SAMPLE:
      case CMD_RAWSIGNAL_DELAY:
      case CMD_RAWSIGNAL_REPEATS:
      case CMD_EVENTLIST_SHOW:
      case CMD_EVENTLIST_ERASE:
      case EVENT_TIMER:
      case EVENT_ALARM:
      case EVENT_BOOT:
      case CMD_DELAY:
      case VALUE_SOURCE_PLUGIN:
      case EVENT_NEWNODO:
      case CMD_VARIABLE_SAVE:
      case CMD_VARIABLE_LOG:
      case CMD_VARIABLE_TOGGLE:
      case CMD_UNIT_SET:
      case CMD_VARIABLE_PULSE_TIME:
      case CMD_VARIABLE_PULSE_COUNT:
        ParameterToView[0]=PAR1_INT;
        break;

      // Par2 int en par1 als tekst
      case CMD_FILE_EXECUTE:
        ParameterToView[0]=PAR2_INT;
        ParameterToView[1]=PAR1_TEXT;
        break;
        
      // Par1 ls int, Par2 als tekst, Par3(uit Par2) als tekst.
      case CMD_SENDTO:
        if(Event->Par1==0)
          {
          Event->Par1=VALUE_OFF;
          ParameterToView[0]=PAR1_TEXT;
          }
        else
          {
          ParameterToView[0]=PAR1_INT;
          ParameterToView[1]=PAR2_TEXT;
          ParameterToView[3]=PAR3_TEXT;
          }
        break;
        
      // geen parameters.
      case CMD_SLEEP:
      case CMD_REBOOT:
      case CMD_CLOCK_SYNC:
      case CMD_RESET:
      case CMD_STOP:
      case CMD_SETTINGS_SAVE:
        break;
        
      // Twee getallen en een aanvullende tekst
      case EVENT_MESSAGE:
        ParameterToView[0]=PAR1_MESSAGE;
        break;

      // Par1 als waarde en par2 als waarde
      default:
        ParameterToView[0]=PAR1_INT;
        ParameterToView[1]=PAR2_INT;
      }
    
    for(byte p=0;p<=3;p++)
      {
      switch (ParameterToView[p])
        {
        case PAR1_INT:
          strcat(EventString,int2str(Event->Par1));
          break;
          
        case PAR1_TEXT:
          strcat(EventString,cmd2str(Event->Par1));
          break;
        
        case PAR1_MESSAGE:
          strcat(EventString,int2str(Event->Par1));
          strcat(EventString, ",");
          strcat(EventString,int2str(Event->Par2));

          if(Event->Par1<=MESSAGE_MAX)
            {
            strcat(EventString, ": ");
            strcpy_P(str,(char*)pgm_read_word(&(MessageText_tabel[Event->Par1])));
            strcat(EventString,str);
            }
          break;

        case PAR2_INT:
          strcat(EventString,int2str(Event->Par2));
          break;

        case PAR2_INT8:
          strcat(EventString,int2str(Event->Par2&0xff));
          break;

        case PAR3_INT:
          strcat(EventString,int2str((Event->Par2>>8)&0xff));
          break;

        case PAR4_INT:
          strcat(EventString,int2str((Event->Par2>>16)&0xff));
          break;

        case PAR5_INT:
          strcat(EventString,int2str((Event->Par2>>24)&0xff));
          break;

        case PAR2_TEXT:
          strcat(EventString,cmd2str(Event->Par2 & 0xff));
          break;

        case PAR3_TEXT:
          strcat(EventString,cmd2str((Event->Par2>>8) & 0xff));
          break;

        case PAR2_DIM:
          if(Event->Par2==VALUE_OFF || Event->Par2==VALUE_ON)
            strcat(EventString, cmd2str(Event->Par2)); // Print 'On' of 'Off'
          else
            strcat(EventString,int2str(Event->Par2));
          break;

        case PAR2_ALARMENABLED:
          // Alarm on/of zit in de 20e bit van Par2.
          if((Event->Par2>>20)&1)
            strcat(EventString, cmd2str(VALUE_ON)); // Print 'On' of 'Off'
          else
            strcat(EventString, cmd2str(VALUE_OFF)); // Print 'On' of 'Off'
          break;
        
        case PAR2_TIME:
          x=(Event->Par2>>12)&0xf;
          if(x==0xf)
            strcat(EventString, "*");
          else if(x==0xe)
            strcat(EventString, "-");
          else
            strcat(EventString, int2str(x));
          
          x=(Event->Par2>>8)&0xf;
          if(x==0xf)
            strcat(EventString, "*");
          else if(x==0xe)
            strcat(EventString, "-");
          else
            strcat(EventString, int2str(x));
          
          strcat(EventString, ":");

          x=(Event->Par2>>4)&0xf;
          if(x==0xf)
            strcat(EventString, "*");
          else if(x==0xe)
            strcat(EventString, "-");
          else
            strcat(EventString, int2str(x));

          x=(Event->Par2)&0xf;
          if(x==0xf)
            strcat(EventString, "*");
          else if(x==0xe)
            strcat(EventString, "-");
          else
            strcat(EventString, int2str(x));
          break;
          
        case PAR2_DATE:
          strcat(EventString, int2str((Event->Par2>>28)&0xf));
          strcat(EventString, int2str((Event->Par2>>24)&0xf));
          strcat(EventString, "-");
          strcat(EventString, int2str((Event->Par2>>20)&0xf));
          strcat(EventString, int2str((Event->Par2>>16)&0xf));
          strcat(EventString, "-");
          strcat(EventString, int2str((Event->Par2>>12)&0xf));
          strcat(EventString, int2str((Event->Par2>>8 )&0xf));
          strcat(EventString, int2str((Event->Par2>>4 )&0xf));
          strcat(EventString, int2str((Event->Par2    )&0xf));
          break;
          
        case PAR2_WDAY:
          // dag van de week zit in bits 17,18, 19 en 20. Waarde 0xf is wildcard, andere waarden vertegenwoordigen de dag van de week.
          x=(Event->Par2>>16)&0xf;
          if(x==0xf || x==0)
            strcat(EventString, "*");
          else if(x==0xe)
            strcat(EventString, "---");
          else            
            {
            char *StrPtr=ProgmemString(Text_04)+((x-1)*3);
            *(StrPtr+3)=0;
            strcat(EventString, StrPtr);
            }
          break;

        case PAR2_FLOAT:
          // Een float en een unsigned long zijn beide 4bytes groot. We gebruiken ruimte van Par2 om een float in op te slaan
          float f;
          memcpy(&f, &Event->Par2, 4);
          
          // Als de float (bij benadering) .00 is, dan alleen de cijfers voor de komma weergeven.
          x=0;
          if((((int)(f*(1000))-((int)f)*(1000)))!=0)
            x=3;
            
          dtostrf(f, 0, x,EventString+strlen(EventString)); // Kaboem... 2100 bytes programmacode extra ! Gelukkig alleen voor de mega.
          break;

        case PAR2_INT_HEX:
          strcat(EventString,int2strhex(Event->Par2));
          break;

        }

      if(ParameterToView[p+1]!=0)// komt er nog een vervolgparameter, dan een komma als scheidingsteken.
        strcat(EventString,",");      
      }
    }          
  free(str);
  }


 /*******************************************************************************************************\
 * Deze funktie parsed een string zoals die door de gebruiker wordt opgegeven. Commando, Par1 en Par2
 * van struct ResultEvent worden gevuld. 
 \*******************************************************************************************************/
boolean Str2Event(char *Command, struct NodoEventStruct *ResultEvent)
  {
  char *TmpStr1=(char*)malloc(INPUT_COMMAND_SIZE);
  int w,x,y;
  byte error=0;
  unsigned long a;
  //struct NodoEventStruct EventToExecute,TempEvent;

  // Bouw een nieuw event op.
  ClearEvent(ResultEvent);
  GetArgv(Command,TmpStr1,1);
  ResultEvent->Command=str2cmd(TmpStr1); 

  // Haal Par1 uit het commando.
  if(GetArgv(Command,TmpStr1,2))
    {
    ResultEvent->Par1=str2cmd(TmpStr1);
    if(!ResultEvent->Par1)
      ResultEvent->Par1=str2int(TmpStr1);
    }
  
  // Haal Par2 uit het commando.
  if(GetArgv(Command,TmpStr1,3))
    {
    ResultEvent->Par2=str2cmd(TmpStr1);
    if(!ResultEvent->Par2)
      ResultEvent->Par2=str2int(TmpStr1);
    }        

  switch(ResultEvent->Command)
    {
    //test; geen, altijd goed
    case EVENT_CLOCK_DAYLIGHT:
    case EVENT_USEREVENT:
      ResultEvent->Type=NODO_TYPE_EVENT;
      break; 

    // altijd goed
    case CMD_SLEEP:
    case CMD_EVENTLIST_SHOW:
    case CMD_EVENTLIST_ERASE:
    case CMD_STOP:
    case CMD_RESET:
    case CMD_REBOOT:
    case CMD_SETTINGS_SAVE:
    case CMD_STATUS:
    case CMD_DELAY:
    case CMD_SOUND: 
    case CMD_SEND_USEREVENT:
    case CMD_CLOCK_SYNC:
      ResultEvent->Type=NODO_TYPE_COMMAND;
      break; 

    case CMD_LOG:
    case CMD_WAIT_FREE_NODO:
    case CMD_DEBUG:
    case CMD_ECHO:
    case CMD_RAWSIGNAL_RECEIVE:
      ResultEvent->Type=NODO_TYPE_COMMAND;
      if(ResultEvent->Par1!=VALUE_OFF && ResultEvent->Par1!=VALUE_ON)
      
        error=MESSAGE_INVALID_PARAMETER;
     break;
                  
    case EVENT_MESSAGE:
      ResultEvent->Type=NODO_TYPE_EVENT; 
      if(ResultEvent->Par1 <1 || ResultEvent->Par1>MESSAGE_MAX)
        error=MESSAGE_INVALID_PARAMETER;
      break;
        
    case CMD_TIMER_SET:
      ResultEvent->Type=NODO_TYPE_COMMAND;
      if(ResultEvent->Par1==0 || ResultEvent->Par1>TIMER_MAX)
        error=MESSAGE_INVALID_PARAMETER;
      break;
            
    case CMD_TIMER_SET_VARIABLE:
      ResultEvent->Type=NODO_TYPE_COMMAND;
      if(ResultEvent->Par1>TIMER_MAX || ResultEvent->Par2<1 || ResultEvent->Par2>USER_VARIABLES_MAX)
        error=MESSAGE_INVALID_PARAMETER;
      break;
            
    case EVENT_ALARM:
      ResultEvent->Type=NODO_TYPE_EVENT;
      if(ResultEvent->Par1<1 || ResultEvent->Par1>ALARM_MAX)
        error=MESSAGE_INVALID_PARAMETER;

    case EVENT_NEWNODO:
    case EVENT_BOOT:
      ResultEvent->Type=NODO_TYPE_EVENT;
      if(ResultEvent->Par1<1 || ResultEvent->Par1>UNIT_MAX)
        error=MESSAGE_INVALID_PARAMETER;
      break;

    case CMD_UNIT_SET:
      ResultEvent->Type=NODO_TYPE_COMMAND;
      if(ResultEvent->Par1<1 || ResultEvent->Par1>UNIT_MAX)
        error=MESSAGE_INVALID_PARAMETER;
      break;

    case CMD_ANALYSE_SETTINGS:
      ResultEvent->Type=NODO_TYPE_COMMAND;
      if(ResultEvent->Par1<1 || ResultEvent->Par1>50)
        error=MESSAGE_INVALID_PARAMETER;
      break;

    case CMD_VARIABLE_PULSE_TIME:
    case CMD_VARIABLE_PULSE_COUNT:
      ResultEvent->Type=NODO_TYPE_COMMAND;
      if(ResultEvent->Par1<1 || ResultEvent->Par1>USER_VARIABLES_MAX)
        error=MESSAGE_INVALID_PARAMETER;
      break;

    case CMD_BREAK_ON_VAR_LESS_VAR:
    case CMD_BREAK_ON_VAR_MORE_VAR:
    case CMD_VARIABLE_VARIABLE:
      ResultEvent->Type=NODO_TYPE_COMMAND;
      if(ResultEvent->Par1<1 || ResultEvent->Par1>USER_VARIABLES_MAX)
        error=MESSAGE_INVALID_PARAMETER;
      if(ResultEvent->Par2<1 || ResultEvent->Par2>USER_VARIABLES_MAX)
        error=MESSAGE_INVALID_PARAMETER;
      break;
        
    case CMD_VARIABLE_SET_WIRED_ANALOG:
      ResultEvent->Type=NODO_TYPE_COMMAND;
      if(ResultEvent->Par1<1 || ResultEvent->Par1>USER_VARIABLES_MAX)
        error=MESSAGE_INVALID_PARAMETER;
      if(ResultEvent->Par2<1 || ResultEvent->Par2>WIRED_PORTS)
        error=MESSAGE_INVALID_PARAMETER;
      break;
        
    case CMD_TIMER_RANDOM:
      ResultEvent->Type=NODO_TYPE_COMMAND;
      if(ResultEvent->Par1<1 || ResultEvent->Par1>TIMER_MAX)
        error=MESSAGE_INVALID_PARAMETER;
      break;

    case EVENT_TIMER:
      ResultEvent->Type=NODO_TYPE_EVENT;
      if(ResultEvent->Par1<1 || ResultEvent->Par1>TIMER_MAX)
        error=MESSAGE_INVALID_PARAMETER;
      break;
                
    // geldige tijd    
    case CMD_BREAK_ON_TIME_LATER:
    case CMD_BREAK_ON_TIME_EARLIER:
    case CMD_CLOCK_TIME:
      ResultEvent->Type=NODO_TYPE_COMMAND;
      error=MESSAGE_INVALID_PARAMETER;
      if(GetArgv(Command,TmpStr1,2))
        {
        ResultEvent->Par1=0;
        if((ResultEvent->Par2=str2ultime(TmpStr1))!=0xffffffff)
          error=0;
        }              
      break;

    // geldige datum
    case CMD_CLOCK_DATE:
      ResultEvent->Type=NODO_TYPE_COMMAND;
      // datum in Par2 volgens format DDMMYYYY. 
      ResultEvent->Par1=0;
      if((ResultEvent->Par2=str2uldate(Command))==0xffffffff)
        error=MESSAGE_INVALID_PARAMETER;
      break;
      
    // test:ResultEvent->Par1 binnen bereik maximaal beschikbare wired poorten.
    case EVENT_WIRED_IN:
      ResultEvent->Type=NODO_TYPE_EVENT;
      if(ResultEvent->Par1<1 || ResultEvent->Par1>WIRED_PORTS)
        error=MESSAGE_INVALID_PARAMETER;
      if(ResultEvent->Par2!=VALUE_ON && ResultEvent->Par2!=VALUE_OFF)
        error=MESSAGE_INVALID_PARAMETER;
      break;

    case CMD_WIRED_OUT:
      ResultEvent->Type=NODO_TYPE_COMMAND;
      if(ResultEvent->Par1>WIRED_PORTS)
        error=MESSAGE_INVALID_PARAMETER;
      if(ResultEvent->Par2!=VALUE_ON && ResultEvent->Par2!=VALUE_OFF)
        error=MESSAGE_INVALID_PARAMETER;
      break;

    case CMD_WIRED_PULLUP:
      ResultEvent->Type=NODO_TYPE_COMMAND;
      if(ResultEvent->Par1<1 || ResultEvent->Par1>WIRED_PORTS)
        error=MESSAGE_INVALID_PARAMETER;
      if(ResultEvent->Par2!=VALUE_ON && ResultEvent->Par2!=VALUE_OFF)
        error=MESSAGE_INVALID_PARAMETER;
      break;

    case CMD_SEND_EVENT:
      ResultEvent->Type=NODO_TYPE_COMMAND;
      if(ResultEvent->Par2==0);
        ResultEvent->Par2=VALUE_ALL;
      switch(ResultEvent->Par1)
        {
        case VALUE_ALL:
        case VALUE_SOURCE_I2C:
        case VALUE_SOURCE_IR:
        case VALUE_SOURCE_RF:
        case VALUE_SOURCE_HTTP:
          break;
        default:
          error=MESSAGE_INVALID_PARAMETER;
        }
      break;

    case CMD_VARIABLE_SEND:
      ResultEvent->Type=NODO_TYPE_COMMAND;
      if(ResultEvent->Par1>USER_VARIABLES_MAX)
        error=MESSAGE_INVALID_PARAMETER;        
      switch(ResultEvent->Par2)
        {
        case 0:
          ResultEvent->Par2=VALUE_ALL;
          break;          
        case VALUE_ALL:
        case VALUE_SOURCE_I2C:
        case VALUE_SOURCE_IR:
        case VALUE_SOURCE_RF:
        case VALUE_SOURCE_HTTP:
          break;
          
        default:
          error=MESSAGE_INVALID_PARAMETER;
        }
      break;

    case EVENT_WILDCARD:
      ResultEvent->Type=NODO_TYPE_EVENT;
      switch(ResultEvent->Par1)
        {
        case VALUE_ALL:
        case VALUE_SOURCE_CLOCK:
        case VALUE_SOURCE_SYSTEM:
        case VALUE_SOURCE_EVENTLIST:
        case VALUE_SOURCE_WIRED:
        case VALUE_SOURCE_I2C:
        case VALUE_SOURCE_IR:
        case VALUE_SOURCE_RF:
        case VALUE_SOURCE_SERIAL:
        case VALUE_SOURCE_HTTP:
        case VALUE_SOURCE_TELNET:
        case VALUE_SOURCE_PLUGIN:
          break;
        default:
          error=MESSAGE_INVALID_PARAMETER;
        }
        
      switch(ResultEvent->Par2)
        {
        case VALUE_ALL:
        case EVENT_USEREVENT:
        case EVENT_CLOCK_DAYLIGHT:
        case EVENT_TIME:
        case EVENT_RAWSIGNAL:
        case EVENT_TIMER:
        case EVENT_WIRED_IN:
        case EVENT_VARIABLE:
        case EVENT_NEWNODO:
        case EVENT_MESSAGE:
        case EVENT_BOOT:
        case EVENT_ALARM:
          break;
        default:
          error=MESSAGE_INVALID_PARAMETER;
        }
        
      if(GetArgv(Command,TmpStr1,4))
        ResultEvent->Par2|=(str2int(TmpStr1)<<8);

      break;
      
     // par1 alleen On of Off.
     // par2 mag alles zijn
    case CMD_ALIAS_SHOW:
    case CMD_BREAK_ON_DAYLIGHT:
    case CMD_WAIT_FREE_RX:
      ResultEvent->Type=NODO_TYPE_COMMAND;
      if(ResultEvent->Par1!=VALUE_OFF && ResultEvent->Par1!=VALUE_ON)
        error=MESSAGE_INVALID_PARAMETER;
      break;

    case CMD_RAWSIGNAL_SAVE:
      ResultEvent->Type=NODO_TYPE_COMMAND;
      if(ResultEvent->Par1!=VALUE_ON && ResultEvent->Par1!=VALUE_OFF && ResultEvent->Par1!=0)
        error=MESSAGE_INVALID_PARAMETER;
      if(!(ResultEvent->Par2==0 || (ResultEvent->Par2>=2  && ResultEvent->Par2<=5 )))
        error=MESSAGE_INVALID_PARAMETER;
      break;
      
    case CMD_OUTPUT:
      ResultEvent->Type=NODO_TYPE_COMMAND;
      if(ResultEvent->Par1!=VALUE_SOURCE_I2C && ResultEvent->Par1!=VALUE_SOURCE_IR && ResultEvent->Par1!=VALUE_SOURCE_RF && ResultEvent->Par1!=VALUE_SOURCE_HTTP)
        error=MESSAGE_INVALID_PARAMETER;
      if(ResultEvent->Par2!=VALUE_OFF && ResultEvent->Par2!=VALUE_ON && (ResultEvent->Par2!=VALUE_ALL && ResultEvent->Par1!=VALUE_SOURCE_HTTP))
        error=MESSAGE_INVALID_PARAMETER;
      break;


    case CMD_LOCK: // Hier wordt de lock code o.b.v. het wachtwoord ingesteld. Alleen van toepassing voor de Mega
      ResultEvent->Type=NODO_TYPE_COMMAND;
      ResultEvent->Par2=0L;
      for(x=0;x<strlen(Settings.Password);x++)
        {// beetje hutselen met bitjes ;-)
        ResultEvent->Par2=ResultEvent->Par2<<5;
        ResultEvent->Par2^=Settings.Password[x];
        }
      break;

    case CMD_VARIABLE_TOGGLE:
    case CMD_VARIABLE_SAVE:
    case CMD_VARIABLE_LOG:
      ResultEvent->Type=NODO_TYPE_COMMAND;
      if(ResultEvent->Par1>USER_VARIABLES_MAX)
        error=MESSAGE_INVALID_PARAMETER;
      break;
      
    case CMD_VARIABLE_SET:
      ResultEvent->Type=NODO_TYPE_COMMAND;
      if(ResultEvent->Par1>USER_VARIABLES_MAX)
        error=MESSAGE_INVALID_PARAMETER;
      else if(GetArgv(Command,TmpStr1,3))// waarde van de variabele
        ResultEvent->Par2=float2ul(atof(TmpStr1));
      else
        error=MESSAGE_INVALID_PARAMETER;
      break;

    case CMD_VARIABLE_GET: // VariableGet <Variabele>, <Unit>, <VariabeleBron> 
      ResultEvent->Type=NODO_TYPE_COMMAND;
      error=MESSAGE_INVALID_PARAMETER;

      if(ResultEvent->Par1>0 && ResultEvent->Par1<=USER_VARIABLES_MAX)
        {
        if(ResultEvent->Par2>0 && ResultEvent->Par2<=UNIT_MAX)
          {
          if(GetArgv(Command,TmpStr1,4))// VariabeleBron
            {
            x=str2int(TmpStr1);
            if(x>0 && x<=USER_VARIABLES_MAX)
              {
              ResultEvent->Par2|=(x<<8);
              error=0;
              }
            }
          }
        }
       break;

    case CMD_BREAK_ON_VAR_EQU:
    case CMD_BREAK_ON_VAR_LESS:
    case CMD_BREAK_ON_VAR_MORE:
    case CMD_BREAK_ON_VAR_NEQU:
    case CMD_VARIABLE_DEC:
    case CMD_VARIABLE_INC:
      ResultEvent->Type=NODO_TYPE_COMMAND;
      if(ResultEvent->Par1<1 || ResultEvent->Par1>USER_VARIABLES_MAX)
        error=MESSAGE_INVALID_PARAMETER;
      else if(GetArgv(Command,TmpStr1,3))// waarde van de variabele
        ResultEvent->Par2=float2ul(atof(TmpStr1));
      else
        error=MESSAGE_INVALID_PARAMETER;
      break;


    case EVENT_VARIABLE:
      ResultEvent->Type=NODO_TYPE_EVENT;
      if(ResultEvent->Par1<1 || ResultEvent->Par1>USER_VARIABLES_MAX)
        error=MESSAGE_INVALID_PARAMETER;
      else if(GetArgv(Command,TmpStr1,3))// waarde van de variabele
        ResultEvent->Par2=float2ul(atof(TmpStr1));
      else
        error=MESSAGE_INVALID_PARAMETER;
      break;

    case VALUE_WIRED_ANALOG:
    case CMD_WIRED_THRESHOLD:
    case CMD_WIRED_SMITTTRIGGER:
      ResultEvent->Type=NODO_TYPE_COMMAND;
      if(ResultEvent->Par1<1 || ResultEvent->Par1>WIRED_PORTS)
        error=MESSAGE_INVALID_PARAMETER;
      else if(GetArgv(Command,TmpStr1,3))
        ResultEvent->Par2=str2int(TmpStr1);
      break;

    case EVENT_RAWSIGNAL:
      ResultEvent->Type=NODO_TYPE_RAWSIGNAL;
      if(GetArgv(Command,TmpStr1,2))
        {
        ResultEvent->Par1=0;
        ResultEvent->Par2=str2int(TmpStr1);
        }
      break;

    case CMD_ALARM_SET:
      ResultEvent->Type=NODO_TYPE_COMMAND;
      // Commando format: [AlarmSet <AlarmNumber 1..4>, <Enabled On|Off>, <Time HHMM>, <Day 1..7>]
      //                  [Time <Time HHMM>, <Day 1..7>]
      // We moeten wat truucs uithalen om al deze info in een 32-bit variabele te krijgen.
      // Alarmtijd wordt in Par2 opgeslagen volgens volgende format: MSB-EEEEWWWWAAAABBBBCCCCDDDD-LSB
      // E=Enabled, WWWW=weekdag, AAAA=Uren tiental, BBBB=uren, CCCC=minuten tiental DDDD=minuten
      // Als een deel gevuld met 0xE, dan waarde niet setten.
      // Als gevuld met 0xF, dan wildcard.             
      if(GetArgv(Command,TmpStr1,2)) // Alarm number
        {
        error=MESSAGE_INVALID_PARAMETER;
        ResultEvent->Par1=str2int(TmpStr1);
        ResultEvent->Par2=0xEEEEEEEE; 
        
        if(ResultEvent->Par1>0 && ResultEvent->Par1<=ALARM_MAX)
          {
          if(GetArgv(Command,TmpStr1,3)) // Enabled
            {
            x=str2cmd(TmpStr1);
            if(x==VALUE_ON || x==VALUE_OFF)
              {
              ResultEvent->Par2&=0xff0fffff;
              ResultEvent->Par2|=(unsigned long)(x==VALUE_ON)<<20; // Enabled bit setten.
              error=0;
              if(GetArgv(Command,TmpStr1,4)) // Minutes
                {
                ResultEvent->Par2&=0xffff0000;
                y=0;
                for(x=strlen(TmpStr1)-1;x>=0;x--)
                  {
                  w=TmpStr1[x];
                  if(w>='0' && w<='9' || w=='*')
                    {
                    if(w=='*')
                      ResultEvent->Par2|=(0xFUL<<y); // vul nibble met wildcard
                    else
                      ResultEvent->Par2|=(w-'0')<<y; // vul nibble met token
                    y+=4;
                    }
                  else if(w==':');
                  else
                    {
                    error=MESSAGE_INVALID_PARAMETER;
                    break;
                    }
                  }
                if(GetArgv(Command,TmpStr1,5)) // Day is optioneel. Maar als deze parameter ingevuld, dan meenemen in de berekening.
                  y=str2weekday (TmpStr1);
                else // Dag niet opgegeven
                  y=0xF;

                ResultEvent->Par2&=0xfff0ffff;
                if(y!=-1)
                  ResultEvent->Par2|=(unsigned long)y<<16;
                else
                  error=MESSAGE_INVALID_PARAMETER;
                }
              }
            }
          }
        }
      else 
        error=MESSAGE_INVALID_PARAMETER;

      break;
    
    case EVENT_TIME:
      // Event format: [Time <Time HHMM>, <Day 1..7>]
      // We moeten wat truucs uithalen om al deze info in een 32-bit variabele te krijgen.
      // Tijd wordt in Par2 opgeslagen volgens volgende format: MSB-0000WWWWAAAABBBBCCCCDDDD-LSB
      // WWWW=weekdag, AAAA=Uren tiental, BBBB=uren, CCCC=minuten tiental DDDD=minuten
      {              
      ResultEvent->Type=NODO_TYPE_EVENT;
      if(GetArgv(Command,TmpStr1,2)) // Minutes
        {
        ResultEvent->Par2=0L; 
        y=0;
        for(x=strlen(TmpStr1)-1;x>=0;x--)
          {
          w=TmpStr1[x];
          if(w>='0' && w<='9' || w=='*')
            {
            a=0xffffffff  ^ (0xfUL <<y); // Mask maken om de nibble positie y te wissen.
            ResultEvent->Par2&=a; // maak nibble leeg
            if(w=='*')
              ResultEvent->Par2|=(0xFUL<<y); // vul nibble met wildcard
            else
              ResultEvent->Par2|=(w-'0')<<y; // vul nibble met token
            y+=4;
            }
          else if(w==':');
          else
            {
            error=MESSAGE_INVALID_PARAMETER;
            break;
            }
          }
        if(GetArgv(Command,TmpStr1,3)) // Day is optioneel. Maar als deze parameter ingevuld, dan meenemen in de berekening.
          y=str2weekday(TmpStr1);
        else // Dag niet opgegeven
          y=0xF;

        ResultEvent->Par2&=0xfff0ffff;
        if(y!=-1)
          ResultEvent->Par2|=(unsigned long)y<<16;
        else
          error=MESSAGE_INVALID_PARAMETER;
        }
      else 
        error=MESSAGE_INVALID_PARAMETER;
      break;
      }

    case CMD_RAWSIGNAL_SHOW:      
    case CMD_RAWSIGNAL_SEND:      
      ResultEvent->Type=NODO_TYPE_COMMAND;
      ResultEvent->Par1=VALUE_ALL;
      if(GetArgv(Command,TmpStr1,3))
        ResultEvent->Par1=str2cmd(TmpStr1);            // Haal Par2 uit het commando. let op Par2 gebruiker wordt opgeslagen in struct Par1.
      if(GetArgv(Command,TmpStr1,2))
        ResultEvent->Par2=str2int(TmpStr1);            // Haal Par1 uit het commando. let op Par1 gebruiker is een 32-bit hex-getal die wordt opgeslagen in struct Par2.
      break;

    case CMD_PORT_INPUT:
    case CMD_PORT_OUTPUT:
      {
      ResultEvent->Type=NODO_TYPE_COMMAND;
      if(GetArgv(Command,TmpStr1,2))
        ResultEvent->Par2=str2int(TmpStr1);
      if(ResultEvent->Par2 > 0xffff)
        error=MESSAGE_INVALID_PARAMETER;
      break;
      }  

    default:
      {              
      error=MESSAGE_UNKNOWN_COMMAND;
      }
    }      
  free(TmpStr1);
  return error;
  }

#else

/**********************************************************************************************\
 * Print de welkomsttekst van de Nodo. ATMega328 variant
 \*********************************************************************************************/

void PrintWelcome(void)
  {
  // Print Welkomsttekst
  //Serial.println(F("!******************************************************************************!"));
  //Serial.println(F("Nodo Domotica controller (c) Copyright 2014 P.K.Tonkes."));
  //Serial.println(F("Licensed under GNU General Public License."));
  //Serial.print(F("Nodo-Small V3.7, Product=SWACNC-SMALL-R"));
  //Serial.print(NODO_BUILD);
  //Serial.print(F(", ThisUnit="));
  //Serial.println(Settings.Unit);
  //Serial.println(F("!******************************************************************************!"));
  }

#endif
  
