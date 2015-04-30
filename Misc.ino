
void DetectHardwareReset(void)
  {
  unsigned long ResetTime=millis()+10000;
  boolean toggle=false;
  Led(0);
  do
    {
    toggle=!toggle;
    digitalWrite(PIN_LED_RGB_R,toggle);
    delay(100);
    if(ResetTime<millis())
      ResetFactory();
    }while(digitalRead(PIN_IR_RX_DATA)==toggle);
  }
  
  
//#######################################################################################################
//##################################### Misc: EEPROM / Eventlist  #######################################
//#######################################################################################################


/**********************************************************************************************\
 * Schrijft een event in de Eventlist. Deze Eventlist bevindt zich in het EEPROM geheugen.
 \*********************************************************************************************/
boolean Eventlist_Write(int Line, struct NodoEventStruct *Event, struct NodoEventStruct *Action)// LET OP: Gebruikers input. Eerste adres=1
  {
  struct EventlistStruct EEPROM_Block;
  struct NodoEventStruct dummy;
  int x,address;

  // als opgegeven adres=0, zoek dan de eerste vrije plaats.
  if(Line==0)
    {
    Line++;
    while(Eventlist_Read(Line,&dummy,&dummy) && dummy.Command!=0)Line++;
    }
  Line--;                                                                          // echte adressering begint vanaf nul. voor de user vanaf 1.  

  if(Line>EventlistMax)
    return false;

  address=Line * sizeof(struct EventlistStruct) + sizeof(struct SettingsStruct);     // Eerste deel van het EEPROM geheugen is voor de settings. Reserveer deze bytes. Deze niet te gebruiken voor de Eventlist!
  
  byte *B=(byte*)&EEPROM_Block;                                                       // B wijst naar de eerste byte van de struct

  // Nu wegschrijven.
  address=Line * sizeof(struct EventlistStruct) + sizeof(struct SettingsStruct);     // Eerste deel van het EEPROM geheugen is voor de settings. Reserveer deze bytes. Deze niet te gebruiken voor de Eventlist!
  EEPROM_Block.EventType=Event->Type;
  EEPROM_Block.EventCommand=Event->Command;
  EEPROM_Block.EventPar1=Event->Par1;
  EEPROM_Block.EventPar2=Event->Par2;
 
  EEPROM_Block.ActionType=Action->Type;
  EEPROM_Block.ActionCommand=Action->Command;
  EEPROM_Block.ActionPar1=Action->Par1;
  EEPROM_Block.ActionPar2=Action->Par2;

  for(x=0;x<sizeof(struct EventlistStruct);x++) // schrijf alle bytes van de struct
    {
    if(address<EEPROM_SIZE)
      EEPROM.write(address++, *(B+x));
    else
      return false;
    }
  return true;
  }

/**********************************************************************************************\
 * 
 * Revision 01, 09-12-2009, P.K.Tonkes@gmail.com
 \*********************************************************************************************/
boolean Eventlist_Read(int address, struct NodoEventStruct *Event, struct NodoEventStruct *Action)// LET OP: eerste adres=1
  {
  struct EventlistStruct EEPROM_Block;

  if(address>EventlistMax)
    return false;

  address--;// echte adressering begint vanaf nul. voor de user vanaf 1.
  address=address*sizeof(struct EventlistStruct) + sizeof(struct SettingsStruct);     // Eerste deel van het EEPROM geheugen is voor de settings. Reserveer deze bytes. Deze niet te gebruiken voor de Eventlist!
  byte *B=(byte*)&EEPROM_Block; // B wijst naar de eerste byte van de struct

  for(int x=0;x<sizeof(struct EventlistStruct);x++) // lees alle bytes van de struct
    {
    if(address<EEPROM_SIZE)
      *(B+x)=EEPROM.read(address++);
    else
      return false;
    }

  ClearEvent(Event);
  Event->Type=EEPROM_Block.EventType;
  Event->Command=EEPROM_Block.EventCommand;
  Event->Par1=EEPROM_Block.EventPar1;
  Event->Par2=EEPROM_Block.EventPar2;
  
  ClearEvent(Action);
  Action->Type=EEPROM_Block.ActionType;
  Action->Command=EEPROM_Block.ActionCommand;
  Action->Par1=EEPROM_Block.ActionPar1;
  Action->Par2=EEPROM_Block.ActionPar2;

  Action->Port=VALUE_SOURCE_EVENTLIST;
  Action->Direction=VALUE_DIRECTION_INPUT;
  return true;
  }

/*********************************************************************************************\
 * Deze funktie verstuurt een message. Aanroep van deze funktie in de code daar waar de foutmelding 
 * is opgetreden, zodat er geen foutcodes door de code heen getransporteerd hoeven te worden.
 * 
 \*********************************************************************************************/
void RaiseMessage(byte MessageCode, unsigned long Option)
  {
  if(MessageCode)
    {
    struct NodoEventStruct TempEvent;
    ClearEvent(&TempEvent);
    TempEvent.Type      = NODO_TYPE_EVENT;
    TempEvent.Command   = EVENT_MESSAGE;
    TempEvent.Par1      = MessageCode;
    TempEvent.Par2      = Option;
    TempEvent.Direction = VALUE_DIRECTION_INPUT;
    TempEvent.Port      = VALUE_SOURCE_SYSTEM;

    #if NODO_MEGA
    PrintEvent(&TempEvent, VALUE_ALL);
    #endif
  
    switch(MessageCode)                                                         // sommige meldingen mogen niet worden verzonden als event
      {
      case MESSAGE_BREAK:                                                       // normale break
      case MESSAGE_VERSION_ERROR:                                               // gaat rondzingen van events tussen Nodo's opleveren.
      case MESSAGE_EXECUTION_STOPPED:                                           // Slechts een boodschap voor gebruiker. Onnodig om te versturen.
        return;
      }
  
    TempEvent.Port      = VALUE_ALL;
    SendEvent(&TempEvent,false,true,Settings.WaitFree==VALUE_ON);
    }
  }


/**********************************************************************************************\
 * Geef een geluidssignaal met toonhoogte van 'frequentie' in Herz en tijdsduur 'duration' in milliseconden.
 * LET OP: toonhoogte is slechts een grove indicatie. Deze routine is bedoeld als signaalfunctie
 * en is niet bruikbaar voor toepassingen waar de toonhoogte zuiver/exact moet zijn. Geen PWM.
 * Definieer de constante:
 * #define PIN_SPEAKER <LuidsprekerAansluiting>
 * Neem in setup() de volgende regel op:
 * pinMode(PIN_SPEAKER, OUTPUT);
 * Routine wordt verlaten na beeindiging van de pieptoon.
 * Revision 01, 13-02-2009, P.K.Tonkes@gmail.com
 \*********************************************************************************************/

void Beep(int frequency, int duration)//Herz,millisec 
  {
  long halfperiod=500000L/frequency;
  long loops=(long)duration*frequency/(long)1000;

  for(loops;loops>0;loops--) 
    {
    digitalWrite(PIN_SPEAKER, HIGH);
    delayMicroseconds(halfperiod);
    digitalWrite(PIN_SPEAKER, LOW);
    delayMicroseconds(halfperiod);
    }
  }

/**********************************************************************************************\
 * Geeft een belsignaal.
 * Revision 01, 09-03-2009, P.K.Tonkes@gmail.com
 \*********************************************************************************************/
void Alarm(int Variant,int Option)
  {
  byte x,y;

  switch (Variant)
  { 
  case 1:// four beeps
    for(y=1;y<=(Option>1?Option:1);y++)
      {
      Beep(3000,30);
      delay(100);
      Beep(3000,30);
      delay(100);
      Beep(3000,30);
      delay(100);
      Beep(3000,30);
      delay(1000);
      }    
    break;

  case 2: // whoop up
    for(y=1;y<=(Option>1?Option:1);y++)
      {
      for(x=1;x<=50;x++)
        Beep(250*x/4,20);
      }          
    break;

  case 3: // whoop down
    for(y=1;y<=(Option>1?Option:1);y++)
      {
      for(x=50;x>0;x--)
        Beep(250*x/4,20);
      }          
    break;

  case 4:// Settings.O.Settings.
    for(y=1;y<=(Option>1?Option:1);y++)
      {
      Beep(1200,50);
      delay(100);
      Beep(1200,50);
      delay(100);
      Beep(1200,50);
      delay(200);
      Beep(1200,300);
      delay(100);
      Beep(1200,300);
      delay(100);
      Beep(1200,300);
      delay(200);
      Beep(1200,50);
      delay(100);
      Beep(1200,50);
      delay(100);
      Beep(1200,50);
      if(Option>1)delay(500);
      }
    break;

  case 5:// ding-dong
    for(x=1;x<=(Option>1?Option:1);x++)
      {
      if(x>1)delay(2000);
      Beep(1500,500);
      Beep(1200,500);
      }    
    break;

  case 6: // phone ring
    for(x=1;x<(15*(Option>1?Option:1));x++)
      {
      Beep(1000,40);
      Beep(750,40);
      }
    break;

  case 7: // boot
    Beep(1500,100);
    Beep(1000,100);
    break;

  default:// beep
    if(Variant==0)
      Variant=5; // tijdsduur

    if(Option==0)
      Option=20; // toonhoogte

    Beep(100*Option,Variant*10);
    break;
    }
  }


/**********************************************************************************************\
 * Stuur de RGB-led.
 *
 * Voor de Nodo geldt:
 *
 * Groen = Nodo in rust en wacht op een event.
 * Rood = Nodo verwerkt event of commando.
 * Blauw = Bijzondere modus Nodo waarin Nodo niet in staat is om events te ontvangen of genereren.
 \*********************************************************************************************/
void Led(byte Color)
  { 
  #if NODO_MEGA
  digitalWrite(PIN_LED_RGB_R,Color==RED);
  digitalWrite(PIN_LED_RGB_B,Color==BLUE);
  digitalWrite(PIN_LED_RGB_G,Color==GREEN);
  #else
  digitalWrite(PIN_LED_RGB_R,(Color==RED || Color==BLUE));
  #endif
  }


/*********************************************************************************************\
 * Wachtloop. Als <EventsInQueue>=true dan worden voorbijkomende events in de queue geplaatst
 *
 * wachtloop die wordt afgebroken als:
 * - <Timeout> seconden zijn voorbij. In dit geval geeft deze funktie een <false> terug. (Timeout verlengd tijd als geldige events binnenkomen!)
 * - Het opgegeven event <WaitForEvent> voorbij is gekomen
 * - De ether weer is vrijgegeven voor Nodo communicatie (WaitForFreeTransmission=true)
 * - Er is een event opgevangen waar de TRANSMISSION_QUEUE_NEXT vlag NIET staat.
 \*********************************************************************************************/
boolean Wait(int Timeout, boolean WaitForFreeTransmission, struct NodoEventStruct *WaitForEvent, boolean EndSequence)
  {
  unsigned long TimeoutTimer=millis() + (unsigned long)(Timeout)*1000;

  #if NODO_MEGA
  unsigned long MessageTimer=millis() + 3000;
  boolean WaitMessage=false;
  #endif

  // Serial.println(F("DEBUG: Wait()"));
  
  struct NodoEventStruct Event;
  ClearEvent(&Event);

  while(TimeoutTimer>millis())
    {
    #if NODO_MEGA
    if(!WaitMessage && MessageTimer<millis())                                   // Als wachten langer duurt dan 5sec. dan melding weergeven aan gebruiker.
      {
      WaitMessage=true;
      PrintString(ProgmemString(Text_07),VALUE_ALL);
      }
    #endif
      
    if(ScanEvent(&Event))
      {            
      // PrintNodoEvent("DEBUG: Wait() Binnengekomen event",&Event);
      
      TimeoutTimer=millis() + (unsigned long)(Timeout)*1000;                    // Zolang er event binnenkomen geen timeout.
      #if NODO_MEGA
      MessageTimer=millis() + 3000;
      #endif
      QueueAdd(&Event);                                                         // Events die voorbij komen in de queue plaatsen.

      if(EndSequence && (Event.Flags & TRANSMISSION_QUEUE_NEXT)==0)
        {
        // Serial.println(F("DEBUG: Wait() TRANSMISSION_QUEUE_NEXT niet gedetecteerd."));
        break;
        }
        
      // als het gewacht wordt totdat de communicatie poorten weer beschikbaar zijn, dan wachtloop verlaten.        
      if(WaitForFreeTransmission && BusyNodo==0)
        {
        // Serial.println(F("DEBUG: Wait() BusyNodo=0"));
        break;
        }
      
      // break af als opgegeven event voorbij komt. Let op, alleen events met als bestemming 0 of dit unitnummer worden gedetecteerd!
      // De check vindt alleen plaats Type, Command en Unit, dus niet op Par1 en Par2.
      // Als SourceUnit==0 dan wordt input van alle units geaccepteerd.
      if(WaitForEvent!=0)
        {
        if(WaitForEvent->Command==Event.Command && WaitForEvent->Type==Event.Type)
          {
          if(WaitForEvent->SourceUnit==Event.SourceUnit || WaitForEvent->SourceUnit==0)
            {
            *WaitForEvent=Event;
            // Serial.println(F("DEBUG: Wait() WaitForEvent geslaagd."));
            break;
            }
          }
        }
      }
    Led(RED);
    }   
    
  // Serial.println(F("DEBUG: Wait() verlaten."));

  HoldTransmission=NODO_TX_TO_RX_SWITCH_TIME+millis();                          // Hier ingelast om snel afwisselend events uitwisselen tussen twee i2c Nodo's te laten werken.         

  if(TimeoutTimer<=millis())                                                    // als timeout, dan error terug geven
    {
    // Serial.println(F("DEBUG: Wait() Timeout."));
    return false;
    }

  else
    return true;
  }



/*********************************************************************************************\
 * Haal voor het opgegeven Command de status op door resultaat in de event struct te plaatsen.
 * Geef een false terug als commando niet bestaat.
 \*********************************************************************************************/
boolean GetStatus(struct NodoEventStruct *Event)
  { 
  int x;

  // bewaar de opvraag gegevens en begin met een schoon event
  byte xPar1=Event->Par1;
  byte xCommand=Event->Command;  
  ClearEvent(Event);

  Event->Command=xCommand;
  Event->Type=NODO_TYPE_EVENT;

  switch (xCommand)
    {
    case VALUE_EVENTLIST_COUNT:
      x=0;
      struct NodoEventStruct dummy;
      while(Eventlist_Read(++x,&dummy,&dummy))
        {
        if(dummy.Command)
          Event->Par1++;
        }
      Event->Par2=x-Event->Par1-1;
      break;

  case CMD_WAIT_FREE_RX: 
    Event->Par1=Settings.WaitFree;
    break;

  case VALUE_BUILD:
    Event->Par2=NODO_BUILD;      
    break;        

  case VALUE_HWCONFIG: 
    Event->Par2=HW_Config;      
    break;        

  #if CLOCK 
  case EVENT_CLOCK_DAYLIGHT:
    Event->Par1=Time.Daylight;
    break;
  #endif CLOCK

  case CMD_OUTPUT:
    Event->Par1=xPar1;
    switch(xPar1)
      {
      case VALUE_SOURCE_IR:
        Event->Par2=Settings.TransmitIR;
        break;
  
      case VALUE_SOURCE_RF:
        Event->Par2=Settings.TransmitRF;
        break;
  
      #if NODO_MEGA
      case VALUE_SOURCE_HTTP:
        Event->Par2=Settings.TransmitHTTP;
        break;
       #endif 
      default:
        Event->Command=0;                                                       // Geen geldige optie. Als 0 wordt teruggegeven in command dan wordt niets weergegeven met de status.
      }
    break;

  case VALUE_SOURCE_PLUGIN:
    x=Plugin_id[xPar1];
    if(x)
      Event->Par1=x;
    else
        Event->Command=0;                                                       // Geen device op deze positie
    break;

  case CMD_VARIABLE_SET:
    Event->Par1=xPar1;
    Event->Par2=float2ul(UserVar[xPar1-1]);
    break;

  #if CLOCK
  case CMD_CLOCK_DATE:
    Event->Par2= ((unsigned long)Time.Year  %10)      | ((unsigned long)Time.Year  /10)%10<<4  | ((unsigned long)Time.Year/100)%10<<8 | ((unsigned long)Time.Year/1000)%10<<12 | 
                 ((unsigned long)Time.Month %10) <<16 | ((unsigned long)Time.Month /10)%10<<20 | 
                 ((unsigned long)Time.Date  %10) <<24 | ((unsigned long)Time.Date  /10)%10<<28 ;                
    break;

  case CMD_CLOCK_TIME:
    Event->Par2=Time.Minutes%10 | Time.Minutes/10<<4 | Time.Hour%10<<8 | Time.Hour/10<<12;
    break;
  #endif CLOCK

  case CMD_TIMER_SET:
    Event->Par1=xPar1;
    if(UserTimer[xPar1-1])
      Event->Par2=(UserTimer[xPar1-1]-millis())/1000;
    else
      Event->Par2=0;
    break;

  #if WIRED
  case CMD_WIRED_PULLUP:
    Event->Par1=xPar1;
    Event->Par2=Settings.WiredInputPullUp[xPar1-1];
    break;

  case VALUE_WIRED_ANALOG:
    Event->Par1=xPar1;
    Event->Par2=analogRead(PIN_WIRED_IN_1+xPar1-1);
    break;

  case CMD_WIRED_THRESHOLD:
    Event->Par1=xPar1;
    Event->Par2=Settings.WiredInputThreshold[xPar1-1];
    break;

  case CMD_WIRED_SMITTTRIGGER:
    Event->Par1=xPar1;
    Event->Par2=Settings.WiredInputSmittTrigger[xPar1-1];
    break;
    
  case EVENT_WIRED_IN:
    Event->Par1=xPar1;
    Event->Par2=(WiredInputStatus[xPar1-1])?VALUE_ON:VALUE_OFF;
    break;

  case CMD_WIRED_OUT:
    Event->Par1=xPar1;
    Event->Par2=(WiredOutputStatus[xPar1-1])?VALUE_ON:VALUE_OFF;
    break;

  #endif //WIRED

  case VALUE_FREEMEM:    
    Event->Par2=FreeMem();
    break;


  case CMD_LOCK:
    Event->Par1=Settings.Lock?VALUE_ON:VALUE_OFF;;
    break;

#if NODO_MEGA
  case CMD_RAWSIGNAL_SAMPLE:
    Event->Par1=Settings.RawSignalSample;
    break;

  case CMD_RAWSIGNAL_RECEIVE:
    Event->Par1=Settings.RawSignalReceive;
    Event->Par2=Settings.RawSignalChecksum;
    break;

  case CMD_DEBUG:
    Event->Par1=Settings.Debug;
    break;

  case CMD_ALIAS_SHOW:
    Event->Par1=Settings.Alias;
    break;
    
  case VALUE_UNIT:
    x=NodoOnline(xPar1,0);
    if(x!=0)
      {
      Event->Par1=xPar1;
      Event->Par2=x;
      }
    else
      Event->Command=0;                                                         // Als resultaat niet geldig is en niet weergegeven mag worden
    
    break;

  case CMD_ALARM_SET:
    Event->Par1=xPar1;
    Event->Par2=Settings.Alarm[xPar1-1];
    break;

  case CMD_PORT_INPUT:
    Event->Par2=Settings.PortInput;
    break;

  case CMD_PORT_OUTPUT:
    Event->Par2=Settings.PortOutput;
    break;

  case CMD_LOG:
    Event->Par1=Settings.Log;
    break;

  case CMD_WAIT_FREE_NODO:
    Event->Par1=Settings.WaitFreeNodo;
    break;

  case CMD_RAWSIGNAL_SAVE:
    Event->Par1=Settings.RawSignalSave;
    Event->Par2=Settings.RawSignalCleanUp;
    break;

  // pro-forma de commando's die geen fout op mogen leveren omdat deze elders in de statusafhandeling worden weergegeven
  case CMD_NODO_IP:
  case CMD_GATEWAY:
  case CMD_SUBNET:
  case CMD_DNS_SERVER:
  case CMD_HTTP_REQUEST:
  case CMD_ID:
  case CMD_TEMP:
    break;
  #endif      
  default:
    return false;
  }
  return true;
}



/*********************************************************************************************\
 * Sla alle settings op in het EEPROM geheugen.
 \*********************************************************************************************/
void Save_Settings(void)  
  {
  char ByteToSave,*pointerToByteToSave=pointerToByteToSave=(char*)&Settings;    //pointer verwijst nu naar startadres van de struct. 

  for(int x=0; x<sizeof(struct SettingsStruct) ;x++)
    {
    EEPROM.write(x,*pointerToByteToSave); 
    pointerToByteToSave++;
    }  
  }

/*********************************************************************************************\
 * Laad de settings uit het EEPROM geheugen.
 \*********************************************************************************************/
boolean LoadSettings()
 {
  byte x;

  char ByteToSave,*pointerToByteToRead=(char*)&Settings;    //pointer verwijst nu naar startadres van de struct.

  for(int x=0; x<sizeof(struct SettingsStruct);x++)
    {
    *pointerToByteToRead=EEPROM.read(x);
    pointerToByteToRead++;// volgende byte uit de struct
    }
  }


/*********************************************************************************************\
 * Alle settings van de Nodo weer op default.
 \*********************************************************************************************/
void ResetFactory(void)
  {
  int x,y;
  Beep(2000,2000);

  // maak de eventlist leeg.
  struct NodoEventStruct dummy;
  ClearEvent(&dummy);
  x=1;
  while(Eventlist_Write(x++,&dummy,&dummy));

  // Herstel alle settings naar defaults
  Settings.Version                    = NODO_VERSION_MINOR;
  Settings.NewNodo                    = true;
  Settings.Lock                       = 0;
  Settings.WaitFreeNodo               = VALUE_OFF;
  Settings.TransmitIR                 = VALUE_OFF;
  Settings.TransmitRF                 = VALUE_ON;
  Settings.Unit                       = UNIT_NODO;
  Settings.RawSignalReceive           = VALUE_OFF;
  Settings.RawSignalSample            = RAWSIGNAL_SAMPLE_DEFAULT;  

#if WAIT_FREE_RX
  Settings.WaitFree                   = VALUE_ON;
#else
  Settings.WaitFree                   = VALUE_OFF;
#endif

#if NODO_MEGA
  Settings.TransmitHTTP               = VALUE_OFF;
  Settings.Debug                      = VALUE_OFF;
  Settings.HTTPRequest[0]             = 0; // string van het HTTP adres leeg maken
  Settings.Client_IP[0]               = 0;
  Settings.Client_IP[1]               = 0;
  Settings.Client_IP[2]               = 0;
  Settings.Client_IP[3]               = 0;
  Settings.Nodo_IP[0]                 = 0;
  Settings.Nodo_IP[1]                 = 0;
  Settings.Nodo_IP[2]                 = 0;
  Settings.Nodo_IP[3]                 = 0;
  Settings.Gateway[0]                 = 0;
  Settings.Gateway[1]                 = 0;
  Settings.Gateway[2]                 = 0;
  Settings.Gateway[3]                 = 0;
  Settings.Subnet[0]                  = 255;
  Settings.Subnet[1]                  = 255;
  Settings.Subnet[2]                  = 255;
  Settings.Subnet[3]                  = 0;
  Settings.DnsServer[0]               = 0;
  Settings.DnsServer[1]               = 0;
  Settings.DnsServer[2]               = 0;
  Settings.DnsServer[3]               = 0;
  Settings.PortInput                  = 6636;
  Settings.PortOutput                 = 80;
  Settings.ID[0]                      = 0; // string leegmaken
  Settings.Temp[0]                    = 0; // string leegmaken
  Settings.EchoSerial                 = VALUE_OFF;  //VALUE_ON;  not for RFLink
  Settings.EchoTelnet                 = VALUE_OFF;  
  Settings.Log                        = VALUE_OFF;  
  Settings.RawSignalSave              = VALUE_OFF;  
  Settings.RawSignalCleanUp           = 0;  
  Settings.RawSignalChecksum          = VALUE_ON;  
  Settings.Alias                      = VALUE_OFF;  
  Settings.Password[0]                = 0;

  // Maak de alarmen leeg
  for(x=0;x<ALARM_MAX;x++)
    Settings.Alarm[x]=0L;

#endif

  // zet analoge poort  waarden op default
  #if WIRED
  for(x=0;x<WIRED_PORTS;x++)
    {
    Settings.WiredInputThreshold[x]=512; 
    Settings.WiredInputSmittTrigger[x]=10;
    Settings.WiredInputPullUp[x]=VALUE_ON;
    }
  #endif //WIRED

  Save_Settings();
  Reboot();
  }


/**********************************************************************************************\
 * Geeft de status weer of genereert een event.
 * Par1 = Command
 \**********************************************************************************************/
void Status(struct NodoEventStruct *Request)
  {
  byte CMD_Start,CMD_End;
  byte Par1_Start,Par1_End;
  byte Port=0,x;
  boolean s;
  boolean DisplayLocal=false;
  struct NodoEventStruct Result;

  #if NODO_MEGA          
  char *TempString=(char*)malloc(INPUT_LINE_SIZE);
  boolean dhcp=(Settings.Nodo_IP[0] + Settings.Nodo_IP[1] + Settings.Nodo_IP[2] + Settings.Nodo_IP[3])==0;
  #endif

  // bepaal waar het resultaat naar toe moet.
  switch(Request->Port)
    {
    // bronnen waar het resultaat aan teruggestuurd kan worden:
    case VALUE_SOURCE_IR:
    case VALUE_SOURCE_RF:
    case VALUE_SOURCE_I2C:
      Port=Request->Port;
      break;

    #if NODO_MEGA
    // bronnen waar de output geprint moet worden als tekst
    case VALUE_SOURCE_SERIAL:
    case VALUE_SOURCE_TELNET:
    case VALUE_SOURCE_HTTP:
      Port=Request->Port;
      DisplayLocal=true;
      break;
    #endif
  
    // bronnen waar niet teruggestuurd kan worden en dus naar alle poorten:
    case VALUE_SOURCE_SYSTEM:
    case VALUE_SOURCE_WIRED:
    case VALUE_SOURCE_FILE:
    case VALUE_SOURCE_EVENTLIST:              
      Port=VALUE_ALL;
      break;      
    }

  // Als de status informatie verzonden moet worden, dan komt deze terecht in de queue van de master waar het status verzoek
  // vandaan kwam. Hier een korte wachttijd omdat anders i.v.m. de omschakeltijd van de Master van zenden naar ontvangen
  // de eerste waarden die door deze slave worden verzonden niet aankomen.
  if(!DisplayLocal)
    delay(DELAY_BETWEEN_TRANSMISSIONS);
  
  Result.Command=Request->Par1;
  
  if(Request->Par2==VALUE_ALL)
    Request->Par2==0;

//  #if NODO_MEGA          
//  if(DisplayLocal && (Request->Par1==0 || Request->Par1==VALUE_ALL))
//    PrintWelcome();
//  #endif

  if(Request->Par1==VALUE_ALL)
    {
    Request->Par2=0;
    CMD_Start=0;
    CMD_End=COMMAND_MAX;
    }
  else
    {
    Result.Command=Request->Par1;
    if(!GetStatus(&Result))                                                     // kijk of voor de opgegeven parameter de status opvraagbaar is. Zo niet dan klaar.
      return;
    CMD_Start=Request->Par1;
    CMD_End=Request->Par1;
    }

  for(x=CMD_Start; x<=CMD_End; x++)
    {
    s=false;
    if(DisplayLocal)
      {
      s=true;
      switch (x)
        {
        #if NODO_MEGA          
        #ifdef ethernetserver_h
        case CMD_CLIENT_IP:
          sprintf(TempString,ProgmemString(Text_18),cmd2str(CMD_CLIENT_IP),Settings.Client_IP[0],Settings.Client_IP[1],Settings.Client_IP[2],Settings.Client_IP[3]);
          PrintString(TempString, Port);
          break;

        case CMD_NODO_IP:
          sprintf(TempString,ProgmemString(Text_18),cmd2str(CMD_NODO_IP), EthernetNodo.localIP()[0],EthernetNodo.localIP()[1],EthernetNodo.localIP()[2],EthernetNodo.localIP()[3]);
          if(dhcp)
            strcat(TempString,"(DHCP)");
          PrintString(TempString, Port);
          break;
  
        case CMD_GATEWAY:
          // Gateway
          if(!dhcp)
            {
            sprintf(TempString,ProgmemString(Text_18),cmd2str(CMD_GATEWAY),Settings.Gateway[0],Settings.Gateway[1],Settings.Gateway[2],Settings.Gateway[3]);
            PrintString(TempString, Port);
            }
          break;
  
        case CMD_SUBNET:
          // Subnetmask
          if(!dhcp)
            {
            sprintf(TempString,ProgmemString(Text_18),cmd2str(CMD_SUBNET),Settings.Subnet[0],Settings.Subnet[1],Settings.Subnet[2],Settings.Subnet[3]);
            PrintString(TempString, Port);
            }
          break;
  
        case CMD_DNS_SERVER:
          if(!dhcp)
            {
            // DnsServer
            sprintf(TempString,ProgmemString(Text_18),cmd2str(CMD_DNS_SERVER),Settings.DnsServer[0],Settings.DnsServer[1],Settings.DnsServer[2],Settings.DnsServer[3]);
            PrintString(TempString, Port);
            }
          break;
  
        case CMD_PORT_INPUT:
          sprintf(TempString,"%s %d",cmd2str(CMD_PORT_INPUT), Settings.PortInput);
          PrintString(TempString, Port);
          break;
  
        case CMD_PORT_OUTPUT:
          sprintf(TempString,"%s %d",cmd2str(CMD_PORT_OUTPUT), Settings.PortOutput);
          PrintString(TempString, Port);
          break;
  
        case CMD_HTTP_REQUEST:
          sprintf(TempString,"%s %s",cmd2str(CMD_HTTP_REQUEST),Settings.HTTPRequest);
          PrintString(TempString, Port);
          break;
  
        #endif // ethernetserver_h

        case CMD_ID:
          sprintf(TempString,"%s %s",cmd2str(CMD_ID), Settings.ID);
          PrintString(TempString, Port);
          break;
  
        case CMD_TEMP:
          sprintf(TempString,"%s %s",cmd2str(CMD_TEMP), Settings.Temp);
          PrintString(TempString, Port);
          break;
  
  #endif
  
        default:
          s=false; 
          break;
        }
      }

    Result.Command=x;
    Result.Par1=Request->Par1;    
    
    if(!s && GetStatus(&Result))                                                // Als het een geldige uitvraag is.
      {
      if(Request->Par2==0)                                                      // Als in het commando 'Status Par1, Par2' Par2 niet is gevuld met een waarde
        {
        switch(x)
          {
          case VALUE_SOURCE_PLUGIN:
            Par1_Start=0;
            Par1_End=PLUGIN_MAX-1;
            break;

          case CMD_OUTPUT:
            Par1_Start=0;
            Par1_End=COMMAND_MAX;
            break;
  
          case VALUE_WIRED_ANALOG:
          case CMD_WIRED_OUT:
          case CMD_WIRED_PULLUP:
          case CMD_WIRED_SMITTTRIGGER:
          case CMD_WIRED_THRESHOLD:
          case EVENT_WIRED_IN:
            Par1_Start=1;
            Par1_End=WIRED_PORTS;
            break;      
  
          case CMD_VARIABLE_SET:
            Par1_Start=1;
            Par1_End=USER_VARIABLES_MAX;
            break;
    
          case CMD_TIMER_SET:
            Par1_Start=1;
            Par1_End=TIMER_MAX;
            break;
  
          #if NODO_MEGA
          case CMD_ALARM_SET:
            Par1_Start=1;
            Par1_End=ALARM_MAX;
            break;

          case VALUE_UNIT:
            Par1_Start=1;
            Par1_End=UNIT_MAX;
            break;
          #endif

          default:
            Par1_Start=0;
            Par1_End=0;
          }
        }
      else
        {
        Par1_Start=Request->Par2;
        Par1_End=Request->Par2;
        }

      for(byte y=Par1_Start;y<=Par1_End;y++)
        {
        Result.Command=x;
        Result.Par1=y;
        GetStatus(&Result); 
        Result.Port=Port;
        
        if(Result.Command!=0)
          {
          if(!DisplayLocal)
            {
            Result.Flags=TRANSMISSION_VIEW | TRANSMISSION_QUEUE | TRANSMISSION_QUEUE_NEXT | TRANSMISSION_BUSY;
            SendEvent(&Result,false,false,false);
            }            
  
          #if NODO_MEGA
          else
            {
            Event2str(&Result,TempString);
            PrintString(TempString, Request->Port); // geef weer op terminal
            }
          #endif
          }
        }
      }
    }

  #if NODO_MEGA
  if(DisplayLocal && Request->Par1==VALUE_ALL)
    PrintString(ProgmemString(Text_22), Request->Port);

  free(TempString);
  #endif
  }



#if NODO_MEGA
/**********************************************************************************************\
 * Deze functie haalt een tekst op uit PROGMEM en geeft als string terug
 \*********************************************************************************************/
char* ProgmemString(prog_char* text)
  {
  byte x=0;
  static char buffer[90]; 

  buffer[0]=0;
  do
    {
    buffer[x]=pgm_read_byte_near(text+x);
    }while(buffer[x++]!=0);
    
  return buffer;
  }

/*********************************************************************************************\
 * Deze routine parsed string en geeft het opgegeven argument nummer Argc terug in Argv
 * argumenten worden van elkaar gescheiden door een komma of een spatie.
 * Let op dat de ruimte in de doelstring voldoende is EN dat de bron string netjes is afgesloten 
 * met een 0-byte. Bij succes keert de routine terug met als waarde de positie naar het eerste teken 
 * van het volgende argument.  
 \*********************************************************************************************/
byte GetArgv(char *string, char *argv, int argc)
{
  int string_pos=0,argv_pos=0,argc_pos=0; 
  char c,d;

  while(string_pos<strlen(string))
  {
    c=string[string_pos];
    d=string[string_pos+1];

    if       (c==' ' && d==' '){}
    else if  (c==' ' && d==','){}
    else if  (c==',' && d==' '){}
    else if  (c==' ' && d>=33 && d<=126){}
    else if  (c==',' && d>=33 && d<=126){}
    else 
      {
      if(c!=' ' && c!=',')
        {
        argv[argv_pos++]=c;
        argv[argv_pos]=0;
        }          

      if(d==' ' || d==',' || d==0)
        {
        // Bezig met toevoegen van tekens aan een argument, maar er kwam een scheidingsteken.
        argv[argv_pos]=0;
        argc_pos++;

        if(argc_pos==argc)
          return string_pos+1;
          
        argv[0]=0;
        argv_pos=0;
        string_pos++;
      }
    }
    string_pos++;
  }
  return 0;
}


/*********************************************************************************************\
 * Deze routine parsed string en zoekt naar keyword. Geeft de startpositie terug waar het keyword
 * gevonden is. -1 indien niet gevonden. Niet casesensitive.
 \*********************************************************************************************/
int StringFind(char *string, char *keyword)
{
  int x,y;
  int keyword_len=strlen(keyword);
  int string_len=strlen(string);

  if(keyword_len>string_len) // doe geen moeite als het te vinden eyword langer is dan de string.
    return -1;

  for(x=0; x<=(string_len-keyword_len); x++)
  {
    y=0;
    while(y<keyword_len && (tolower(string[x+y])==tolower(keyword[y])))
      y++;

    if(y==keyword_len)
      return x;
  }
  return -1;
}


/**********************************************************************************************\
 * Geeft een string terug met een cookie op basis van een random HEX-waarde van 32-bit.
 \*********************************************************************************************/
void RandomCookie(char* Ck)
  {
  byte  x,y;

  for(x=0;x<8;x++)
    {
    y=random(0x0, 0xf);
    Ck[x]=y<10?(y+'0'):((y-10)+'A');
    }
  Ck[8]=0; // afsluiten string
  }




//#######################################################################################################
//######################################### Misc: MD5      ##############################################
//#######################################################################################################


uint32_t md5_T[] PROGMEM = {
  0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf,
  0x4787c62a, 0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af,
  0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e,
  0x49b40821, 0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
  0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8, 0x21e1cde6,
  0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,
  0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122,
  0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
  0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039,
  0xe6db99e5, 0x1fa27cf8, 0xc4ac5665, 0xf4292244, 0x432aff97,
  0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d,
  0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
  0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 };


#define MD5_HASH_BITS  128
#define MD5_HASH_BYTES (MD5_HASH_BITS/8)
#define MD5_BLOCK_BITS 512
#define MD5_BLOCK_BYTES (MD5_BLOCK_BITS/8)
#define MD5_HASH_BITS  128

struct md5_ctx_t 
{
  uint32_t a[4];
  int32_t counter;
};

// typedef uint8_t md5_hash_t[MD5_HASH_BYTES];

void md5_init(struct md5_ctx_t *s){
  s->counter = 0;
  s->a[0] = 0x67452301;
  s->a[1] = 0xefcdab89;
  s->a[2] = 0x98badcfe;
  s->a[3] = 0x10325476;
}

static
uint32_t md5_F(uint32_t x, uint32_t y, uint32_t z){
  return ((x&y)|((~x)&z));
}

static
uint32_t md5_G(uint32_t x, uint32_t y, uint32_t z){
  return ((x&z)|((~z)&y));
}

static
uint32_t md5_H(uint32_t x, uint32_t y, uint32_t z){
  return (x^y^z);
}

static
uint32_t md5_I(uint32_t x, uint32_t y, uint32_t z){
  return (y ^ (x | (~z)));
}

typedef uint32_t md5_func_t(uint32_t, uint32_t, uint32_t);

#define ROTL32(x,n) (((x)<<(n)) | ((x)>>(32-(n))))  

static void md5_core(uint32_t* a, void* block, uint8_t as, uint8_t s, uint8_t i, uint8_t fi){
  uint32_t t;
  md5_func_t* funcs[]={
    md5_F, md5_G, md5_H, md5_I      };
  as &= 0x3;
  /* a = b + ((a + F(b,c,d) + X[k] + T[i]) <<< s). */
  t = a[as] + funcs[fi](a[(as+1)&3], a[(as+2)&3], a[(as+3)&3])
    + *((uint32_t*)block) + pgm_read_dword(md5_T+i) ;
  a[as]=a[(as+1)&3] + ROTL32(t, s);
}

void md5_nextBlock(struct md5_ctx_t *state, const void* block){
  uint32_t	a[4];
  uint8_t		m,n,i=0;
  /* this requires other mixed sboxes */
  a[0]=state->a[0];
  a[1]=state->a[1];
  a[2]=state->a[2];
  a[3]=state->a[3];

  /* round 1 */
  uint8_t s1t[]={
    7,12,17,22      }; // 1,-1   1,4   2,-1   3,-2
  for(m=0;m<4;++m){
    for(n=0;n<4;++n){
      md5_core(a, &(((uint32_t*)block)[m*4+n]), 4-n, s1t[n],i++,0);
    }
  }
  /* round 2 */
  uint8_t s2t[]={
    5,9,14,20      }; // 1,-3   1,1   2,-2   2,4
  for(m=0;m<4;++m){
    for(n=0;n<4;++n){
      md5_core(a, &(((uint32_t*)block)[(1+m*4+n*5)&0xf]), 4-n, s2t[n],i++,1);
    }
  }
  /* round 3 */
  uint8_t s3t[]={
    4,11,16,23      }; // 0,4   1,3   2,0   3,-1
  for(m=0;m<4;++m){
    for(n=0;n<4;++n){
      md5_core(a, &(((uint32_t*)block)[(5-m*4+n*3)&0xf]), 4-n, s3t[n],i++,2);
    }
  }
  /* round 4 */
  uint8_t s4t[]={
    6,10,15,21      }; // 1,-2   1,2   2,-1   3,-3
  for(m=0;m<4;++m){
    for(n=0;n<4;++n){
      md5_core(a, &(((uint32_t*)block)[(0-m*4+n*7)&0xf]), 4-n, s4t[n],i++,3);
    }
  }
  state->a[0] += a[0];
  state->a[1] += a[1];
  state->a[2] += a[2];
  state->a[3] += a[3];
  state->counter++;
}

void md5_lastBlock(struct md5_ctx_t *state, const void* block, uint16_t length_b){
  uint16_t l;
  uint8_t b[64];
  while (length_b >= 512){
    md5_nextBlock(state, block);
    length_b -= 512;
    block = ((uint8_t*)block) + 512/8;
  }
  memset(b, 0, 64);
  memcpy(b, block, length_b/8);
  /* insert padding one */
  l=length_b/8;
  if(length_b%8){
    uint8_t t;
    t = ((uint8_t*)block)[l];
    t |= (0x80>>(length_b%8));
    b[l]=t;
  }
  else{
    b[l]=0x80;
  }
  /* insert length value */
  if(l+sizeof(uint64_t) >= 512/8){
    md5_nextBlock(state, b);
    state->counter--;
    memset(b, 0, 64-8);
  }
  *((uint64_t*)&b[64-sizeof(uint64_t)]) = (state->counter * 512) + length_b;
  md5_nextBlock(state, b);
}

void md5_ctx2hash(struct md5_hash_t* dest, const struct md5_ctx_t* state){
  memcpy(dest, state->a, MD5_HASH_BYTES);
}

void md5(char* dest)
{
  const void* src=dest;
  uint32_t length_b = strlen((char*)src) * 8;
  struct md5_ctx_t ctx;
  char *Str=(char*)malloc(20);

  md5_init(&ctx);
  while(length_b>=MD5_BLOCK_BITS)
  {
    md5_nextBlock(&ctx, src);
    src = (uint8_t*)src + MD5_BLOCK_BYTES;
    length_b -= MD5_BLOCK_BITS;
  }
  md5_lastBlock(&ctx, src, length_b);
  md5_ctx2hash((md5_hash_t*)&MD5HashCode, &ctx);

  strcpy(Str,PROGMEM2str(Text_05));              
  int y=0;
  for(int x=0; x<16; x++)
  {
    dest[y++]=Str[MD5HashCode[x]>>4  ];
    dest[y++]=Str[MD5HashCode[x]&0x0f];
  }
  dest[y]=0;

  free(Str);
}

#endif

uint8_t *heapptr, *stackptr;

unsigned long FreeMem(void)
  {
  stackptr = (uint8_t *)malloc(4);        // use stackptr temporarily
  heapptr = stackptr;                     // save value of heap pointer
  free(stackptr);                         // free up the memory again (sets stackptr to 0)
  stackptr =  (uint8_t *)(SP);            // save value of stack pointer
  return (stackptr-heapptr);
  }
    

void PulseCounterISR()
  {
  static unsigned long PulseTimePrevious=0L;

  // in deze interrupt service routine staat millis() stil. Dit is echter geen bezwaar voor de meting.
  PulseTime=millis()-PulseTimePrevious;
  if(PulseTime>=PULSE_DEBOUNCE_TIME)
    PulseCount++;
  else
    PulseTime=0;

  PulseTimePrevious=millis();
  }     


#if NODO_MEGA
//################### Calculate #################################

#define CALCULATE_OK                            0
#define CALCULATE_ERROR_STACK_OVERFLOW          1
#define CALCULATE_ERROR_BAD_OPERATOR            2
#define CALCULATE_ERROR_PARENTHESES_MISMATCHED  3
#define CALCULATE_ERROR_UNKNOWN_TOKEN           4
#define STACK_SIZE 50
#define TOKEN_MAX 20

float stack[STACK_SIZE];
float *sp = stack-1;
float *sp_max = &stack[STACK_SIZE-1];

#define is_operator(c)  (c == '+' || c == '-' || c == '*' || c == '/' )

int push(float value)
{
  if(sp != sp_max) // Full
  {
    *(++sp) = value;
    return 0;
  }
  else 
    return CALCULATE_ERROR_STACK_OVERFLOW;
}

float pop()
{
  if(sp != (stack-1)) // empty
    return *(sp--);
}

float apply_operator(char op, float first, float second)
  {
  switch(op)
  {
  case '+': 
    return first + second;
  case '-': 
    return first - second;
  case '*': 
    return first * second;
  case '/': 
    return first / second;
    return 0;
  }  
}

char *next_token(char *linep)
{
  while(isspace(*(linep++)));
  while(*linep && !isspace(*(linep++)));
  return linep;
}

int RPNCalculate(char* token)
{
  if(token[0]==0)
    return 0; // geen moeite doen voor een lege string

  if(is_operator(token[0]))
  {
    float second = pop();
    float first = pop();

    if(push(apply_operator(token[0], first, second)))
      return CALCULATE_ERROR_STACK_OVERFLOW;
  }
  else // Als er nog een is, dan deze ophalen
  if(push(atof(token))) // is het een waarde, dan op de stack plaatsen
      return CALCULATE_ERROR_STACK_OVERFLOW;

  return 0;
}

// operators
// precedence   operators         associativity
// 3            !                 right to left
// 2            * / %             left to right
// 1            + - ^             left to right
int op_preced(const char c)
{
  switch(c)    
  {
  case '*':  
  case '/': 
    return 2;
  case '+': 
  case '-': 
    return 1;
  }
  return 0;
}

bool op_left_assoc(const char c)
{
  switch(c)
  { 
  case '*': 
  case '/': 
  case '+': 
  case '-': 
    return true;     // left to right
    //case '!': return false;    // right to left
  }
  return false;
}

unsigned int op_arg_count(const char c)
{
  switch(c)  
  {
  case '*': 
  case '/': 
  case '+': 
  case '-': 
    return 2;
    //case '!': return 1;
  }
  return 0;
}


int Calculate(const char *input, float* result)
{
  const char *strpos = input, *strend = input + strlen(input);
  char token[25];
  char c, *TokenPos = token;
  char stack[32];       // operator stack
  unsigned int sl = 0;  // stack length
  char     sc;          // used for record stack element
  int error=0;

  *sp=0;

  while(strpos < strend)   
  {
    // read one token from the input stream
    c = *strpos;
    if(c != ' ')
    {
      // If the token is a number (identifier), then add it to the token queue.
      if((c >= '0' && c <= '9') || c=='.')
      {
        *TokenPos = c; 
        ++TokenPos;
      }

      // If the token is an operator, op1, then:
      else if(is_operator(c))
      {
        *(TokenPos)=0;
        error=RPNCalculate(token);
        TokenPos=token;
        if(error)return error;
        while(sl > 0)
        {
          sc = stack[sl - 1];
          // While there is an operator token, op2, at the top of the stack
          // op1 is left-associative and its precedence is less than or equal to that of op2,
          // or op1 has precedence less than that of op2,
          // The differing operator priority decides pop / push
          // If 2 operators have equal priority then associativity decides.
          if(is_operator(sc) && ((op_left_assoc(c) && (op_preced(c) <= op_preced(sc))) || (op_preced(c) < op_preced(sc))))
          {
            // Pop op2 off the stack, onto the token queue;
            *TokenPos = sc; 
            ++TokenPos;
            *(TokenPos)=0;
            error=RPNCalculate(token);
            TokenPos=token; 
            if(error)return error;
            sl--;
          }
          else
            break;
        }
        // push op1 onto the stack.
        stack[sl] = c;
        ++sl;
      }
      // If the token is a left parenthesis, then push it onto the stack.
      else if(c == '(')
      {
        stack[sl] = c;
        ++sl;
      }
      // If the token is a right parenthesis:
      else if(c == ')')
      {
        bool pe = false;
        // Until the token at the top of the stack is a left parenthesis,
        // pop operators off the stack onto the token queue
        while(sl > 0)
        {
          *(TokenPos)=0;
          error=RPNCalculate(token);
          TokenPos=token; 
          if(error)return error;
          sc = stack[sl - 1];
          if(sc == '(')
          {
            pe = true;
            break;
          }
          else  
          {
            *TokenPos = sc; 
            ++TokenPos;
            sl--;
          }
        }
        // If the stack runs out without finding a left parenthesis, then there are mismatched parentheses.
        if(!pe)  
          return CALCULATE_ERROR_PARENTHESES_MISMATCHED;

        // Pop the left parenthesis from the stack, but not onto the token queue.
        sl--;

        // If the token at the top of the stack is a function token, pop it onto the token queue.
        if(sl > 0)
          sc = stack[sl - 1];

      }
      else
        return CALCULATE_ERROR_UNKNOWN_TOKEN;
    }
    ++strpos;
  }
  // When there are no more tokens to read:
  // While there are still operator tokens in the stack:
  while(sl > 0)
  {
    sc = stack[sl - 1];
    if(sc == '(' || sc == ')')
      return CALCULATE_ERROR_PARENTHESES_MISMATCHED;

    *(TokenPos)=0;
    error=RPNCalculate(token);
    TokenPos=token; 
    if(error)return error;
    *TokenPos = sc; 
    ++TokenPos;
    --sl;
  }

  *(TokenPos)=0;
  error=RPNCalculate(token);
  TokenPos=token; 
  if(error)
  {
    *result=0;
    return error;
  }  
  *result=*sp;
  return CALCULATE_OK;
}
//################### Einde Calculate #################################

boolean Substitute(char* Input)
{
  boolean Grab=false;
  byte Res;
  byte x;

  // Serial.print(F("DEBUG: Substitute() Input="));Serial.println(Input); 

  char *Output=(char*)malloc(INPUT_LINE_SIZE);
  char *TmpStr=(char*)malloc(INPUT_LINE_SIZE);
  char *TmpStr2=(char*)malloc(INPUT_COMMAND_SIZE);
  char* InputPos  = Input;
  char* OutputPos = Output;
  char* TmpStrPos = TmpStr;
  boolean error=false;

  Res=0;
  while(*(InputPos)!=0)// zolang einde van de string Input nog niet bereikt
   {
    if(*InputPos=='%') 
    {
      if(!Grab)
      {
        Grab=true;
        TmpStrPos=TmpStr;
      }
      else
        {
        Grab=false;
        *TmpStrPos=0;// Sluit string af

        // Haal de status van de variabele
        byte Cmd=0;
        GetArgv(TmpStr,TmpStr2,1);
        Cmd=str2cmd(TmpStr2); // commando deel
        TmpStr2[0]=0;

        if(Cmd!=0)
          {
          // Er zijn twee type mogelijk: A)Direct te vullen omdat ze niet met status opvraagbaar zijn, B)Op te vragen met status
          byte Par1=0;
          byte Par2=0;
          Res=2;

          // A)Direct te vullen omdat ze niet met status opvraagbaar zijn
          switch(Cmd)
          {
          case CMD_ID:
            strcpy(TmpStr2,Settings.ID);
            break;    

          case CMD_TEMP:
            strcpy(TmpStr2,Settings.Temp);
            break;    

          case VALUE_RECEIVED_EVENT:
            Event2str(&LastReceived,TmpStr);
            GetArgv(TmpStr,TmpStr2,1);
            break;    

          case VALUE_RECEIVED_PAR1:
            Event2str(&LastReceived,TmpStr);
            GetArgv(TmpStr,TmpStr2,2);
            break;    

          case VALUE_RECEIVED_PAR2:
            Event2str(&LastReceived,TmpStr);
            GetArgv(TmpStr,TmpStr2,3);
            break;    

          default:
            {
            // B) Op te vragen met status  
            if(GetArgv(TmpStr,TmpStr2,2))
              {
              Par1=str2cmd(TmpStr2);
              if(!Par1)
                Par1=str2int(TmpStr2);
              }

            struct NodoEventStruct Temp;
            ClearEvent(&Temp);
            Temp.Par1=Par1;
            Temp.Par2=Par2;
            Temp.Command=Cmd;
            if(GetStatus(&Temp))
              {
              Event2str(&Temp,TmpStr);
              if(!GetArgv(TmpStr,TmpStr2,3)) // Als de waarde niet in de 3e parameter zat...
                GetArgv(TmpStr,TmpStr2,2);   // dan moet hij in de tweede zitten.
              }
            }
          }
          // plak de opgehaalde waarde aan de output string
          for(x=0;x<strlen(TmpStr2);x++)
            *OutputPos++=TmpStr2[x];
        }
      }     
    }
    else if(Grab)
      *TmpStrPos++=*InputPos;// Voeg teken toe aan variabele      
    else
      *OutputPos++=*InputPos;// Voeg teken toe aan outputstring

    InputPos++;  
  }
  *OutputPos=0;// Sluit string af.

  // Serial.print(F("DEBUG: Substitute() Substituted="));Serial.println(Input); 


  if(TmpStr[0]!=0)
  {
    strcpy(Input,Output);  
    if(Settings.Debug==VALUE_ON)
    {
      //Serial.print(F("Substituted: "));
      //Serial.println(Input);
    }
  }

  if(Grab) // Als % niet correct afgesloten...
    error=true;
  else
  {
    // Nu zijn de formules aan de beurt.
    InputPos  = Input;
    OutputPos = Output;
    TmpStrPos = TmpStr;
    Res=0;

    while(*(InputPos)!=0)// zolang einde van de string Input nog niet bereikt
    {
      if(*InputPos=='#') 
      {
        if(!Grab)
        {
          Grab=true;
          TmpStrPos=TmpStr;
        }
        else
        {
          Grab=false;
          *TmpStrPos=0;// Sluit string af
          float result;
          if(Calculate(TmpStr,&result)==CALCULATE_OK)
          {
            floatToString(TmpStr,result,2,0);
            x=StringFind(TmpStr,".00");          // de overbodige nullen weghalen
            if(x>0)
              TmpStr[x]=0;

            // plak de opgehaalde waarde aan de output string
            for(x=0;x<strlen(TmpStr);x++)
              *OutputPos++=TmpStr[x];

            Res=true;
          }
          else
            Res=false;

        }     
      }
      else if(Grab)
        *TmpStrPos++=*InputPos;// Voeg teken toe aan variabele      
      else
        *OutputPos++=*InputPos;// Voeg teken toe aan outputstring
      InputPos++;  
    }
    *OutputPos=0;// Sluit string af.

    if(Grab) // Als % niet correct afgesloten...
      error=true;

    if(Res && !error)
    {
      strcpy(Input,Output);  
      if(Settings.Debug==VALUE_ON)
      {
        //Serial.print(F("Calculated: "));
        //Serial.println(Input);
      }
      strcpy(Input,Output);  
    }
  }
  free(TmpStr2);
  free(TmpStr);
  free(Output);

  // Serial.print(F("DEBUG: Substitute() Calculated="));Serial.println(Input); 
  return error;
}
#endif

/**********************************************************************************************\
 * Indien het een vers geresette Nodo is, dan ongedaan maken van deze status.
 \*********************************************************************************************/
void UndoNewNodo(void)
  {
  if(Settings.NewNodo)
    {
    Settings.NewNodo=false;
    Save_Settings();
    } 
  }

/**********************************************************************************************\
 * Reset een vers geïnitialiseerde struct. Nodog om dat niet mag worden aangenomen dat alle
 * waarden Na initialisatie leeg zijn.
 \*********************************************************************************************/
void ClearEvent(struct NodoEventStruct *Event)
{    
  Event->Command            = 0;
  Event->Par1               = 0;
  Event->Par2               = 0L;
  Event->Flags              = 0;
  Event->Port               = 0;
  Event->Type               = 0;
  Event->Direction          = 0;
  Event->DestinationUnit    = 0;
  Event->SourceUnit         = Settings.Unit;
  Event->Version            = NODO_VERSION_MINOR;
  Event->Checksum           = 0;
}


//#######################################################################################################
//##################################### Clock            ################################################
//#######################################################################################################
#if CLOCK

#define DS1307_SEC       0
#define DS1307_MIN       1
#define DS1307_HR        2
#define DS1307_DOW       3
#define DS1307_DATE      4
#define DS1307_MTH       5
#define DS1307_YR        6
#define DS1307_CTRL      7
#define DS1307_DLS       8 // De DS1307 heeft nog 56 bytes over voor data. De eerste positie 0x08 gebruiken we als vlag voor zomertijd/wintertijd (DLS).
#define DS1307_DLS_M     9 // Maand laatste DLS omschakeling.
#define DS1307_DLS_D    10 // Datum laatste DLS omschakeling.
#define DS1307_RESERVED 11 // Reserve
uint8_t rtc[12];

#define DS1307_BASE_YR   2000
#define DS1307_CTRL_ID   B1101000  //De RTC zit op adres 104. Dit is een vast gegeven van de DS1307 chip.
#define DS1307_CLOCKHALT B10000000
#define DS1307_LO_BCD    B00001111
#define DS1307_HI_BCD    B11110000
#define DS1307_HI_SEC    B01110000
#define DS1307_HI_MIN    B01110000
#define DS1307_HI_HR     B00110000
#define DS1307_LO_DOW    B00000111
#define DS1307_HI_DATE   B00110000
#define DS1307_HI_MTH    B00110000
#define DS1307_HI_YR     B11110000

// update the data on the RTC from the bcd formatted data in the buffer
void DS1307_save(void)
  {
  WireNodo.beginTransmission(DS1307_CTRL_ID);
  WireNodo.write((uint8_t)0x00); // reset register pointer
  for(byte i=0; i<11; i++)WireNodo.write(rtc[i]);
  WireNodo.endTransmission();
  }

// Aquire data from the RTC chip in BCD format, refresh the buffer

void DS1307_read(void)
  {
  WireNodo.beginTransmission(DS1307_CTRL_ID);  // reset the register pointer to zero
  WireNodo.write((uint8_t)0x00);
  if (WireNodo.endTransmission(false) == 0) // Try to become I2C Master, send data and collect bytes, keep master status for next request...
    {
      WireNodo.requestFrom(DS1307_CTRL_ID, 11);  // request the 9 bytes of data    (secs, min, hr, dow, date. mth, yr. ctrl, dls)
      if(WireNodo.available() == 11) for(byte i=0; i<11; i++)rtc[i]=WireNodo.read();// store data in raw bcd format
    }
  WireNodo.endTransmission(true); // Release I2C Master status...
  }


/**********************************************************************************************\
 * Zet de RTC op tijd.
 \*********************************************************************************************/
void ClockSet(void) 
  {
  // bereken uit de datum de dag van de week.
  const int t[] = {0,3,2,5,0,3,5,1,4,6,2,4};
  int y = Time.Year;
  y-=Time.Month<3;
  Time.Day=(y + y/4 - y/100 + y/400 + t[Time.Month-1] + Time.Date) % 7 +1;

  rtc[DS1307_SEC]      =DS1307_CLOCKHALT;                                       // Stop the clock. Set the ClockHalt bit high to stop the rtc. This bit is part of the seconds byte
  DS1307_save();
  
  rtc[DS1307_MIN]      = ((Time.Minutes/10)<<4)+(Time.Minutes%10);
  rtc[DS1307_HR]       = ((Time.Hour/10)<<4)+(Time.Hour%10);                    // schrijf de wintertijd weg.
  rtc[DS1307_DOW]      = Time.Day;
  rtc[DS1307_DATE]     = ((Time.Date/10)<<4)+(Time.Date%10);
  rtc[DS1307_MTH]      = ((Time.Month/10)<<4)+(Time.Month%10);
  rtc[DS1307_YR]       = (((Time.Year-DS1307_BASE_YR)/10)<<4)+(Time.Year%10); 
  rtc[DS1307_SEC]      = ((Time.Seconds/10)<<4)+(Time.Seconds%10);              // and start the clock again...
  rtc[DS1307_DLS]      = Time.DaylightSaving;
  rtc[DS1307_DLS_M]    = Time.DaylightSavingSetMonth;                           // Maand waarin de laatste zomertijd/wintertijd omschakeling plaats gevonden heeft
  rtc[DS1307_DLS_D]    = Time.DaylightSavingSetDate;                            // Datum waarop de laatste zomertijd/wintertijd omschakeling plaats gevonden heeft
  rtc[DS1307_RESERVED] = 0;
  DS1307_save();
}

/**********************************************************************************************\
 * Leest de realtime clock en plaatst actuele waarden in de struct Time. 
 * Eveneens wordt de Event code terug gegeven
 \*********************************************************************************************/
void ClockRead(void)
  {
  DS1307_read();// lees de RTC chip uit
  if (rtc[4] <= 0) return;

  Time.Seconds                = (10*((rtc[DS1307_SEC] & DS1307_HI_SEC)>>4))+(rtc[DS1307_SEC] & DS1307_LO_BCD);
  Time.Minutes                = (10*((rtc[DS1307_MIN] & DS1307_HI_MIN)>>4))+(rtc[DS1307_MIN] & DS1307_LO_BCD);
  Time.Date                   = (10*((rtc[DS1307_DATE] & DS1307_HI_DATE)>>4))+(rtc[DS1307_DATE] & DS1307_LO_BCD);
  Time.Month                  = (10*((rtc[DS1307_MTH] & DS1307_HI_MTH)>>4))+(rtc[DS1307_MTH] & DS1307_LO_BCD);
  Time.Year                   = (10*((rtc[DS1307_YR] & DS1307_HI_YR)>>4))+(rtc[DS1307_YR] & DS1307_LO_BCD)+DS1307_BASE_YR;
  Time.Hour                   = (10*((rtc[DS1307_HR] & DS1307_HI_HR)>>4))+(rtc[DS1307_HR] & DS1307_LO_BCD);
  Time.Day                    = rtc[DS1307_DOW] & DS1307_LO_DOW;
  Time.DaylightSaving         = rtc[DS1307_DLS];
  Time.DaylightSavingSetMonth = rtc[DS1307_DLS_M];
  Time.DaylightSavingSetDate  = rtc[DS1307_DLS_D];

  // Het kan zijn als de klok niet aangesloten is, dat er via I2C 'rommel' gelezen is. Doe eenvoudige check.
  if(Time.Minutes>60 || Time.Hour>23 || Time.Day>8 || Time.Month>12 || Time.Date>31)
    {
    Time.Day=0; // De dag wordt gebruikt als checksum of de klok aanwezig is. Deze nooit op 0 als klok juist aangesloten
    bitWrite(HW_Config,HW_CLOCK,0);
    }
  else
    bitWrite(HW_Config,HW_CLOCK,1);


  // Bereken of het volgens de datum van vandaag zomertijd of wintertijd is. Eventueel de klok verzetten.
  long x=(long)pgm_read_word_near(DLSDate+Time.Year-DLSBase);
  long y=(long)((long)(Time.Date*100L)+(long)(Time.Month*10000L)+(long)Time.Hour);
  boolean DLS = (y>=((x/100L)*100L+30002L) && y<((x%100L)*100L+100003L));  
  x=Time.Month*100                  + Time.Date;  
  y=Time.DaylightSavingSetMonth*100 + Time.DaylightSavingSetDate;

  if(Time.DaylightSaving!=DLS  && x!=y)  // Als DaylightSaving status volgens de RTC niet overeenkomt met de DaylightSaving zoals berekend uit de datum EN de RTC is vandaag nog niet verzet...
    {  
    if(!DLS)// als het zomertijd is en wintertijd wordt
      Time.Hour=Time.Hour==0?23:Time.Hour-1;// ...dan de klok een uur terug.
    else // als het wintertijd is en zomertijd wordt
      Time.Hour=Time.Hour<23?Time.Hour+1:0; //... dan klok uur vooruit.
              
    Time.DaylightSavingSetMonth=Time.Month;
    Time.DaylightSavingSetDate=Time.Date;
    Time.DaylightSaving=DLS;
    ClockSet();// verzet de RTC klok
    }
  }

#if NODO_MEGA
/**********************************************************************************************\
 * 
 * Deze functie vult de globale variabele Time.DayLight met de status van zonsopkomst & -opgang
 \*********************************************************************************************/
void SetDaylight()
{
  // Tabel Sunset & Sunrise: om de 10 dagen de tijden van zonsopkomst en zonsondergang in minuten na middernacht. 
  // Geldig voor in Nederland (gemiddelde voor midden Nederland op 52.00 graden NB en 5.00 graden OL) 
  // Eerste dag is 01 januari, tweede is 10, januari, derde is 20 januari, etc.
  // tussenliggende dagen worden berekend aan de hand van lineaire interpolatie tussen de tabelwaarden. 
  // Afwijking t.o.v. KNMI-tabel is hiermee beperkt tot +/-1 min.

  const int offset=120;  
  int DOY,index,now,up,down;
  int u0,u1,d0,d1;

  DOY=((Time.Month-1)*304)/10+Time.Date;// schrikkeljaar berekening niet nodig, levert slechts naukeurigheidsafwijking van één minuut.
  index=(DOY/10);
  now=Time.Hour*60+Time.Minutes;

  //zomertijd correctie 
  if(Time.DaylightSaving)
  {
    if(now>=60)now-=60;
    else now=now+1440-60;
  }

  u0=pgm_read_word_near(Sunrise+index);
  u1=pgm_read_word_near(Sunrise+index+1);
  d0=pgm_read_word_near(Sunset+index);
  d1=pgm_read_word_near(Sunset+index+1);

  up  =u0+((u1-u0)*(DOY%10))/10;// Zon op in minuten na middernacht
  down=d0+((d1-d0)*(DOY%10))/10;// Zon onder in minuten na middernacht

  Time.Daylight=0;                        // astronomische start van de dag (in de wintertijd om 0:00 uur)
  if(now>=(up-offset))   Time.Daylight=1; // <offset> minuten voor zonsopkomst 
  if(now>=up)            Time.Daylight=2; // zonsopkomst
  if(now>=(down-offset)) Time.Daylight=3; // <offset> minuten voor zonsondergang
  if(now>=down)          Time.Daylight=4; // zonsondergang
}

#endif
#endif CLOCK 

//#######################################################################################################
//##################################### Misc: Conversions     ###########################################
//#######################################################################################################

#if NODO_MEGA
/*********************************************************************************************\
 * kopiëer de string van een commando naar een string[]
 \*********************************************************************************************/
char* cmd2str(int i)
  {
  static char string[80];

  if(i<=COMMAND_MAX)
    strcpy_P(string,(char*)pgm_read_word(&(CommandText_tabel[i])));
  else
    string[0]=0;// als er geen gevonden wordt, dan is de string leeg

  return string;
  }


/*********************************************************************************************\
 * converteer een string met tijd volgens format HH:MM naar een UL int met tijd ----HHMM
 * 0xffffffff indien geen geldige invoer.
 \*********************************************************************************************/
unsigned long str2ultime(char* str)
  {
  byte y=0;
  unsigned long TimeInt=0L;
  byte x=strlen(str);
  
  while(!isdigit(str[x]))x--;
  while(isdigit(str[x]))
    {
    TimeInt|=(str[x]-'0')<<y;
    x--;
    y+=4;
    }
  
  y=8;
  while(!isdigit(str[x]))x--;
  while(isdigit(str[x]))
    {
    TimeInt|=(str[x]-'0')<<y;
    x--;
    y+=4;
    }
    
  x=((TimeInt>>12)&0xf)*10 + ((TimeInt>>8)&0xf); // Uren
  y=((TimeInt>>4 )&0xf)*10 + ((TimeInt   )&0xf); // Minuten
  
  if(x >23 || y>59)
    TimeInt=0xffffffff;

  return TimeInt;
  }

/*********************************************************************************************\
 * converteer een string met datum volgens format DD-MM-YYYY naar een UL int met datum ddmmyyyy
 * 0xffffffff indien geen geldige invoer.
 \*********************************************************************************************/
unsigned long str2uldate(char* str)
  {
  int x,y;
  unsigned long DateInt=0L;
  
  x=strlen(str);
  y=0;
  
  while(!isdigit(str[x]))x--;
  while(isdigit(str[x]))
    {
    DateInt|=((unsigned long)(str[x--]-'0'))<<y;
    y+=4;
    }
 
  if(DateInt<0x100)
    DateInt+=0x2000;

  y=16;
  while(!isdigit(str[x]))x--;
  while(isdigit(str[x]))
    {
    DateInt|=((unsigned long)(str[x--]-'0'))<<y;
    y+=4;
    }

  if((((DateInt>>20)&0xf)*10 + ((DateInt>>16 )&0xf))>12)// Maand: ongeldige invoer
    return 0xffffffff;
  
  y=24;
  while(!isdigit(str[x]))x--;
  while(isdigit(str[x]))
    {
    DateInt|=((unsigned long)(str[x--]-'0'))<<y;
    y+=4;
    }

  if((((DateInt>>28)&0xf)*10 + ((DateInt>>24 )&0xf))>31)// Maand: ongeldige invoer
    return 0xffffffff;
  
  return DateInt;
  }

/*********************************************************************************************\
 * Haal uit een string de commando code. False indien geen geldige commando code.
 \*********************************************************************************************/
int str2cmd(char *command)
  {
  for(int x=0;x<=COMMAND_MAX;x++)
    if(strcasecmp(command,cmd2str(x))==0)
      return x;      

  return false;
  }


/*********************************************************************************************\
 * String mag HEX, DEC
 * Deze routine converteert uit een string een unsigned long waarde.
 \*********************************************************************************************/
unsigned long str2int(char *string)
{
  return(strtoul(string,NULL,0));  
}

/**********************************************************************************************\
 * geeft *char pointer terug die naar een PROGMEM string wijst.
 \*********************************************************************************************/
char* PROGMEM2str(prog_char* text)
{
  byte x=0;
  static char buffer[60];

  do
  {
    buffer[x]=pgm_read_byte_near(text+x);
  }
  while(buffer[x++]!=0);
  return buffer;  
}

/**********************************************************************************************\
 * Converteert een 4byte array IP adres naar een string.
 \*********************************************************************************************/
char* ip2str(byte* IP)
{
  static char str[20];
  sprintf(str,"%u.%u.%u.%u",IP[0],IP[1],IP[2],IP[3]);
  return str;
}

/**********************************************************************************************\
 * Converteert een string naar een 4byte array IP adres
 * 
 \*********************************************************************************************/
boolean str2ip(char *string, byte* IP)
  {
  byte c;
  byte part=0;
  int value=0;

  for(int x=0;x<=strlen(string);x++)
    {
    c=string[x];
    if(isdigit(c))
      {
      value*=10;
      value+=c-'0';
      }

    else if(c=='.' || c==0) // volgende deel uit IP adres
      {
      if(value<=255)
        IP[part++]=value;
      else 
        return false;
      value=0;
      }
    else if(c==' ') // deze tekens negeren
      ;
    else // ongeldig teken
    return false;
    }
  if(part==4)// correct aantal delen van het IP adres
    return true;
  return false;
}


/**********************************************************************************************\
 * Converteert een unsigned long naar een string met decimale integer.
 \*********************************************************************************************/
char* int2str(unsigned long x)
{
  static char OutputLine[12];
  char* OutputLinePosPtr=&OutputLine[10];
  int y;

  *OutputLinePosPtr=0;

  if(x==0)
    {
    *--OutputLinePosPtr='0';
    }
  else
    {  
    while(x>0)
      {
      *--OutputLinePosPtr='0'+(x%10);
      x/=10;
      }
    }    
  return OutputLinePosPtr;
  }

/**********************************************************************************************\
 * Converteert een unsigned long naar een hexadecimale string.
 \*********************************************************************************************/
char* int2strhex(unsigned long x)
  {
  static char OutputLine[12];
  char* OutputLinePosPtr=&OutputLine[10];
  int y;

  *OutputLinePosPtr=0;

  if(x==0)
    {
    *--OutputLinePosPtr='0';
    }
  else
    {  
    while(x>0)
      {
      y=x&0xf;

      if(y<10)
        *--OutputLinePosPtr='0'+y;
      else
        *--OutputLinePosPtr='A'+(y-10);

      x=x>>4;
      ;
    }
    *--OutputLinePosPtr='x';
    *--OutputLinePosPtr='0';
  }
  return OutputLinePosPtr;
}


/**********************************************************************************************\
 * vult een string met een regel uit de Eventlist.
 * geeft false terug als de regel leeg is
 * Let op dat er voldoende ruimte is in [Line]
 \*********************************************************************************************/
boolean EventlistEntry2str(int entry, byte d, char* Line, boolean Script)
  {
  struct NodoEventStruct Event, Action;
  ClearEvent(&Event);
  ClearEvent(&Action);
 
  char *TempString=(char*)malloc(INPUT_LINE_SIZE);
  boolean Ok;

  if(Ok=Eventlist_Read(entry,&Event,&Action)) // lees regel uit de Eventlist. Ga door als gelukt.
    {
    if(Event.Command) // Als de regel gevuld is
      {
      if(!Script)
        {
        strcpy(Line,int2str(entry));
        strcat(Line,": ");
        }                                          
      else
        {
        strcpy(Line,cmd2str(CMD_EVENTLIST_WRITE));
        strcat(Line,"; ");
        }
  
      // geef het event weer
      Event2str(&Event, TempString);
      if(Settings.Alias==VALUE_ON)
        Alias(TempString,false);
      strcat(Line, TempString);
  
      // geef het action weer
      strcat(Line,"; ");
      Event2str(&Action, TempString);  
      if(Settings.Alias==VALUE_ON)
        Alias(TempString,false);
      strcat(Line,TempString);
      }
    else
      Line[0]=0;
    }

  free(TempString);
  return Ok;
  }

/*******************************************************************************************
 *  floatToString.h
 *
 *  Usage: floatToString(buffer string, float value, precision, minimum text width)
 *
 *  Example:
 *  char test[20];    // string buffer
 *  float M;          // float variable to be converted
 *                 // precision -> number of decimal places
 *                 // min text width -> character output width, 0 = no right justify
 * 
 *  Serial.print(floatToString(test, M, 3, 7)); // call for conversion function
 *  
 * Thanks to capt.tagon / Orygun
 **********************************************************************************************/

char * floatToString(char * outstr, double val, byte precision, byte widthp){
  char temp[16]; //increase this if you need more digits than 15
  byte i;

  temp[0]='\0';
  outstr[0]='\0';

  if(val < 0.0){
    strcpy(outstr,"-\0");  //print "-" sign
    val *= -1;
  }

  if( precision == 0) {
    strcat(outstr, ltoa(round(val),temp,10));  //prints the int part
  }
  else {
    unsigned long frac, mult = 1;
    byte padding = precision-1;

    while (precision--)
      mult *= 10;

    val += 0.5/(float)mult;      // compute rounding factor

    strcat(outstr, ltoa(floor(val),temp,10));  //prints the integer part without rounding
    strcat(outstr, ".\0"); // print the decimal point

    frac = (val - floor(val)) * mult;

    unsigned long frac1 = frac;

    while(frac1 /= 10) 
      padding--;

    while(padding--) 
      strcat(outstr,"0\0");    // print padding zeros

    strcat(outstr,ltoa(frac,temp,10));  // print fraction part
  }

  // generate width space padding 
  if ((widthp != 0)&&(widthp >= strlen(outstr))){
    byte J=0;
    J = widthp - strlen(outstr);

    for (i=0; i< J; i++) {
      temp[i] = ' ';
    }

    temp[i++] = '\0';
    strcat(temp,outstr);
    strcpy(outstr,temp);
  }

  return outstr;
}

/*******************************************************************************************************\
 * Converteert een string naar een weekdag. Geldige input:
 * Sun, Mon, ..... All, 0,1..7
 * 1..7=Zondag...zaterdag
 * 0xf=Wildcard
 * 0 als geen geldige invoer
 *
 * LET OP: voorloop spaties en vreemde tekens moeten reeds zijn verwijderd
 \*******************************************************************************************************/
int str2weekday(char *Input)
   {
   char Cmp[4];
   
   int x,y;
   
   // check if de gebruiker de waarde 'All', een '0' of een '*' heeft ingevoerd.
   y=str2cmd(Input);
   if(y==VALUE_ALL || Input[0]=='*' || Input[0]=='0')return 0xF;
   
   // check of de gebruiker een getal 1..7 heeft ingevoerd.
   y=str2int(Input);
   if(y>=1 && y<=7)return y;   
   
   // De string moet nu minimaal drie tekens bevatten, anders geen geldige invoer
   if(strlen(Input)<3)return 0;
   
   Input[3]=0; // Sluit de string voor de zekerheid af zodat er niet meer dan drie posities zijn
   
   for(x=0;x<=6;x++)
     {
     char *StrPtr=ProgmemString(Text_04)+(x*3);
     *(StrPtr+3)=0; // Sluit deze string ook af einde afkorting weekdag
     if(strcasecmp(StrPtr,Input)==0)return x+1;
     }
   return false;   
   }       
#endif

/*******************************************************************************************************\
 * Houdt bij welke Nodo's binnen bereik zijn en via welke Poort.
 * Als Port ongelijk aan reeds bekende poort, dan wordt de lijst geactualiseerd.
 * Als Port=0 dan wordt alleen de poort teruggegeven als de Nodo bekend is.
 \*******************************************************************************************************/
byte NodoOnline(byte Unit, byte Port)
  {
  static byte NodoOnlinePort[UNIT_MAX+1];
  static boolean FirstTime=true;
  
  int x;
  
  // Maak eerste keer de tabel leeg.
  if(FirstTime)
    {
    FirstTime=false;
    for(x=0;x<=UNIT_MAX;x++)
      NodoOnlinePort[x]=0;
    NodoOnlinePort[Settings.Unit]=VALUE_SOURCE_SYSTEM;//Dit is deze unit.
    }
    
  if(Port && Port!=NodoOnlinePort[Unit])
    {
    // Werk tabel bij. Voorkeurspoort voor communicatie.

    if(Port==VALUE_SOURCE_I2C)
      NodoOnlinePort[Unit]=VALUE_SOURCE_I2C;
    else if(Port==VALUE_SOURCE_IR && NodoOnlinePort[Unit]!=VALUE_SOURCE_I2C)
      NodoOnlinePort[Unit]=VALUE_SOURCE_IR;
    else if(Port==VALUE_SOURCE_RF && NodoOnlinePort[Unit]!=VALUE_SOURCE_IR && NodoOnlinePort[Unit]!=VALUE_SOURCE_I2C)
      NodoOnlinePort[Unit]=VALUE_SOURCE_RF;
    }    
  return NodoOnlinePort[Unit];
  }
  
 /********************************************************************************************\
 * Een float en een unsigned long zijn beide 4bytes groot. Deze zijn niet te casten naar 
 * elkaar. Onderstaande twee funkties converteren de unsigned long
 * en de float.
 \*********************************************************************************************/
unsigned long float2ul(float f)
  {
  unsigned long ul;
  memcpy(&ul, &f,4);
  return ul;
  }

float ul2float(unsigned long ul)
  {
  float f;
  memcpy(&f, &ul,4);
  return f;
  }
  
#if NODO_MEGA    
 /********************************************************************************************\
 *
 *
 *  
 \*********************************************************************************************/
boolean Alias(char* Command, boolean IsInput)
  {
  boolean Success=false;
  int c=0,y=0;

  SelectSDCard(true);
  File dataFile=SD.open(PathFile(IsInput?ProgmemString(Text_11):ProgmemString(Text_12),int2strhex(AliasHash(Command))+2,"DAT"));
  
  if(dataFile) 
    {
    while(dataFile.available())
      {
      c=dataFile.read();
      if(isprint(c) && y<(INPUT_COMMAND_SIZE-1) && c!='=')
        Command[y++]=c;
      else
        {
        Command[y]=0;
        break;
        }
      }
    dataFile.close();
    }  
  SelectSDCard(false);

  return y>0;
  } 

 /********************************************************************************************\
 *
 *
 *  
 \*********************************************************************************************/
byte AliasWrite(char* Line)
  {
  byte x=0,y=0,w,error=0;
  unsigned long HashOutput;
  unsigned long HashInput;
    
  char *StrInput=(char*)malloc(INPUT_COMMAND_SIZE);
  char *StrOutput=(char*)malloc(INPUT_COMMAND_SIZE);
  char *String=(char*)malloc(INPUT_LINE_SIZE);

  // Zowel het door de gebruiker opgegeven Input als de route terug moeten worden opgeslagen.
  // Beide worden in een aparte file in een aparte directory opgeslagen zodat beide richtingen uit
  // weer snel gevonden kunnen worden.
  
  do{
    w=Line[x];
    StrInput[x++]=w;              
    }while(w!='=' && w!=0 && x<(INPUT_COMMAND_SIZE-1));
  StrInput[x-1]=0;

    
  do{
    w=Line[x++];
    StrOutput[y++]=w;              
    }while(w!=0 && y<(INPUT_COMMAND_SIZE-1));
    
  
  // een Nodo events is het niet toegestaan om een alias voor aan te maken. Dit omdat anders
  // een situatie kan ontstaan waarbij de Nodo op slot gezet kan worden doordat essentiele commando's niet 
  // meer benaderbaar zijn.  
  GetArgv(StrInput,String,1);
  if(str2cmd(String)!=0)
    {
    error=MESSAGE_INVALID_PARAMETER;
    }    
  else
    {
    // Omdat de Nodo Inputs ook weer terugvertaald moeten worden vanuit de Output, is het nodig om het opgegeven Input
    // op te bouwen zoals deze ook door de Nodo weergegeven wordt. Dus [Sound] wordt dan [Sound 0,0]
    struct NodoEventStruct TempEvent;
    if(Str2Event(StrOutput,&TempEvent)==0)
      Event2str(&TempEvent,StrOutput);
  
    HashInput  = AliasHash(StrInput);
    HashOutput = AliasHash(StrOutput);

    // Wis eventuele bestaande Output die is aangemaakt bij een eerder alias commando 
    AliasErase(StrOutput);
  
    strcpy(String, StrOutput);
    strcat(String, "=");
    strcat(String, StrInput); 
    FileWriteLine(ProgmemString(Text_11),int2strhex(HashInput)+2,"DAT",String,true);//Input map

    strcpy(String, StrInput);
    strcat(String, "=");
    strcat(String, StrOutput); 
    error=FileWriteLine(ProgmemString(Text_12),int2strhex(HashOutput)+2,"DAT",String,true);// Output map
    }
    
  free(StrInput);
  free(StrOutput);
  free(String);
  return error;
  }  

/********************************************************************************************\
 *
 *
 *  
 \*********************************************************************************************/
byte AliasErase(char* FileToDelete)
  {
  char *Keyword=(char*)malloc(INPUT_COMMAND_SIZE);
  strcpy(Keyword,FileToDelete);// maak een kopie anders wordt de originele string in de functiecall onbedoeld veranderd.

  unsigned long Hash_1=AliasHash(Keyword); // Input
  Alias(Keyword,true);
  unsigned long Hash_2=AliasHash(Keyword); // Output

  if(Hash_1==42)// Keyword = "*"
    {
    FileErase(ProgmemString(Text_11),"*","DAT");// Input
    FileErase(ProgmemString(Text_12),"*","DAT");// Output
    }
  else
    {
    FileErase(ProgmemString(Text_11),int2strhex(Hash_1)+2,"DAT"); // Input
    FileErase(ProgmemString(Text_12),int2strhex(Hash_2)+2,"DAT"); // Output
    }

  free(Keyword);
  return 0;
  }  

 /********************************************************************************************\
 * Deze routine berekent uit een string (Commando) een hashwaarde volgens de Daniel Bernstein 
 * methode. In het commando worden parameters van elkaar gescheiden door komma's of spaties.
 * De string wordt eerst 'opgeschoond' zodat spaties en komma's niet meer van invloed zijn
 * op de berekende hashwaarde. Ook hoofdletters zijn niet van invloed.   
 \*********************************************************************************************/
unsigned long AliasHash(char* Input)
  {
  char *TmpStr=(char*)malloc(INPUT_COMMAND_SIZE);
  char *c;
  unsigned long Hash=0UL;
  int x=1;
    
  while(GetArgv(Input,TmpStr,x++))
    {
    c=TmpStr;
    while(*c) Hash=Hash * 33 + tolower(*c++);
    }  
  
  free(TmpStr);
  return Hash;
  }  
  
  
/*********************************************************************************************\
 * Wis een file
 \*********************************************************************************************/
void AliasList(char* Keyword, byte Port)
  {
  SelectSDCard(true);
    
  File root;
  File entry;
  char *TempString=(char*)malloc(INPUT_COMMAND_SIZE);
  int y;
  
  SelectSDCard(true);
  if(root=SD.open(ProgmemString(Text_12))) // We nemen de output map
    {
    root.rewindDirectory();
    while(entry = root.openNextFile())
      {
      if(!entry.isDirectory())
        {
        strcpy(TempString,entry.name());
        y=StringFind(TempString,".");
        TempString[y]=0;
        FileShow(ProgmemString(Text_12), TempString, "DAT", Port);
        SelectSDCard(true);
        }
      entry.close();
      }                                                                       
    root.close();
    }
  free(TempString);  
  SelectSDCard(false);
  }  


#endif


#if !NODO_MEGA
 /********************************************************************************************\
 * Onderstaande funkties zijn alleen voor de Small en zetten de Nodo in de sleep mode
 * zodat batterij voeding mogelijk is.
 *  
 \********************************************************************************************/

#if SLEEP
#include <avr/sleep.h>
#include <avr/wdt.h>

// watchdog interrupt routine
ISR (WDT_vect) 
  {
  wdt_disable();  // disable watchdog
  }


#define WDT_TIME 8000
void GoodNightSleepTight(void)
  {
  // De WDT timer haalt de Nodo om de acht seconden uit de Sleep mode. We blijven net zo lang in de Sleep mode totdat
  // een timer is afgelopen. Aangezien de timers tijdens de Sleep mode niet meer lopen, zullen we deze zelf moeten laten
  // aftellen. De eerste timer die afloopt bepaalt de Sleep tijd. Dit mechanisme zorgt wel voor onnauwkeurigheid in het
  // aflopen van de timers. Als er geen timer binnen 10 seconden afloopt, dan wordt de sleep mode NIET geactiveerd.
  unsigned long SleepTimer=0L;
  byte x;
  

  // Haal de tijd op van de eerste timer die afloopt.
  SleepTimer=0xffffff;
  for(x=0;x<TIMER_MAX;x++)
    if(UserTimer[x]>0 && UserTimer[x]<SleepTimer)
      SleepTimer=UserTimer[x];

  // bereken de tijd in eenheden van 8 seconden dat de Nodo moet gaan slapen. 
  SleepTimer=(SleepTimer-millis())/WDT_TIME+1;

  // Als we uit de Sleep mode komen, dan hebben alle timers een achterstand gelijk aan de Sleep tijd omdat
  // millis(); stil staat gedurende de sleep mode. Zet de Timers alvast op de juiste tijd. 
  for(x=0;x<TIMER_MAX;x++)
    if(UserTimer[x]>0)                                                          // Als de timer in gebruik, dan bevat deze een tijdstip in millis()
      UserTimer[x]-=SleepTimer*WDT_TIME;                                        // dan min acht seconden

  // Spaar energie door de LED uit te zetten en de spanning naar de ontvanger.
  Led(0);     
  digitalWrite(PIN_RF_RX_VCC,LOW);                                            // Spanning naar de RF ontvanger uit

  // Zzzzz.....
  while(SleepTimer--)
    wdsleep();

  // Spanning weer op de RF ontvanger anders is de Nodo doof.
  digitalWrite(PIN_RF_RX_VCC,HIGH);                                             // Spanning naar de RF ontvanger weer aan
  Led(RED);     
  }

void wdsleep()
  {
  ADCSRA = 0;                                                                   // disable ADC  
  MCUSR = 0;                                                                    // clear various "reset" flags     
  WDTCSR = _BV (WDCE) | _BV (WDE);                                              // allow changes, disable reset
  WDTCSR = _BV (WDIE) | _BV (WDP3) | _BV (WDP0);                                // set interrupt mode and an interval set WDIE, and 8 seconds delay
  wdt_reset();                                                                  // pat the dog
 
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
  sleep_enable();

  // turn off brown-out enable in software
  MCUCR = _BV (BODS) | _BV (BODSE);
  MCUCR = _BV (BODS); 
  sleep_cpu();  
   
  // cancel sleep as a precaution
  sleep_disable();
  }
#endif
#endif
