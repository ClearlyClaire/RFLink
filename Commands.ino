/*********************************************************************************************\
 * Deze functie checked of de code die ontvangen is een uitvoerbare opdracht is/
 * Als het een correct commando is wordt deze uitgevoerd en 
 * true teruggegeven. Zo niet dan wordt er een 'false' retour gegeven.
 \*********************************************************************************************/

boolean ExecuteCommand(struct NodoEventStruct *EventToExecute)
  {
  unsigned long a;
  int w,x,y,z;
  byte error=0;
  
  struct NodoEventStruct TempEvent,TempEvent2;
  ClearEvent(&TempEvent);
  ClearEvent(&TempEvent2);
  
  #if NODO_MEGA
  char *TempString=(char*)malloc(80);
  char *TempString2=(char*)malloc(15);
  #endif
  
  switch(EventToExecute->Command)
    {   
    case CMD_VARIABLE_TOGGLE:
      UserVar[EventToExecute->Par1-1]=UserVar[EventToExecute->Par1-1]>0.5?0.0:1.0;
      TempEvent.Type         = NODO_TYPE_EVENT;
      TempEvent.Command      = EVENT_VARIABLE;
      TempEvent.Par1         = EventToExecute->Par1;
      TempEvent.Par2         = float2ul(UserVar[EventToExecute->Par1-1]);
      TempEvent.Direction    = VALUE_DIRECTION_INPUT;
      TempEvent.Port         = VALUE_SOURCE_SYSTEM;
      ProcessEvent(&TempEvent);
      break;        

    case CMD_VARIABLE_INC:
      UserVar[EventToExecute->Par1-1]+=ul2float(EventToExecute->Par2);
      TempEvent.Type         = NODO_TYPE_EVENT;
      TempEvent.Command      = EVENT_VARIABLE;
      TempEvent.Par1         = EventToExecute->Par1;
      TempEvent.Par2         = float2ul(UserVar[EventToExecute->Par1-1]);
      TempEvent.Direction    = VALUE_DIRECTION_INPUT;
      TempEvent.Port         = VALUE_SOURCE_SYSTEM;
      ProcessEvent(&TempEvent);
      break;        

    case CMD_VARIABLE_DEC:
      UserVar[EventToExecute->Par1-1]-=ul2float(EventToExecute->Par2);
      TempEvent.Type         = NODO_TYPE_EVENT;
      TempEvent.Command      = EVENT_VARIABLE;
      TempEvent.Par1         = EventToExecute->Par1;
      TempEvent.Par2         = float2ul(UserVar[EventToExecute->Par1-1]);
      TempEvent.Direction    = VALUE_DIRECTION_INPUT;
      TempEvent.Port         = VALUE_SOURCE_SYSTEM;
      ProcessEvent(&TempEvent);
      break;        

    case CMD_VARIABLE_SET:
      UserVar[EventToExecute->Par1-1]=ul2float(EventToExecute->Par2);
      TempEvent.SourceUnit   = Settings.Unit;
      TempEvent.Type         = NODO_TYPE_EVENT;
      TempEvent.Command      = EVENT_VARIABLE;
      TempEvent.Port         = VALUE_SOURCE_SYSTEM;
      TempEvent.Direction    = VALUE_DIRECTION_INPUT;
      TempEvent.Par1         = EventToExecute->Par1;
      TempEvent.Par2         = EventToExecute->Par2;
      ProcessEvent(&TempEvent);      // verwerk binnengekomen event.
      break;         

    #if WIRED
    case CMD_VARIABLE_SET_WIRED_ANALOG:
      UserVar[EventToExecute->Par1-1]=analogRead(PIN_WIRED_IN_1+EventToExecute->Par2-1);
      TempEvent.Par1         = EventToExecute->Par1;
      TempEvent.Par2=float2ul(UserVar[EventToExecute->Par1-1]);
      TempEvent.Type         = NODO_TYPE_EVENT;
      TempEvent.Command=EVENT_VARIABLE;
      TempEvent.Port=VALUE_SOURCE_SYSTEM;
      TempEvent.Direction=VALUE_DIRECTION_INPUT;
      ProcessEvent(&TempEvent);      // verwerk binnengekomen event.
      break;
    #endif         
  
    case CMD_VARIABLE_VARIABLE:
      UserVar[EventToExecute->Par1-1]=UserVar[EventToExecute->Par2-1];
      TempEvent.Type         = NODO_TYPE_EVENT;
      TempEvent.Command=EVENT_VARIABLE;
      TempEvent.Port=VALUE_SOURCE_SYSTEM;
      TempEvent.Par1         = EventToExecute->Par1;
      TempEvent.Par2=float2ul(UserVar[EventToExecute->Par1-1]);
      TempEvent.Direction=VALUE_DIRECTION_INPUT;
      ProcessEvent(&TempEvent);      // verwerk binnengekomen event.
      break;        

    case CMD_VARIABLE_PULSE_COUNT:
      // Tellen van pulsen actief: enable IRQ behorende bij PIN_IR_RX_DATA
      // Als er toch een reeks pulsen komt, dan wordt in FetchSignal() het tellen van pulsen gedisabled.
      bitWrite(HW_Config,HW_PULSE,true);
      attachInterrupt(PULSE_IRQ,PulseCounterISR,PULSE_TRANSITION); 
      
      UserVar[EventToExecute->Par1-1]=PulseCount;
      TempEvent.Par1         = EventToExecute->Par1;
      TempEvent.Par2=float2ul(UserVar[EventToExecute->Par1-1]);
      TempEvent.Command=EVENT_VARIABLE;
      TempEvent.Type=NODO_TYPE_EVENT;
      TempEvent.Port=VALUE_SOURCE_SYSTEM;
      TempEvent.Direction=VALUE_DIRECTION_INPUT;
      PulseCount=0;
      ProcessEvent(&TempEvent);      // verwerk binnengekomen event.
      break;         

    case CMD_VARIABLE_PULSE_TIME:
      // Tellen van pulsen actief: enable IRQ behorende bij PIN_IR_RX_DATA
      // Als er toch een reeks pulsen komt, dan wordt in FetchSignal() het tellen van pulsen gedisabled.
      bitWrite(HW_Config,HW_PULSE,true);
      attachInterrupt(PULSE_IRQ,PulseCounterISR,PULSE_TRANSITION); 

      UserVar[EventToExecute->Par1-1]=PulseTime;
      TempEvent.Par1         = EventToExecute->Par1;
      TempEvent.Par2=float2ul(UserVar[EventToExecute->Par1-1]);
      TempEvent.Type= NODO_TYPE_EVENT;
      TempEvent.Command=EVENT_VARIABLE;
      TempEvent.Port=VALUE_SOURCE_SYSTEM;
      TempEvent.Direction=VALUE_DIRECTION_INPUT;
      ProcessEvent(&TempEvent);      // verwerk binnengekomen event.
      break;         

    case CMD_STOP:
      error=MESSAGE_EXECUTION_STOPPED;
      break;

    case CMD_BREAK_ON_VAR_EQU:
      {
      if((int)UserVar[EventToExecute->Par1-1]==(int)ul2float(EventToExecute->Par2))
        error=MESSAGE_BREAK;
      break;
      }
      
    case CMD_BREAK_ON_VAR_NEQU:
      if((int)UserVar[EventToExecute->Par1-1]!=(int)ul2float(EventToExecute->Par2))
        error=MESSAGE_BREAK;
      break;

    case CMD_BREAK_ON_VAR_MORE:
      if(UserVar[EventToExecute->Par1-1] > ul2float(EventToExecute->Par2))
        error=MESSAGE_BREAK;
      break;

    case CMD_BREAK_ON_VAR_LESS:
      if(UserVar[EventToExecute->Par1-1] < ul2float(EventToExecute->Par2))
        error=MESSAGE_BREAK;
      break;

    case CMD_BREAK_ON_VAR_LESS_VAR:
      if(UserVar[EventToExecute->Par1-1] < UserVar[EventToExecute->Par2-1])
        error=MESSAGE_BREAK;
      break;

    case CMD_BREAK_ON_VAR_MORE_VAR:
      if(UserVar[EventToExecute->Par1-1] > UserVar[EventToExecute->Par2-1])
        error=MESSAGE_BREAK;
      break;


    #if CLOCK
    case CMD_BREAK_ON_DAYLIGHT:
      if(EventToExecute->Par1==VALUE_ON && (Time.Daylight==2 || Time.Daylight==3))
        error=MESSAGE_BREAK;

      if(EventToExecute->Par1==VALUE_OFF && (Time.Daylight==0 || Time.Daylight==1 || Time.Daylight==4))
        error=MESSAGE_BREAK;
      break;

    case CMD_BREAK_ON_TIME_LATER:
      if(EventToExecute->Par2<(Time.Minutes%10 | (unsigned long)(Time.Minutes/10)<<4 | (unsigned long)(Time.Hour%10)<<8 | (unsigned long)(Time.Hour/10)<<12))
        error=MESSAGE_BREAK;
      break;

    case CMD_BREAK_ON_TIME_EARLIER:
      if(EventToExecute->Par2>(Time.Minutes%10 | (unsigned long)(Time.Minutes/10)<<4 | (unsigned long)(Time.Hour%10)<<8 | (unsigned long)(Time.Hour/10)<<12))
        error=MESSAGE_BREAK;
      break;
    #endif CLOCK 


    case CMD_SEND_USEREVENT:
      TempEvent.Port                  = VALUE_ALL;
      TempEvent.Type                  = NODO_TYPE_EVENT;
      TempEvent.Command               = EVENT_USEREVENT;
      TempEvent.Par1                  = EventToExecute->Par1;
      TempEvent.Par2                  = EventToExecute->Par2;
      SendEvent(&TempEvent, false, true,Settings.WaitFree==VALUE_ON);
      break;

    case CMD_VARIABLE_SEND:
      TempEvent.Type=NODO_TYPE_EVENT;
      TempEvent.Command=EVENT_VARIABLE;
      TempEvent.Port=EventToExecute->Par2;
      TempEvent.Direction=VALUE_DIRECTION_OUTPUT;
      TempEvent.Par1=EventToExecute->Par1;
      TempEvent.Par2=float2ul(UserVar[EventToExecute->Par1-1]);
      SendEvent(&TempEvent, false, true,Settings.WaitFree==VALUE_ON);
      break;         

    case CMD_VARIABLE_SAVE:
      for(z=1;z<=USER_VARIABLES_MAX;z++)
        {
        if(z==EventToExecute->Par1 || EventToExecute->Par1==0)
          {
          x=0;
          w=0;                                                                  // Plaats waar variabele al bestaat in eventlist
          y=0;                                                                  // hoogste beschikbare plaats
          while(Eventlist_Read(++x,&TempEvent,&TempEvent2))                     // Zoek of variabele al bestaat in eventlist
            {
            if(TempEvent.Type==NODO_TYPE_EVENT && TempEvent.Command==EVENT_BOOT && TempEvent.Par1==Settings.Unit)
              if(TempEvent2.Type==NODO_TYPE_COMMAND && TempEvent2.Command==CMD_VARIABLE_SET && TempEvent2.Par1==z)
                w=x;
              
            if(TempEvent.Command==0)
              y=x;
            }            

          x=w>0?w:y;                                                            // Bestaande regel of laatste vrije plaats.
    
          TempEvent.Type      = NODO_TYPE_EVENT;
          TempEvent.Command   = EVENT_BOOT;
          TempEvent.Par1      = Settings.Unit;
          TempEvent.Par2      = 0;
          TempEvent2.Type     = NODO_TYPE_COMMAND;
          TempEvent2.Command  = CMD_VARIABLE_SET;
          TempEvent2.Par1     = z;
          TempEvent2.Par2     = float2ul(UserVar[z-1]);
         
          Eventlist_Write(x, &TempEvent, &TempEvent2);                          // Schrijf weg in eventlist
          }
        }

      break;

    case CMD_LOCK:
      if(EventToExecute->Par1==VALUE_ON)
        {// Als verzoek om inschakelen dan Lock waarde vullen
        if(Settings.Lock==0)// mits niet al gelocked.
          Settings.Lock=EventToExecute->Par2;
        else
          error=MESSAGE_ACCESS_DENIED;
        }
      else
        {// Verzoek om uitschakelen
        if(Settings.Lock==EventToExecute->Par2 || Settings.Lock==0)// als lock code overeen komt of nog niet gevuld
          Settings.Lock=0;
        else
          error=MESSAGE_ACCESS_DENIED;
        }              
      Save_Settings();
      break;


    #if NODO_MEGA
    #if CLOCK
    case CMD_ALARM_SET:
      if(EventToExecute->Par1>=1 && EventToExecute->Par1<=ALARM_MAX)              // niet buiten bereik array!
        {
        for(x=0;x<8;x++)                                                          // loop de acht nibbles van de 32-bit Par2 langs
          {          
          y=(EventToExecute->Par2>>(x*4))&0xF;                                    // selecter nibble
          if(y!=0xE)                                                              // als de waarde geset moet worden
            {
            a=0xffffffff  ^ (0xfUL <<(x*4));                                      // Mask maken om de nibble positie y te wissen.
            Settings.Alarm[EventToExecute->Par1-1]  &=  a;                        // Maak nibble leeg
            Settings.Alarm[EventToExecute->Par1-1]  |= ((unsigned long)y)<<(x*4); // vul met door user opgegeven token
            }
          }
        }
      break;

    case CMD_CLOCK_SYNC:
      if(bitRead(HW_Config,HW_CLOCK)) // bitRead(HW_Config,HW_CLOCK)=true want dan is er een RTC aanwezig.
        {   
        // haal de tijd op van de Webserver. Dit vind plaats in de funktie: boolean SendHTTPRequest(char* Request)
        if(bitRead(HW_Config,HW_WEBAPP))
          {
          ClockSyncHTTP=true;
          EventToExecute->Port=VALUE_SOURCE_HTTP;
          SendEvent(EventToExecute, false, true, Settings.WaitFree==VALUE_ON);
          ClockSyncHTTP=false;
          }
  
        // Geef de juiste tijd nu door aan alle andere Nodo's

        // Verzend datum
        TempEvent.Port                  = VALUE_ALL;
        TempEvent.Type                  = NODO_TYPE_COMMAND;
        TempEvent.DestinationUnit       = 0;    
        TempEvent.Flags                 = TRANSMISSION_QUEUE | TRANSMISSION_QUEUE_NEXT; 
        TempEvent.Command=CMD_CLOCK_DATE;
        TempEvent.Par2= ((unsigned long)Time.Year  %10)      | ((unsigned long)Time.Year  /10)%10<<4  | ((unsigned long)Time.Year/100)%10<<8 | ((unsigned long)Time.Year/1000)%10<<12 | 
                        ((unsigned long)Time.Month %10) <<16 | ((unsigned long)Time.Month /10)%10<<20 | 
                        ((unsigned long)Time.Date  %10) <<24 | ((unsigned long)Time.Date  /10)%10<<28 ;

        SendEvent(&TempEvent, false, true, Settings.WaitFree==VALUE_ON);
              
        // Verzend tijd
        ClearEvent(&TempEvent);    
        TempEvent.Port                  = VALUE_ALL;
        TempEvent.Type                  = NODO_TYPE_COMMAND;
        TempEvent.DestinationUnit       = 0;    
        TempEvent.Flags                 = TRANSMISSION_QUEUE; 
        TempEvent.Command=CMD_CLOCK_TIME;
        TempEvent.Par2=Time.Minutes%10 | Time.Minutes/10<<4 | Time.Hour%10<<8 | Time.Hour/10<<12;

        SendEvent(&TempEvent, false, true, Settings.WaitFree==VALUE_ON);        
        }
      break;
    #endif CLOCK 
    #endif
          
    #if CLOCK
    case CMD_CLOCK_TIME:
      Time.Hour    =((EventToExecute->Par2>>12)&0xf)*10 + ((EventToExecute->Par2>>8)&0xf);
      Time.Minutes =((EventToExecute->Par2>>4 )&0xf)*10 + ((EventToExecute->Par2   )&0xf);
      Time.Seconds=0;
      ClockSet();
      break;

    case CMD_CLOCK_DATE:
      Time.Date    =((EventToExecute->Par2>>28 )&0xf)*10   + ((EventToExecute->Par2>>24 )&0xf);
      Time.Month   =((EventToExecute->Par2>>20 )&0xf)*10   + ((EventToExecute->Par2>>16 )&0xf);
      Time.Year    =((EventToExecute->Par2>>12 )&0xf)*1000 + ((EventToExecute->Par2>>8 )&0xf)*100 + ((EventToExecute->Par2>>4)&0xf)*10 + ((EventToExecute->Par2)&0xf);
      ClockSet();
      break;

    #endif CLOCK 

    case CMD_TIMER_SET:
      if(EventToExecute->Par2==0)
        UserTimer[EventToExecute->Par1-1]=0L;
      else
        UserTimer[EventToExecute->Par1-1]=millis()+EventToExecute->Par2*1000L;
      break;

    case CMD_TIMER_SET_VARIABLE:
       UserTimer[EventToExecute->Par1-1]=millis()+(unsigned long)(UserVar[EventToExecute->Par2-1])*1000L;
      break;

    case CMD_TIMER_RANDOM:
      UserTimer[EventToExecute->Par1-1]=millis()+random(EventToExecute->Par2)*1000;
      break;

    case CMD_SLEEP:
      #if !NODO_MEGA
      #if SLEEP
      GoodNightSleepTight();
      #endif
      #endif      
      break;

    case CMD_DELAY:
      Wait(EventToExecute->Par1, false, 0, false);
      break;        

    case CMD_SEND_EVENT:
      TempEvent=LastReceived;
      TempEvent.Port=EventToExecute->Par1==0?VALUE_ALL:EventToExecute->Par1;
      SendEvent(&TempEvent, TempEvent.Command==EVENT_RAWSIGNAL,true, Settings.WaitFree==VALUE_ON);
      break;        

    case CMD_SOUND: 
      Alarm(EventToExecute->Par1,EventToExecute->Par2);
      break;     
  
    #if WIRED
    case CMD_WIRED_PULLUP:
      Settings.WiredInputPullUp[EventToExecute->Par1-1]=EventToExecute->Par2; // EventToExecute->Par1 is de poort[1..]
      
      if(EventToExecute->Par2==VALUE_ON)
        pinMode(A0+PIN_WIRED_IN_1+EventToExecute->Par1-1,INPUT_PULLUP);
      else
        pinMode(A0+PIN_WIRED_IN_1+EventToExecute->Par1-1,INPUT);
      break;
                 
    case CMD_WIRED_OUT:
      digitalWrite(PIN_WIRED_OUT_1+EventToExecute->Par1-1,(EventToExecute->Par2==VALUE_ON));
      WiredOutputStatus[EventToExecute->Par1-1]=(EventToExecute->Par2==VALUE_ON);
      bitWrite(HW_Config,HW_WIRED_OUT,true);
      #if NODO_MEGA
      TempEvent.Par1=EventToExecute->Par1;
      TempEvent.Port=VALUE_SOURCE_SYSTEM;
      TempEvent.Direction=VALUE_DIRECTION_OUTPUT;
      PrintEvent(&TempEvent,VALUE_ALL);
      #endif
      break;

    case CMD_WIRED_SMITTTRIGGER:
      if(EventToExecute->Par1>0 && EventToExecute->Par1<=WIRED_PORTS)
        Settings.WiredInputSmittTrigger[EventToExecute->Par1-1]=EventToExecute->Par2;
      break;                  

    case CMD_WIRED_THRESHOLD:
      if(EventToExecute->Par1>0 && EventToExecute->Par1<=WIRED_PORTS)
        Settings.WiredInputThreshold[EventToExecute->Par1-1]=EventToExecute->Par2;
      break;                  

    #endif //WIRED
                         
    case CMD_SETTINGS_SAVE:
      UndoNewNodo();// Status NewNodo verwijderen indien van toepassing
      Save_Settings();
      break;

    case CMD_WAIT_FREE_RX: 
      Settings.WaitFree=EventToExecute->Par1;
      break;

    case CMD_OUTPUT:
      switch(EventToExecute->Par1)
        {
        case VALUE_SOURCE_IR:
          Settings.TransmitIR=EventToExecute->Par2;
          break;       
        case VALUE_SOURCE_RF:
          Settings.TransmitRF=EventToExecute->Par2;
          break;       

        #if NODO_MEGA
        case VALUE_SOURCE_HTTP:
          Settings.TransmitHTTP=EventToExecute->Par2;        
          break;       
        #endif
        }
      
      break;
      
    case CMD_STATUS:
      Status(EventToExecute);
      break;
            
    case CMD_UNIT_SET:
      if(EventToExecute->Par1>0 && EventToExecute->Par1<=UNIT_MAX)
        {
        Settings.Unit=EventToExecute->Par1;  
        Save_Settings();
        Reboot();
        break;
        }
      
    case CMD_REBOOT:
      Reboot();
      break;        
      
    case CMD_RESET:
      ResetFactory();
      break;

    case CMD_WAIT_FREE_NODO:
      Settings.WaitFreeNodo=EventToExecute->Par1;

      if(Settings.WaitFreeNodo!=VALUE_ON)
        {
        BusyNodo=0;
        TempEvent.Port                  = VALUE_ALL;
        TempEvent.Type                  = NODO_TYPE_COMMAND;
        TempEvent.Command               = CMD_WAIT_FREE_NODO;
        TempEvent.Par1                  = VALUE_OFF;
        SendEvent(&TempEvent, false, true,Settings.WaitFree==VALUE_ON);
        }

      break;                                                                      

    case CMD_EVENTLIST_ERASE:
      Led(BLUE);
      if(EventToExecute->Par1==0)
        {
        x=1;                                          
        UndoNewNodo();// Status NewNodo verwijderen indien van toepassing
        while(Eventlist_Write(x++,&TempEvent,&TempEvent));
        }
      else
        {
        Eventlist_Write(EventToExecute->Par1,&TempEvent,&TempEvent);
        }
      break;        
        
    case CMD_EVENTLIST_SHOW:
      // Er kunnen zich hier twee situaties voordoen: het verzoek is afkomstig van een Terminal (Serial/Telnet) of 
      // via IR/RF/I2C. Beide kennen een andere afhandeling immers de Terminal variant kan direct naar de MMI.
      // Bij de anderen moet er nog transport plaats vinden via IR, RF, I2C. De Terminal variant is NIET relevant
      // voor een Small omdat deze geen MMI heeft.

      #if NODO_MEGA
      if(EventToExecute->Port==VALUE_SOURCE_SERIAL || EventToExecute->Port==VALUE_SOURCE_TELNET)
        {      
        if(EventToExecute->Par1<=EventlistMax)
          {
          PrintString(ProgmemString(Text_22),EventToExecute->Port);
          if(EventToExecute->Par1==0)
            {
            x=1;
            while(EventlistEntry2str(x++,0,TempString,false))
              if(TempString[0]!=0)
                PrintString(TempString,EventToExecute->Port);
            }
          else
            {
            EventlistEntry2str(EventToExecute->Par1,0,TempString,false);
              if(TempString[0]!=0)
                PrintString(TempString,EventToExecute->Port);
            }
          PrintString(ProgmemString(Text_22),EventToExecute->Port);
          }
        else
          error=MESSAGE_INVALID_PARAMETER;
        }
      else // Transmissie via I2C/RF/IR: dan de inhoud van de Eventlist versturen.
        {

      #endif
      
        if(EventToExecute->Par1==0)
          {
          x=1;
          y=EventlistMax;
          }
        else
          {
          x=EventToExecute->Par1;
          y=EventToExecute->Par1;
          }
                
        // Haal de event en action op uit eeprom en verzend deze met extra transmissie vlaggen zodat de data:
        // 1. alleen wordt verstuurd naar de nodo die de data heeft opgevraagd.
        // 2. alleen wordt verzonden naar de poort waar het verzoek vandaan kwam
        // 3. aan de ontvangende zijde in de queue wordt geplaatst
        // 4. de vlag VIEW_SPECIAL mee krijgt zodat de events/commando's niet worden uitgevoerd aan de ontvangende zijde.
        // 5. Met LOCK alle andere Nodo's tijdelijk in de hold worden gezet.
        // In geval van verzending naar queue zal deze tijd niet van toepassing zijn omdat er dan geen verwerkingstijd nodig is.
        // Tussen de events die de queue in gaan een kortere delay tussen verzendingen.

        z=EventToExecute->Port;
        w=EventToExecute->SourceUnit;
        
        while(x<=y && Eventlist_Read(x,&TempEvent,&TempEvent2))
          {
          ClearEvent(EventToExecute);
          EventToExecute->Par1=x;
          EventToExecute->Command=SYSTEM_COMMAND_QUEUE_EVENTLIST_SHOW;
          EventToExecute->Flags=TRANSMISSION_QUEUE | TRANSMISSION_QUEUE_NEXT | TRANSMISSION_BUSY;
          EventToExecute->Type=NODO_TYPE_SYSTEM;
          EventToExecute->Port=z;
          EventToExecute->SourceUnit=Settings.Unit;
          EventToExecute->DestinationUnit=w;

          if(TempEvent.Command!=0)
            {
            SendEvent(EventToExecute,false,false,false);

            TempEvent.Flags=TRANSMISSION_VIEW_SPECIAL | TRANSMISSION_QUEUE | TRANSMISSION_QUEUE_NEXT | TRANSMISSION_BUSY;
            TempEvent.Port=z;
            TempEvent.DestinationUnit=w;
            SendEvent(&TempEvent,false,false,false);
    
    
            if(x==y)                                                            // Als laatste regel uit de eventlist, dan de ether weer vrijgeven. 
              TempEvent2.Flags=TRANSMISSION_VIEW_SPECIAL | TRANSMISSION_QUEUE ; 
            else
              TempEvent2.Flags=TRANSMISSION_VIEW_SPECIAL | TRANSMISSION_QUEUE | TRANSMISSION_QUEUE_NEXT | TRANSMISSION_BUSY;

            TempEvent2.Port=z;
            TempEvent2.DestinationUnit=w;
            SendEvent(&TempEvent2,false,false,false);
            }
          x++;
          }
        #if NODO_MEGA
        }        
        #endif
      break;

#if NODO_MEGA // vanaf hier commando's die alleen de Mega kent.

    case CMD_VARIABLE_LOG:
      {
      if(EventToExecute->Par1==0)
        {
        x=1;
        y=USER_VARIABLES_MAX;
        }
      else
        {
        x=EventToExecute->Par1;
        y=EventToExecute->Par1;
        }
      
      for(w=x;w<=y;w++)
        {
        sprintf(TempString,ProgmemString(Text_16),Time.Date,Time.Month,Time.Year,Time.Hour,Time.Minutes,w);
        dtostrf(UserVar[w-1], 0, 2,TempString+strlen(TempString));
        strcpy(TempString2,"VAR_");
        strcat(TempString2,int2str(w));
        FileWriteLine("",TempString2,"DAT",TempString, false);
        }        
      break;
      }

    case CMD_VARIABLE_GET: // VariableReceive <Variabelenummer_Bestemming>, <unit>, <Variabelenummer_Bron_Andere_Nodo>
      y=0; // retries
      error=MESSAGE_SENDTO_ERROR;
      do
        {
        // <VariabeleNummerBestemming> zit in Par1
        // <Unit> zit in bit 0..7 van Par2
        // <VariabeleNummerBron> zit in bit bit 15..8 van Par2
        //
        // Verzend naar de andere Nodo een verzoek om de variabele te verzenden.
        ClearEvent(&TempEvent);
        TempEvent.DestinationUnit=EventToExecute->Par2&0xff;
        TempEvent.Type=NODO_TYPE_COMMAND;
        TempEvent.Command=CMD_VARIABLE_SEND;
        TempEvent.Port=VALUE_ALL;
        TempEvent.Par1=(EventToExecute->Par2>>8)&0xff;                          // VariabeleBron
        TempEvent.Par2=NodoOnline(EventToExecute->Par2&0xff,0);                 // Poort waaronder de Slave Nodo bekend is.
        TempEvent.Flags=TRANSMISSION_QUEUE;

        if(TempEvent.Par2==0)                                                   // Als unitnummer niet bekend is
          TempEvent.Par2=VALUE_ALL;

        if(TempEvent.Par2==VALUE_SOURCE_SYSTEM)
          {
          error=MESSAGE_INVALID_PARAMETER;
          break;
          }

        SendEvent(&TempEvent,false,y==0,Settings.WaitFree==VALUE_ON);
        
        // Wacht tot event voorbij komt. De Wait(); funktie wacht op type, command en unit.
        ClearEvent(&TempEvent);
        TempEvent.SourceUnit          = EventToExecute->Par2&0xff;
        TempEvent.Command             = EVENT_VARIABLE;
        TempEvent.Type                = NODO_TYPE_EVENT;

        if(Wait(3,false,&TempEvent,false))
          {
          TempEvent.Par1            = EventToExecute->Par1;  
          TempEvent.Type            = NODO_TYPE_COMMAND;
            TempEvent.Command         = CMD_VARIABLE_SET;
          TempEvent.Direction       = VALUE_DIRECTION_INPUT;
          if(QueuePosition)QueuePosition--;                                   // binnengekomen event is eveneens op de queue geplaatst. deze mag weg.
          ProcessEvent(&TempEvent);                                           // verwerk binnengekomen event.
          error=0;
          }
        }while(error && ++y<3);
      break;        

    case CMD_PORT_INPUT:
      Settings.PortInput=EventToExecute->Par2;
      break;

    case CMD_PORT_OUTPUT:
      Settings.PortOutput=EventToExecute->Par2;
      break;

    case CMD_ECHO:
      if(EventToExecute->Port==VALUE_SOURCE_TELNET) 
        Settings.EchoTelnet=EventToExecute->Par1;
      if(EventToExecute->Port==VALUE_SOURCE_SERIAL) 
        Settings.EchoSerial=EventToExecute->Par1;        
      break;

    case CMD_ALIAS_SHOW:
      Settings.Alias=EventToExecute->Par1; 
      break;

    case CMD_DEBUG: 
      Settings.Debug=EventToExecute->Par1;
      break;

    case CMD_RAWSIGNAL_RECEIVE:
      Settings.RawSignalReceive=EventToExecute->Par1;
      if(EventToExecute->Par2==VALUE_ON || EventToExecute->Par2==VALUE_OFF)
        Settings.RawSignalChecksum=EventToExecute->Par2;
      break;

    case CMD_RAWSIGNAL_SAVE: // Geen Par1=Huidige inhoud opslaan. In andere gevallen de settine op VALUE_ON of VALUE_OFF instellen 
      if(EventToExecute->Par1==0)
        {
        // Sla huidige inhoud van de RawSignal buffer op.
        // Daarvoor moet eerst de bijbehorende HEX-code worden uitgerekend.
        RawSignal_2_32bit(&TempEvent);
        error=RawSignalWrite(TempEvent.Par2);
        }
      else
        {
        Settings.RawSignalSave=EventToExecute->Par1;
        Settings.RawSignalChecksum=EventToExecute->Par2&0xff;
        if(EventToExecute->Par1==VALUE_ON)
          Settings.RawSignalReceive=VALUE_ON;
        }        
      break;

    case CMD_RAWSIGNAL_SHOW: 
      error=FileShow(ProgmemString(Text_08),int2strhex(EventToExecute->Par2)+2,"DAT", VALUE_ALL);
      break;

    case CMD_RAWSIGNAL_SEND:
      if(RawSignal.Repeats==0)
        RawSignal.Repeats=RAWSIGNAL_TX_REPEATS;
      RawSignal.Delay=RAWSIGNAL_TX_DELAY;
      RawSignal.Source=EventToExecute->Par1;

      if(EventToExecute->Par2!=0)
        error=FileExecute(ProgmemString(Text_08),int2strhex(EventToExecute->Par2)+2,"DAT", false, 0);
      
      if(!error)
        {        
        TempEvent.Port=EventToExecute->Par1;
        TempEvent.Type=NODO_TYPE_RAWSIGNAL;
        TempEvent.Command=EVENT_RAWSIGNAL;
        TempEvent.Par1=EventToExecute->Par1;
        TempEvent.Par2=EventToExecute->Par2;
        SendEvent(&TempEvent, true ,true, Settings.WaitFree==VALUE_ON);
        }
      break;

    case CMD_LOG: 
      Settings.Log=EventToExecute->Par1;
      break;
      
    case CMD_FILE_EXECUTE:
      error=FileExecute("",int2str(EventToExecute->Par2),"DAT", EventToExecute->Par1==VALUE_ON, VALUE_ALL);
      break;      
              
    #endif    
    }

  #if NODO_MEGA
  free(TempString2);
  free(TempString);
  #endif

  return error;
  }


