void ProcessingStatus(boolean Processing)
  {
  static boolean PreviousProcessing=false;
  
  if(PreviousProcessing!=Processing)
    {
    PreviousProcessing=Processing;
    if(Processing)
      {
//      Serial.write(XOFF);
      Led(RED);
      }
    else
      {
      QueueProcess();                                                           // Verwerk eventuele events die in de queue zijn geplaatst.    

      if(RequestForConfirm)                                                     // Een event kan een verzoek bevatten om bevestiging. Doe dit dan pas na alle verwerking. 
        {  
        struct NodoEventStruct TempEvent;
        ClearEvent(&TempEvent);    
        TempEvent.Port                  = VALUE_ALL;
        TempEvent.Type                  = NODO_TYPE_SYSTEM;                     // Event is niet voor de gebruiker bedoeld
        TempEvent.Command               = SYSTEM_COMMAND_CONFIRMED;
        TempEvent.Par1                  = RequestForConfirm;
        SendEvent(&TempEvent, false,false,false);
        RequestForConfirm=false;
        }
//      Serial.write(XON);
      Led(GREEN);
      }  
    }
  }
    

byte ProcessEvent(struct NodoEventStruct *Event)
  {
  struct NodoEventStruct TempEvent;
  int x;
  byte error=0;
  boolean Continue=true;

  if(Event->Command==0)
    return error;

  #if NODO_MEGA
  if(FileWriteMode!=0)
    return 0;
  #endif

  ProcessingStatus(true);

  if(++ExecutionDepth>=MACRO_EXECUTION_DEPTH)
    {
    QueuePosition=0;
    Continue=false;
    error=MESSAGE_NESTING_ERROR;                                                // bij geneste loops ervoor zorgen dat er niet meer dan MACRO_EXECUTION_DEPTH niveaus diep macro's uitgevoerd worden
    }

  PluginCall(PLUGIN_EVENT_IN, Event,0);                                         // loop de plugins langs voor eventuele afhandeling van dit event.

  // Als er een LOCK actief is, dan commando's blokkeren behalve...
  if(Settings.Lock && (Event->Port==VALUE_SOURCE_RF || Event->Port==VALUE_SOURCE_IR ))
    {
    Continue=false;
    if(Event->Type==NODO_TYPE_COMMAND)
      {
      if(Event->Command==CMD_LOCK                   )Continue=true;
      }
    else if(Event->Type==NODO_TYPE_EVENT)
      {
      if     (Event->Command==EVENT_VARIABLE         )Continue=true;
      else if(Event->Command==EVENT_MESSAGE          )Continue=true;
      else if(Event->Command==EVENT_BOOT             )Continue=true;
      else if(Event->Command==EVENT_USEREVENT        )Continue=true;
      }
    else if(Event->Type==NODO_TYPE_SYSTEM)
      {
      if(Event->Command==SYSTEM_COMMAND_QUEUE_SENDTO )Continue=true;
      }
    if(!Continue)
      RaiseMessage(MESSAGE_ACCESS_DENIED,0);
    }
  
  if(Event->Type == NODO_TYPE_SYSTEM)
    {                       
    switch(Event->Command)
      {
      case SYSTEM_COMMAND_QUEUE_SENDTO:
        QueueReceive(Event);
        break;

      #if NODO_MEGA
      case SYSTEM_COMMAND_QUEUE_EVENTLIST_SHOW:                                 // Normaal deel van een SendTo en dus al in de Wait afgevangen
        Wait(5, false,0 , false);  
        break;
      #endif
      }
    Continue=false;
    }

  // Events die nu nog de TRANSMISSION_SENDTO vlag hebben staan hoorden oorspronkelijk tot een reeks die verzonden is
  // met een SendTo. Deze events hebben dier geen betekenis en dus volledig negeren.
  if(Continue && (Event->Flags & TRANSMISSION_SENDTO))
    Continue=false;


  if(Continue && (Event->Flags & TRANSMISSION_QUEUE))
    {
    QueueAdd(Event);
    if(Event->Flags & TRANSMISSION_QUEUE_NEXT)
      Wait(5, true,0,false);  
    Continue=false;
    }


  #if NODO_MEGA  
  // Alleen weergeven zonder event af te handelen
  if(Continue && (Event->Flags & TRANSMISSION_VIEW_SPECIAL))
    {
    Continue=false;
    }
  #endif
  
  #if NODO_MEGA
  if(Continue && bitRead(HW_Config,HW_SDCARD))
    if(Event->Command==EVENT_RAWSIGNAL && Settings.RawSignalSave==VALUE_ON)
      if(!RawSignalExist(Event->Par2))
        RawSignalWrite(Event->Par2);
  #endif    
  
  if(Continue && error==0)
    {
    #if NODO_MEGA
    PrintEvent(Event, VALUE_ALL);
    
    // Enkele commando's moet de WebApp terug ontvangen omdat anders statussen niet correct zijn.
    // Dit is een tijdelijke oplossing.
    x=false;
    if(Settings.TransmitHTTP==VALUE_ON && Event->Type==NODO_TYPE_COMMAND)
      {
      switch(Event->Command)
        {
        case CMD_ALARM_SET:
        case CMD_WIRED_OUT:
          x=true;
        }
      }
      

    if(Settings.TransmitHTTP==VALUE_ALL || x)
      {
      TempEvent=*Event;
      TempEvent.Port=VALUE_SOURCE_HTTP;
      SendEvent(&TempEvent,false,true,false);
      }
    #endif


    
    // ############# Verwerk event ################  
    if(Event->Type==NODO_TYPE_COMMAND)
      {
      Led(RED);                                                                 // LED aan als er iets verwerkt wordt      
      error=ExecuteCommand(Event);
      }

    else if(Event->Type==NODO_TYPE_PLUGIN_COMMAND)
      {
      Led(RED);                                                                 // LED aan als er iets verwerkt wordt      
      PluginCall(PLUGIN_COMMAND,Event,0);
      }
      
    else
      {
      struct NodoEventStruct EventlistEvent, EventlistAction;                   // het is een ander soort event. Loop de gehele eventlist langs om te kijken of er een treffer is.   
      LastReceived=*Event;                                                      // sla event op voor later gebruik (o.a. SendEvent)

      x=0;
      while(Eventlist_Read(++x,&EventlistEvent,&EventlistAction) && error==0)   // Zolang er nog regels zijn in de eventlist...
        {      
        if(CheckEvent(Event,&EventlistEvent))                                   // Als er een match is tussen het binnengekomen event en de regel uit de eventlist.
          {        
          Led(RED);                                                             // LED aan als er iets verwerkt wordt      
          ExecutionLine=x;
          error=ProcessEvent(&EventlistAction);
          }
        }
      
      if(error==MESSAGE_BREAK)                                                  // abort is geen fatale error maar een break. Deze dus niet verder behandelen als een error.
        error=0;
      }      

    #if NODO_MEGA
    if(!Transmission_SendToAll)Transmission_SendToUnit=0;                       // Als de SendTo niet permanent is ingeschakeld, dan deze weer uitzetten
    #endif
    }
    
  ExecutionDepth--;

  return error;
  }

 /**********************************************************************************************\
 * Toetst of Event ergens als entry voorkomt in de Eventlist. Geeft False als de opgegeven code niet bestaat.
 * geeft positie terug bij een match.
 \*********************************************************************************************/
byte CheckEventlist(struct NodoEventStruct *Event)
  {
  struct NodoEventStruct MacroEvent,MacroAction;

  int x=1;
  while(Eventlist_Read(x++,&MacroEvent,&MacroAction))
    if(MacroEvent.Command)
      if(CheckEvent(Event,&MacroEvent))
        return x;                                                               // match gevonden

  return false;
  }


 /**********************************************************************************************\
 * Vergelijkt twee events op matching voor uitvoering
 \*********************************************************************************************/
boolean CheckEvent(struct NodoEventStruct *Event, struct NodoEventStruct *MacroEvent)
  {  
  // geen lege events verwerken
  if(MacroEvent->Command==0 || Event->Command==0)
    return false;  

  // PrintNodoEvent("\nDEBUG: CheckEvent(), Input",Event);
  // PrintNodoEvent("DEBUG: CheckEvent(), Eventlist",MacroEvent);
    
  // ### WILDCARD:      
  if(MacroEvent->Command == EVENT_WILDCARD)                                                                                 // is regel uit de eventlist een WildCard?
    if( MacroEvent->Par1==VALUE_ALL          ||   MacroEvent->Par1==Event->Port)                                            // Correspondeert de poort of mogen alle poorten?
      if((MacroEvent->Par2&0xff)==VALUE_ALL  ||  (MacroEvent->Par2&0xff)==Event->Command && (Event->Type==NODO_TYPE_EVENT || Event->Type==NODO_TYPE_RAWSIGNAL))   // Correspondeert het commando deel
        if(((MacroEvent->Par2>>8)&0xff)==0   || ((MacroEvent->Par2>>8)&0xff)==Event->SourceUnit)                       // Correspondeert het unitnummer of is deze niet opgegeven
          return true;          

  // ### USEREVENT: beschouw bij een UserEvent een 0 voor Par1 of Par2 als een wildcard.
  if(Event->Type==NODO_TYPE_EVENT)
    if(Event->Command==EVENT_USEREVENT && MacroEvent->Command==EVENT_USEREVENT)                             // Is het een UserCommand?
      if( (Event->Par1==MacroEvent->Par1 || MacroEvent->Par1==0 || Event->Par1==0)                          // Par1 deel een match?
       && (Event->Par2==MacroEvent->Par2 || MacroEvent->Par2==0 || Event->Par2==0))                         // Par2 deel een match?
         return true; 
    
  // Events die niet voor deze Nodo bestemd zijn worden an niet doorgelaten door EventScanning(), echter events die voor alle Nodo's bestemd zijn
  // horen NIET langs de eventlist te worden gehaald. Deze daarom niet verder behandelen TENZIJ het een UserEvent is of behandeling door Wildcard. 
  // Die werden hierboven al behandeld.
  if(Event->SourceUnit!=0  && Event->SourceUnit!=Settings.Unit)
    return false;

  // #### EXACT: als huidige event exact overeenkomt met het event in de regel uit de Eventlist, dan een match. 
  if(MacroEvent->Command == Event->Command &&
     MacroEvent->Type    == Event->Type    &&
     MacroEvent->Par1    == Event->Par1    &&
     MacroEvent->Par2    == Event->Par2    )
       return true; 

  // ### TIME:
  if(Event->Type==NODO_TYPE_EVENT && Event->Command==EVENT_TIME)                // het binnengekomen event is een clock event.
    {
    // Structuur technisch hoort onderstaande regel hier thuis, maar qua performance niet optimaal!
    unsigned long Cmp=MacroEvent->Par2;
    unsigned long Inp=Event->Par2;

    // In het event in de eventlist kunnen zich wildcards bevinden. Maskeer de posities met 0xF wildcard nibble 
    // doe dit kruislings omdat zowel het invoer event als het event in de eventlist wildcards kunnen bevatten.
    for(byte y=0;y<8;y++)// loop de acht nibbles van de 32-bit Par2 langs
      {          
      unsigned long Mask=0xffffffff  ^ (0xFUL <<(y*4));                         // Masker maken om de nibble positie y te wissen.
      if(((Inp>>(y*4))&0xf) == 0xf)                                             // als in nibble y een wildcard waarde 0xf staat
        {
        Cmp&=Mask;                                                              // Maak nibble leeg
        Cmp|=(0xFUL<<(y*4));                                                    // vul met wildcard waarde 0xf
        }                                                 
      if(((Cmp>>(y*4))&0xf) == 0xf)                                             // als in nibble y een wildcard waarde 0xf staat
        {
        Inp&=Mask;                                                              // Maak nibble leeg
        Inp|=(0xFUL<<(y*4));                                                    // vul met wildcard waarde 0xf
        }
      }
            
     if(Inp==Cmp)                                                               // Als ingestelde alarmtijd overeen komt met huidige tijd.
       return true;
     }
  return false;                                                                 // geen match gevonden
  }


//#######################################################################################################
//##################################### Processing: Queue ###############################################
//#######################################################################################################

// De queue is een tijdelijke wachtrij voor ontvangen of te verzenden events. 
// De queue bevat event gegevens behalve de transmissie gegevens. 
struct QueueStruct
  {
  byte Flags;
  byte Port;
  byte Unit;
  byte Type;
  byte Command;
  byte Par1;
  unsigned long Par2;
  }Queue[EVENT_QUEUE_MAX];


 /**********************************************************************************************\
 * Voeg event toe aan de queue.
 \*********************************************************************************************/
boolean QueueAdd(struct NodoEventStruct *Event)
  {
  if(QueuePosition<EVENT_QUEUE_MAX)
    {
    // PrintNodoEvent("DEBUG: QueueAdd()",Event);

    Event->Flags&=~TRANSMISSION_QUEUE;                                          //  Haal eventuele QUEUE vlag er af. Anders loopt de queue vol door recursiviteit;
    
    Queue[QueuePosition].Flags   = Event->Flags; 
    Queue[QueuePosition].Port    = Event->Port;
    Queue[QueuePosition].Unit    = Event->SourceUnit;
    Queue[QueuePosition].Type    = Event->Type;
    Queue[QueuePosition].Command = Event->Command;
    Queue[QueuePosition].Par1    = Event->Par1;
    Queue[QueuePosition].Par2    = Event->Par2;
    QueuePosition++;           

    // Een EventlistShow staat welliswaar in de queue, maar die kunnen we als het de informatie compleet is gelijk weergeven
    // en vervolgens weer uit de queue halen. Deze voorziening is er alleen voor de Mega omdag een Small geen MMI heeft.
    #if NODO_MEGA
    char *TempString=(char*)malloc(INPUT_COMMAND_SIZE);
    char *TempString2=(char*)malloc(INPUT_LINE_SIZE);
    
    if(Event->Flags & TRANSMISSION_VIEW)                                        // Als alleen weergeven, dan direct doen en niet in de queue plaatsen.
      {
      Event2str(Event, TempString);  
      if(Settings.Alias==VALUE_ON)
        Alias(TempString,false);
      PrintString(TempString,VALUE_ALL);
      QueuePosition--;
      }

    else if(QueuePosition>=3)
      {
      if(Queue[QueuePosition-3].Type==NODO_TYPE_SYSTEM && Queue[QueuePosition-3].Command==SYSTEM_COMMAND_QUEUE_EVENTLIST_SHOW)
        {
        struct NodoEventStruct E;
        struct NodoEventStruct A;
        ClearEvent(&E);
        ClearEvent(&A);
        
        strcpy(TempString2,int2str(Queue[QueuePosition-3].Par1)); 
        strcat(TempString2,": ");

        // geef het event weer
        E.Type    = Queue[QueuePosition-2].Type;
        E.Command = Queue[QueuePosition-2].Command;
        E.Par1    = Queue[QueuePosition-2].Par1;
        E.Par2    = Queue[QueuePosition-2].Par2;
        Event2str(&E, TempString);
        if(Settings.Alias==VALUE_ON)
          Alias(TempString,false);
        strcat(TempString2, TempString);

        // geef het action weer
        A.Type    = Queue[QueuePosition-1].Type;
        A.Command = Queue[QueuePosition-1].Command;
        A.Par1    = Queue[QueuePosition-1].Par1;
        A.Par2    = Queue[QueuePosition-1].Par2;
        strcat(TempString2,"; ");
        Event2str(&A, TempString);  
        if(Settings.Alias==VALUE_ON)
          Alias(TempString,false);
        strcat(TempString2,TempString);
        
        PrintString(TempString2,VALUE_ALL);

        QueuePosition=0;
        }
      }    
    free(TempString2);
    free(TempString);
    #endif
    return true;
    }
  return false;
  }


 /**********************************************************************************************\
 * Verwerk de inhoud van de queue
 * Geeft het aantal verwerkte events uit de queue terug.
 \*********************************************************************************************/
void QueueProcess(void)
  {
  byte x;
  
  // Initialiseer een Event en Transmissie
  struct NodoEventStruct Event;
  ClearEvent(&Event);
  struct NodoEventStruct E;
  struct NodoEventStruct A;

  // De queue moet worden verwerkt. Er kunnen zich twee mogelijkheden voor doen:
  // A: de queue bevat een willekeurige reeks events/commando's of,
  // B: de queue bevat een reeks events/commando's die een speciale behandeling nodig hebben zoals SendTo of EventlistShow. In dit 
  //   geval bevat het eerste element een systeemcommando met bijzondere gegevens die benodigd zijn voor verdere verwerking.

  if(QueuePosition)
    {
    if(Queue[0].Type==NODO_TYPE_SYSTEM)
      {
      // Als er in de queue een EventlistWrite commando zit, dan moeten de twee opvolgende posities uit de queue
      // worden weggeschreven naar de eventlist. De rest uit de queue negeren we omdat een EventlistWrite altijd 
      // in een nieuwe verzending zit. Op de eerste positie (=0) zit het systeem commando QUEUE_SEND, die moeten
      // we negeren. Op de tweede zit het systeemcommando EventlistWrite, daarna het Event en de Actie.    
  
      if(Queue[0].Command==SYSTEM_COMMAND_QUEUE_EVENTLIST_WRITE && QueuePosition==3) // cmd
        {
        E.Type=Queue[1].Type;
        E.Command=Queue[1].Command;
        E.Par1=Queue[1].Par1;
        E.Par2=Queue[1].Par2;
        A.Type=Queue[2].Type;
        A.Command=Queue[2].Command;
        A.Par1=Queue[2].Par1;
        A.Par2=Queue[2].Par2;
       
        if(Eventlist_Write(Queue[0].Par1, &E, &A))
          UndoNewNodo();

        QueuePosition=0;
        }
      }

    if(QueuePosition)
      {
      for(x=0;x<QueuePosition;x++)
        {      
        Event.SourceUnit=Queue[x].Unit;
        Event.Type=Queue[x].Type;
        Event.Command=Queue[x].Command;
        Event.Par1=Queue[x].Par1;
        Event.Par2=Queue[x].Par2;
        Event.Direction=VALUE_DIRECTION_INPUT;
        Event.Port=Queue[x].Port;
        Event.Flags=Queue[x].Flags;
        ProcessEvent(&Event);                                                   // verwerk binnengekomen event.
        }
      }
    }
  QueuePosition=0;                                                              // Aan het einde van verwerken van de Queue deze weer leeg maken.
  }

#if NODO_MEGA  
/*********************************************************************************************\
 * Deze routine verzendt de inhoud van de queue naar een andere Nodo.
 \*********************************************************************************************/
byte QueueSend(boolean fast)
  {
  byte x,Port,error=MESSAGE_SENDTO_ERROR, Retry=0;
  unsigned long ID=millis();
  struct NodoEventStruct Event;

  byte Org_WFN=Settings.WaitFreeNodo;                                           // Stel de oorspronkelijke WaitFreeNodo setting veilig
  Settings.WaitFreeNodo=VALUE_OFF;                                              // Schakel WaitFreeNodo uit omdat dit het SendTo mechanisme doorkruist.
    
  if(!fast)
    {
    x=bitRead(HW_Config,HW_I2C);                                                // Zet I2C tijdelijk aan
    bitWrite(HW_Config,HW_I2C,1);
    do
      {
      Port=NodoOnline(Transmission_SendToUnit,0);                               // Port waar SendTo naar toe moet halen we uit lijst met Nodo's onderhouden door NodoOnline();
      if(Port==0)                                                               // als de Nodo nog niet bekend is, dan pollen we naar deze Nodo.
        {
        ClearEvent(&Event);
        Event.Port                  = VALUE_ALL;
        Event.Type                  = NODO_TYPE_SYSTEM;
        Event.Command               = SYSTEM_COMMAND_QUEUE_POLL;
        Event.DestinationUnit       = Transmission_SendToUnit;
        Event.Flags                 = TRANSMISSION_CONFIRM;
  
        SendEvent(&Event, false, false, Settings.WaitFree==VALUE_ON);
  
        Wait(5, false,0 , false);  
        }
      }while(Port==0 && ++Retry<3);
    bitWrite(HW_Config,HW_I2C,Port==VALUE_SOURCE_I2C | x);
    }
  
  if(Port!=0)
    {
    struct QueueStruct SendQueue[EVENT_QUEUE_MAX];                              // We maken tijdelijk gebruik van een SendQueue zodat de reguliere queue zijn werk kan blijven doen.
    for(x=0;x<QueuePosition;x++)
      {
      SendQueue[x]=Queue[x];                                                    // Kopieer de inhoud van de queue.
      
      if(Queue[x].Type==NODO_TYPE_COMMAND)                                      // Er zijn enkele commando's die geen reacte teruggeven. Zet in deze gevallen de fast mode aan.
        {
        switch(Queue[x].Command)
          {
          case CMD_UNIT_SET:
          case CMD_RESET:
          case CMD_REBOOT:
            fast=true;
          }
        }
      }
  
  
    if(fast)
      {
      for(x=0;x<QueuePosition;x++)                                              // Verzend alle events uit de queue.
        {
        ClearEvent(&Event);
        Event.Flags               = TRANSMISSION_QUEUE;  
        Event.DestinationUnit     = Transmission_SendToUnit;
        Event.SourceUnit          = Settings.Unit;
        Event.Port                = Port;
        Event.Type                = Queue[x].Type;
        Event.Command             = Queue[x].Command;
        Event.Par1                = Queue[x].Par1;
        Event.Par2                = Queue[x].Par2;
          
        // Serial.print(F("DEBUG: QueueSend() => FAST: SendEvent() x="));Serial.println(x); 
        SendEvent(&Event,false,false,Settings.WaitFree==VALUE_ON);
        }
      QueuePosition=0;
      return 0;
      }
  
  
    byte SendQueuePosition=QueuePosition;
    QueuePosition=0;
    Retry=0;
    
    
    // Eerste fase: Zorg dat de inhoud van de queue correct aan komt op de slave. Activeer aan slave zijde de QueueReceive()
    // Verzend in deze activering tevens het aantal events dat vanuit de queue verzonden zal worden naar de Slave. Eveneens wordt
    // er een ID verzonden. Deze unieke waarde zorgt er voor dat bij een eventuele re-send de reeks niet nogmaals aankomt op de 
    // slave.
    do
      {
      // Serial.print(F("\n\nDEBUG: QueueSend() Te verzenden aantal events="));Serial.print(SendQueuePosition);Serial.print(F(", ID="));Serial.println(ID); 

      ClearEvent(&Event);
      Event.DestinationUnit     = Transmission_SendToUnit;
      Event.SourceUnit          = Settings.Unit;
      Event.Port                = Port;
      Event.Type                = NODO_TYPE_SYSTEM;      
      Event.Command             = SYSTEM_COMMAND_QUEUE_SENDTO;      
      Event.Par1                = SendQueuePosition;                            // Aantal te verzenden events in de queue. Wordt later gecheckt en teruggezonden in de confirm. +1 omdat DIT event ook in de queue komt.
      Event.Par2                = ID;
      // PrintNodoEvent("DEBUG: QueueSend() Verzend SYSTEM_COMMAND_QUEUE_SENDTO",&Event);
      SendEvent(&Event,false,false,Settings.WaitFree==VALUE_ON);                // Alleen de eerste vooraf laten gaan door een WaitFree (indien setting zo staat ingesteld);
  
      for(x=0;x<SendQueuePosition;x++)                                          // Verzend alle events uit de queue. Alleen de bestemmings Nodo zal deze events in de queue plaatsen
        {
        ClearEvent(&Event);
        Event.DestinationUnit     = Transmission_SendToUnit;
        Event.SourceUnit          = Settings.Unit;
        Event.Port                = Port;
        Event.Type                = SendQueue[x].Type;
        Event.Command             = SendQueue[x].Command;
        Event.Par1                = SendQueue[x].Par1;
        Event.Par2                = SendQueue[x].Par2;
  
        if(x==(SendQueuePosition-1))
          Event.Flags = TRANSMISSION_SENDTO | TRANSMISSION_QUEUE;               // Verzendvlaggen geven aan dat er nog events verzonden gaan worden en dat de ether tijdelijk gereserveerd is
        else
          Event.Flags = TRANSMISSION_SENDTO | TRANSMISSION_QUEUE | TRANSMISSION_QUEUE_NEXT;
  
        // PrintNodoEvent("DEBUG: QueueSend() Verzend event",&Event);
        SendEvent(&Event,false,false,false);
        }
      
      // De ontvangende Nodo verzendt als het goed is een bevestiging dat het is ontvangen en het aantal commando's. De slave haalt tevens de Lock
      // van de Nodo's af zodat de ether weer door alle Nodo's gebruikt kunnen worden.
      ClearEvent(&Event);
      Event.SourceUnit          = Transmission_SendToUnit;
      Event.Command             = SYSTEM_COMMAND_CONFIRMED;
      Event.Type                = NODO_TYPE_SYSTEM;
  
      // Serial.print(F("DEBUG: QueueSend() => Wait() wacht op SYSTEM_COMMAND_CONFIRMED. Verzonden events="));Serial.println(x);
      if(Wait(5,false,&Event,false))
        if(x==(Event.Par1))                                                     // Verzonden events gelijk aan ontvangen events? -1 omdat aan de Slave zijde het eerste element in de queue geen deel uit maakt van de SendTo events.
          error=0;
  
      // if(error){Serial.print(F("DEBUG: QueueSend() Verwacht aantal="));Serial.print(x);Serial.print(F(", Bevestigd aantal="));Serial.print(Event.Par1);PrintNodoEvent(", Ontvangen=",&Event);}
      }while((++Retry<10) && error);   
    }
  QueuePosition=0;
  Settings.WaitFreeNodo=Org_WFN;                                                // Herstel de oorspronkelijke WaitFreeNodo setting.  
  return error;
  }
#endif


void QueueReceive(NodoEventStruct *Event)
  {
  static unsigned long PreviousID=0L;
  struct NodoEventStruct TempEvent;
  byte x, Received;

  byte Org_WFN=Settings.WaitFreeNodo;                                           // Stel de oorspronkelijke WaitFreeNodo setting veilig
  Settings.WaitFreeNodo=VALUE_OFF;                                              // Schakel WaitFreeNodo uit omdat dit het SendTo mechanisme doorkruist.
  
  Transmission_NodoOnly=true;                                                   // Uitsluitend Nodo events. Andere signalen worden tijdelijk volledig genegeerd.
  Wait(5, false, 0, true);
  Transmission_NodoOnly=false;
  Received=QueuePosition; 
  
  if(PreviousID!=Event->Par2 && Received==Event->Par1)                          // Als aantal regels in de queue overeenkomt en queue is nog niet eerder binnen gekomen (ID)
    {
    for(x=0;x<QueuePosition;x++)                                                
      {
      Queue[x].Flags=0;                                                         // Haal de QUEUE vlag van de events af.
      Queue[x].Unit=0;
      }
    PreviousID=Event->Par2;
    QueueProcess();
    }

  ClearEvent(&TempEvent);
  TempEvent.DestinationUnit     = Event->SourceUnit;
  TempEvent.SourceUnit          = Settings.Unit;
  TempEvent.Port                = Event->Port;
  TempEvent.Type                = NODO_TYPE_SYSTEM; 
  TempEvent.Command             = SYSTEM_COMMAND_CONFIRMED;      
  TempEvent.Par1                = Received;
  SendEvent(&TempEvent,false, false, false);                                    // Stuur vervolgens de master ter bevestiging het aantal ontvangen events die zich nu in de queue bevinden.
    
  QueuePosition=0;                                                              // Omdat ProcessQueue de queue heeft leeggedraaid, kan de queue positie op 0 worden gezet.
  Settings.WaitFreeNodo=Org_WFN;                                                // Herstel de oorspronkelijke WaitFreeNodo setting.  
  }
  