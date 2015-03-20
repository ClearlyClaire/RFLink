
/**********************************************************************************************\
 * verzendt een event en geeft dit tevens weer op SERIAL
 * Als UseRawSignal=true, dan wordt er geen signaal opgebouwd, maar de actuele content van de
 * RawSignal buffer gebruikt. In dit geval werkt de WaitFree niet.
 \*********************************************************************************************/
boolean SendEvent(struct NodoEventStruct *ES, boolean UseRawSignal, boolean Display, boolean WaitForFree)
  {    
  ES->Direction=VALUE_DIRECTION_OUTPUT;
  byte Port=ES->Port;
    
  if(Settings.WaitFreeNodo==VALUE_ON)
    {
    if(BusyNodo!=0)                                                               // Als een Nodo heeft aangegeven busy te zijn, dan wachten.
      {
      if(!Wait(30,true,0,false))
        {
        for(byte x=1;x<=31;x++)
          if(BusyNodo&(1<<x))
            RaiseMessage(MESSAGE_BUSY_TIMEOUT,x);

        BusyNodo=0;
        }
      }
      
    if(ES->Type!=NODO_TYPE_SYSTEM && ES->Command!=SYSTEM_COMMAND_CONFIRMED)
      {
      ES->Flags|=TRANSMISSION_BUSY | TRANSMISSION_QUEUE | TRANSMISSION_QUEUE_NEXT;
      RequestForConfirm=true;
      }
    }  
  // PrintNodoEvent("DEBUG: SendEvent():", ES);

  // loop de plugins langs voor eventuele afhandeling van dit event.
  PluginCall(PLUGIN_EVENT_OUT, ES,0);

  // Stuur afhankelijk van de instellingen het event door naar I2C, RF, IR. Eerst wordt het event geprint,daarna een korte wachttijd om
  // te zorgen dat er een minimale wachttijd tussen de signlen zit. Tot slot wordt het signaal verzonden.
  if(WaitForFree)
    if(Port==VALUE_SOURCE_RF || Port==VALUE_SOURCE_IR ||(Settings.TransmitRF==VALUE_ON && Port==VALUE_ALL))
      WaitFree();

  if(!UseRawSignal)
    Nodo_2_RawSignal(ES);

  // Respecteer een minimale tijd tussen verzenden van events. Wachten alvorens event te verzenden.
  while(millis()<HoldTransmission);  
                                             
  // Verstuur signaal als IR
  if(Settings.TransmitIR==VALUE_ON && (Port==VALUE_SOURCE_IR || Port==VALUE_ALL))
    { 
    ES->Port=VALUE_SOURCE_IR;
    #if NODO_MEGA
    if(Display)PrintEvent(ES,VALUE_ALL);
    #endif
    RawSendIR();
    }

  #ifdef ethernetserver_h
  // Verstuur signaal als HTTP-event.
  if(bitRead(HW_Config,HW_ETHERNET))// Als Ethernet shield aanwezig.
    {
    if((Settings.TransmitHTTP==VALUE_ALL || Settings.TransmitHTTP==VALUE_ON) && (Port==VALUE_SOURCE_HTTP || Port==VALUE_ALL) && Settings.PortOutput!=0 )
      {
      SendHTTPEvent(ES);
      ES->Port=VALUE_SOURCE_HTTP;
      if(Display)PrintEvent(ES,VALUE_ALL);
      }
    }
  #endif 

  // Verstuur event via I2C
  // LET OP:  Voor I2C geldt een verwerking: Als een device een signaal verzendt, dan mag dit commando alleen er toe leiden dat het
  //          RawSignal wordt verzonden via RF/IR. Anders zal het commando worden verstuurd over I2C waarna de Nodo's op I2C het commando nogmaal
  //          zullen uitvoeren. Zo zal er een ongewenste loop ontstaan.
  
  #if I2C
  if(!UseRawSignal)
    {
    if((Port==VALUE_SOURCE_I2C || Port==VALUE_ALL) && bitRead(HW_Config,HW_I2C))
      {
      ES->Port=VALUE_SOURCE_I2C;
      #if NODO_MEGA
      if(Display)PrintEvent(ES,VALUE_ALL);
      #endif
      SendI2C(ES);
      }
    }
  #endif
  
  // Verstuur signaal als RF
  if(Settings.TransmitRF==VALUE_ON && (Port==VALUE_SOURCE_RF || Port==VALUE_ALL))
    {
    ES->Port=VALUE_SOURCE_RF;
    #if NODO_MEGA
    if(Display)PrintEvent(ES,VALUE_ALL);
    #endif
    RawSendRF();
    }

  HoldTransmission=DELAY_BETWEEN_TRANSMISSIONS+millis();        
  }
  
#if NODO_MEGA
#endif


//#######################################################################################################
//##################################### Transmission: I2C  ##############################################
//#######################################################################################################


// Deze routine wordt vanuit de Wire library aangeroepen zodra er data op de I2C bus verschijnt die voor deze nodo bestemd is.
// er vindt geen verdere verwerking plaats, slechts opslaan van het data. 
#if I2C
void ReceiveI2C(int n)
  {
  I2C_Received=0;
  byte b;
  
  while(WireNodo.available()) // Haal de bytes op
    {
    b=WireNodo.read();
    if(I2C_Received<I2C_BUFFERSIZE)
      I2C_ReceiveBuffer[I2C_Received++]=b; 
    }
  }

/**********************************************************************************************\
 * Verstuur een Event naar de I2C bus. 
 \*********************************************************************************************/
void SendI2C(struct NodoEventStruct *EventBlock)
  {  
  byte x;
  byte *B=(byte*)EventBlock;  

  Checksum(EventBlock);                                                         // bereken checksum: crc-8 uit alle bytes in de struct.

  for(int y=1;y<=UNIT_MAX;y++)
    {            
    // We sturen de events naar bekende Nodo's die zich op de I2C bus bevinden. Als deze Nodo later op de bus wordt aangesloten,
    // dan zal Boot event van deze Nodo de andere uitnodigen om een bekendmaking te sturen zodat de lijst compleet is.
    // Daarom wordt het Boot, NewNodo en POLL event naar alle I2C adressen gestuurd waar zich een Nodo op kan bevinden.
    if((EventBlock->Type==NODO_TYPE_EVENT  && (EventBlock->Command==EVENT_BOOT || EventBlock->Command==EVENT_NEWNODO)) || 
      (EventBlock->Type==NODO_TYPE_SYSTEM &&  EventBlock->Command==SYSTEM_COMMAND_QUEUE_POLL) || 
      NodoOnline(y,0)==VALUE_SOURCE_I2C)
      {
      WireNodo.beginTransmission(I2C_START_ADDRESS+y-1);
      for(x=0;x<sizeof(struct NodoEventStruct);x++)
        WireNodo.write(*(B+x));
      WireNodo.endTransmission(false);                                          // verzend de data, sluit af maar geef de bus NIET vrij
      }
    }
  WireNodo.endTransmission(true);                                               // Geef de bus vrij
  }
#endif // I2C

#define IP_BUFFER_SIZE            256

#if NODO_MEGA

//#######################################################################################################
//##################################### Transmission: HTTP  #############################################
//#######################################################################################################
#ifdef ethernetserver_h

boolean IPSend(char* URL, int Port, char* Request) // Nog niet operationeel. Test.
  {
  int InByteCounter;
  byte InByte;
  byte Try=0;
  byte error=0;
  unsigned long TimeoutTimer;
  const int TimeOut=5000;
  EthernetClient IPClient;

  //Serial.print(F("URL="));Serial.println(URL);
  //Serial.print(F("Port="));Serial.println(Port);
  //Serial.print(F("Request="));Serial.println(Request);

  do
    {
    if(IPClient.connect(URL,Port))
      {
      IPClient.println(Request);
      TimeoutTimer=millis()+TimeOut; // Als er te lange tijd geen datatransport is, dan wordt aangenomen dat de verbinding (om wat voor reden dan ook) is afgebroken.

      while(TimeoutTimer>millis() && IPClient.connected())
        {
        if(IPClient.available())
          {
          InByte=IPClient.read();
          //Serial.write(InByte);
          TimeoutTimer=millis()+TimeOut; // er is nog data transport, dus de timeout timer weer op max. zetten.
          }
        //Serial.println();
        }        
      delay(100);
      IPClient.flush();// Verwijder eventuele rommel in de buffer.
      IPClient.stop();
      }
    else
      {
      error=MESSAGE_TCPIP_FAILED; 
      }
    }
  while(error && ++Try<3);

  return error;
  }



boolean EthernetInit(void)
  {
  int x;
  boolean Ok=false;
  byte Ethernet_MAC_Address[6];                                // MAC adres van de NodO

  if(!bitRead(HW_Config,HW_ETHERNET))
    return false;
    
  // Stel MAC adres in. Hiering zit het unitnummer van de Nodo verwerkt.
  Ethernet_MAC_Address[0]=ETHERNET_MAC_0;
  Ethernet_MAC_Address[1]=ETHERNET_MAC_1;
  Ethernet_MAC_Address[2]=ETHERNET_MAC_2;
  Ethernet_MAC_Address[3]=(HOME_NODO    %10)+'0';
  Ethernet_MAC_Address[4]=(Settings.Unit/10)+'0';
  Ethernet_MAC_Address[5]=(Settings.Unit%10)+'0';

  // Initialiseer ethernet device  
  if((Settings.Nodo_IP[0] + Settings.Nodo_IP[1] + Settings.Nodo_IP[2] + Settings.Nodo_IP[3])==0)// Als door de user IP adres (of is ingesteld op 0.0.0.0) dan IP adres ophalen via DHCP
    {
    if(EthernetNodo.begin(Ethernet_MAC_Address)!=0) // maak verbinding en verzoek IP via DHCP
      Ok=true;
    }
  else      
    {
    EthernetNodo.begin(Ethernet_MAC_Address, Settings.Nodo_IP, Settings.DnsServer, Settings.Gateway, Settings.Subnet);
    Ok=true;
    }

  if(Ok) // Als er een IP adres is, dan HTTP en TelNet servers inschakelen
    {
    // Start Server voor ontvangst van HTTP-Events
    IPServer=EthernetServer(Settings.PortInput);
    IPServer.begin(); 

    // Start server voor Terminalsessies via TelNet
    TerminalServer=EthernetServer(TERMINAL_PORT);
    TerminalServer.begin(); 

    if(Settings.TransmitHTTP!=VALUE_OFF && Settings.HTTPRequest[0]!=0)
      {
      // Haal IP adres op van de Host waar de nodo de HTTP events naar verzendt zodat niet iedere transactie een DNS-resolve plaats hoeft te vinden.
      // Haal uit het HTTP request URL de Host. Zoek naar de eerste slash in de opgegeven HTTP-Host adres
      char *TempString=(char*)malloc(80);
      x=StringFind(Settings.HTTPRequest,"/");
      strcpy(TempString,Settings.HTTPRequest);
      TempString[x]=0;

      EthernetClient IPClient;
      if(IPClient.connect(TempString,Settings.PortOutput))   
        {
        IPClient.getRemoteIP(IPClientIP);
        Ok=true;
        bitWrite(HW_Config,HW_WEBAPP,1);
        delay(10); //even wachten op response van de server.
        IPClient.flush(); // gooi alles weg, alleen IP adres was van belang.
        IPClient.stop();
        }
      else
        {
        IPClientIP[0]=0;
        IPClientIP[1]=0;
        IPClientIP[2]=0;
        IPClientIP[3]=0;
        //Serial.println(F("Error: No TCP/IP connection to host."));
        Ok=false;
        }
      free(TempString);    
      }
    else
      Ok=true;
    }
  return Ok;
  }

/*******************************************************************************************************\
 * Haal via een HTTP-request een file op
 * De content van de file bevindt zich in de body text die de server terugstuurt en wordt opgevangen
 * in de funktie SendHTTPRequest()
 *
 * Het naar de Nodo schrijven van een scriptfile werkt als volgt:
 *
 * 1. Webserver: Stuurt een regulier http-request naar de Nodo met daarin het commando 'FileGetHttp <filename>'
 * 2. Nodo: Stuurt een http-request terug met '&file=<filename>'
 * 3. WebServer: geeft een standaard response OK 200 terug en verzendt in dezelfde sessie de file als http-bodytext
 * 4. Nodo: ontvangt de OK 200 en slaat alle regels in de bodytext op in de opgegeven bestandsnaam
 * 5. Sessie wordt door server beeindigd en file op de Nodo wordt gesloten.
 * 
 * Een Nodo file bestaat uit acht posities zonder spaties, andere leestekens en zonder extentie.
 * 
 \*******************************************************************************************************/
byte GetHTTPFile(char* filename)
  {
  char *HttpRequest=(char*)malloc(INPUT_LINE_SIZE);
  char *TempString=(char*)malloc(INPUT_LINE_SIZE);
  byte Ok;

  strcpy(HttpRequest,"?id=");
  strcat(HttpRequest,Settings.ID);  
  strcat(HttpRequest,"&file=");
  strcat(HttpRequest,filename);

  if(Settings.Password[0]!=0)
    {
    // sleutel genereren en meesturen in het http-request
    strcpy(TempString,HTTPCookie);
    strcat(TempString,":");
    strcat(TempString,Settings.Password);  
    md5(TempString);
    strcat(HttpRequest,"&key=");
    strcat(HttpRequest,TempString); 
    }

  free(TempString);

  Ok=SendHTTPRequest(HttpRequest);
  free(HttpRequest);
  return Ok;
  }

/*******************************************************************************************************\
 * Verzend een event als HTTP-request 
 *
 \*******************************************************************************************************/
byte SendHTTPEvent(struct NodoEventStruct *Event)
  {
  byte x;

  if(Settings.TransmitHTTP==VALUE_OFF || Settings.HTTPRequest[0]==0)
    return false;

  char *HttpRequest=(char*)malloc(INPUT_LINE_SIZE);
  char *TempString=(char*)malloc(INPUT_LINE_SIZE);

  strcpy(HttpRequest,"?id=");
  strcat(HttpRequest,Settings.ID);  

  strcat(HttpRequest,"&unit=");
  strcat(HttpRequest,int2str(Event->SourceUnit));  

  if(Settings.Password[0]!=0)
    {
    // sleutel genereren en meesturen in het http-request
    strcpy(TempString,HTTPCookie);
    strcat(TempString,":");
    strcat(TempString,Settings.Password);
    md5(TempString);
    strcat(HttpRequest,"&key=");
    strcat(HttpRequest,TempString);    
    }

  strcat(HttpRequest,"&event=");
  Event2str(Event,TempString);
  if(Settings.Alias==VALUE_ON)
    Alias(TempString,false);

  strcat(HttpRequest,TempString);

  free(TempString);
  x=SendHTTPRequest(HttpRequest);
  free(HttpRequest);
  return x;
}


/*******************************************************************************************************\
 * Verzend een nieuwe cookie als HTTP request.
 *
 \*******************************************************************************************************/
boolean SendHTTPCookie(void)
{
  boolean Status;

  if(Settings.TransmitHTTP==VALUE_OFF)
    return false;

  char *HttpRequest=(char*)malloc(INPUT_LINE_SIZE);

  strcpy(HttpRequest,"?id=");
  strcat(HttpRequest,Settings.ID);  

  RandomCookie(HTTPCookie);                                                     // Verzend tevens een nieuwe cookie voor het eerstvolgende event.
  strcat(HttpRequest,"&cookie=");
  strcat(HttpRequest,HTTPCookie);

  Status=SendHTTPRequest(HttpRequest);
  free(HttpRequest);
  return Status;
  }


boolean SendHTTPRequest(char* Request)
  {
  if(Settings.TransmitHTTP==VALUE_OFF)
    return false;

  int InByteCounter,x,y,SlashPos;
  byte InByte;
  byte Try=0;
  unsigned long TimeoutTimer;
  char filename[13];
  EthernetClient IPClient;                                                      // Client class voor HTTP sessie.
  byte State=0;
  char *IPBuffer=(char*)malloc(IP_BUFFER_SIZE);
  char *TempString=(char*)malloc(INPUT_LINE_SIZE);
  File BodyTextFile;
  struct NodoEventStruct TempEvent;
  int ContentLength=0;

  filename[0]=0;
  if(ParseHTTPRequest(Request,"file=", TempString,"&? "))                       // pluk de filename uit het http request als die er is.
    {
    TempString[8]=0;                                                            // voorkom dat filenaam meer dan acht posities heeft
    strcpy(filename,TempString);                
    SelectSDCard(true);
    FileErase("", filename,"DAT");
    SelectSDCard(false);
    }  

  strcpy(IPBuffer,"GET ");

  SlashPos=StringFind(Settings.HTTPRequest,"/");                                // Haal uit het HTTP request URL de Host. Zoek naar de eerste slash in de opgegeven HTTP-Host adres
  if(SlashPos!=-1)
    strcat(IPBuffer,Settings.HTTPRequest+SlashPos);

  y=strlen(IPBuffer);                                                           // Alle spaties omzetten naar %20 en toevoegen aan de te verzenden regel.
  for(x=0;x<strlen(Request);x++)
    {            
    if(Request[x]==32)
      {
      IPBuffer[y++]='%';
      IPBuffer[y++]='2';
      IPBuffer[y++]='0';
      }
    else
      {
      IPBuffer[y++]=Request[x];
      }
    }
  IPBuffer[y]=0;

  strcat(IPBuffer," HTTP/1.1");                                                 // Sluit HTTP-request af met protocol versienummer
                                                                                // IPBuffer bevat nu het volledige HTTP-request, gereed voor verzending.
  if(Settings.Debug==VALUE_ON)
    {
    //Serial.print(F("Debug: HTTP-Output="));
    //Serial.println(IPBuffer);
    }

  strcpy(TempString,Settings.HTTPRequest);
  TempString[SlashPos]=0;

  do
    {
    if(IPClient.connect(IPClientIP,Settings.PortOutput))
      {
      ClientIPAddress[0]=IPClientIP[0];
      ClientIPAddress[1]=IPClientIP[1];
      ClientIPAddress[2]=IPClientIP[2];
      ClientIPAddress[3]=IPClientIP[3];

      IPClient.println(IPBuffer);
      IPClient.print(F("Host: "));
      IPClient.println(TempString);
      IPClient.print(F("User-Agent: Nodo/Build="));
      IPClient.println(int2str(NODO_BUILD));             
      IPClient.println(F("Connection: Close"));
      IPClient.println();                                                       // Afsluiten met een lege regel is verplicht in http protocol/

      TimeoutTimer=millis()+3000;                                               // Als er te lange tijd geen datatransport is, dan wordt aangenomen dat de verbinding (om wat voor reden dan ook) is afgebroken.
      IPBuffer[0]=0;
      InByteCounter=0;

      
      while(TimeoutTimer>millis() && IPClient.connected())                      // afbreken als alle tekens in bodytext ontvangen.
        {
        if(IPClient.available())
          {
          InByte=IPClient.read();
          
          if(State==2 && ContentLength>0)                                       // Als de bodytext verzonden wordt, dan tellen hoeveel bytes al verzonden door client
            ContentLength--;
          
          if(isprint(InByte) && InByteCounter<(IP_BUFFER_SIZE-1))
            IPBuffer[InByteCounter++]=InByte;

          else if(InByte==0x0A)
            {
            IPBuffer[InByteCounter]=0;                                          // De regel is binnen

            if(Settings.Debug==VALUE_ON)
              {
              strcpy(TempString,"DEBUG: HTTP-Input=");
              strcat(TempString,IPBuffer);
              //Serial.println(TempString);
              }
            TimeoutTimer=millis()+3000;                                         // er is nog data transport, dus de timeout timer weer op max. zetten.

            if(State==0)                                                        // State 0: wachten totdat in de initial-request line OK/200 voorbij komt en er dus response is van de server.
              {
              if(StringFind(IPBuffer,"200")!=-1 && StringFind(IPBuffer,"HTTP")!=-1)
                {
                State=1;
                }
              }
    
            else if(State==1)                                                   // State 1: In deze state komen de header-lines voorbij. Deze state is klaar zodra er een lege header-line voorbij komt.
              {            
              if(InByteCounter==0)
                {
                State=2;
                if(filename[0])                                                 // Openen nieuw bestand.
                  {
                  SelectSDCard(true);
                  BodyTextFile = SD.open(PathFile("",filename,"DAT"), FILE_WRITE);
                  SelectSDCard(false);
                  }
                }
              else
                {
                if(ParseHTTPRequest(IPBuffer,"content-length:", TempString,";, "))
                  {
                  ContentLength=str2int(TempString);
                  }

                if(ClockSyncHTTP)                                               // als de tijd wordt meegeleverd, dan de clock gelijkzetten.
                  {
                  if(ParseHTTPRequest(IPBuffer,"time=", TempString,";, "))
                    {
                    ClearEvent(&TempEvent);
                    TempEvent.Command=CMD_CLOCK_TIME;
                    TempEvent.Par2=str2ultime(TempString);
                    if(TempEvent.Par2<0xffffffff)
                      ExecuteCommand(&TempEvent);
                    }            

                  if(ParseHTTPRequest(IPBuffer,"date=", TempString,";, "))
                    {
                    ClearEvent(&TempEvent);
                    TempEvent.Command=CMD_CLOCK_DATE;
                    TempEvent.Par2=str2uldate(TempString);
                    if(TempEvent.Par2<0xffffffff)
                      ExecuteCommand(&TempEvent);
                    }            
                  }            
                }
              }

            else if(State==2)                                                   // State 2: In deze state komen de regels van de body-text voorbij
              {
              if(BodyTextFile)                                                  // als bodytext moet worden opgeslagen in en bestand
                {
                SelectSDCard(true);
                BodyTextFile.write((uint8_t*)IPBuffer,strlen(IPBuffer));
                BodyTextFile.write('\n');                                       // nieuwe regel
                SelectSDCard(false);
                }
              }
            InByteCounter=0;          
            }
          }

        if(State==2 && ContentLength==0)                                        // Als er geen content meer wordt verwacht in de bodytext, dan gereed.
          {
          TimeoutTimer=0;
          }                
        }

      if(BodyTextFile)                                                          // eventueel geopende bestand op SDCard afsluiten
        {
        SelectSDCard(true);
        BodyTextFile.close();
        SelectSDCard(false);
        }        
      IPClient.flush();                                                         // Verwijder eventuele rommel in de buffer.
      IPClient.stop();
      }
    else                                                                        // niet gelukt om de TCP-IP verbinding op te zetten. Genereer error en her-initialiseer de ethernetkaart.
      {
      State=0;
      delay(3000);                                                              // korte pause tussen de nieuwe pogingen om verbinding te maken.
      if(EthernetInit())
        CookieTimer=1;                                                          // gelijk een nieuwe cookie versturen.
      }
    }
  while(State==0 && ++Try<3);

  free(TempString);
  free(IPBuffer);

  if(!State)
    {
    x=Settings.TransmitHTTP;                                                      // HTTP tijdelijk uitzetten want die deed het immers niet.
    Settings.TransmitHTTP=VALUE_OFF;                                              // HTTP tijdelijk uitzetten want die deed het immers niet.
    RaiseMessage(MESSAGE_TCPIP_FAILED,0);
    Settings.TransmitHTTP=x;                                                      // HTTP weer terugzetten naar oorspronkelijke waarde.
    }

  return State;
  }



/*********************************************************************************************\
 * Deze routine haalt uit een string de waarde die bij de opgegeven parameter hoort
 * er wordt niet case sensitief gezocht naar KeyWord. Opvolgende spaties worden verwijderd en de tekens
 * tot aan de eerste worden toegevoed aan ResultString
 * eventuele %20 tekens worden omgezet naar spaties.
 \*********************************************************************************************/
boolean ParseHTTPRequest(char* StringToParse,char* Keyword, char* ResultString,char* BreakChars)
  {
  int x,y,z;
  int Keyword_len=strlen(Keyword);
  int StringToParse_len=strlen(StringToParse);

  ResultString[0]=0;

  if(StringToParse_len<3) // doe geen moeite als de string te weinig tekens heeft.
    return -1;

  for(x=0; x<=(StringToParse_len-Keyword_len); x++)
    {
    y=0;
    while(y<Keyword_len && (tolower(StringToParse[x+y])==tolower(Keyword[y])))
      y++;

    z=x+y;
    if(y==Keyword_len)
      {
      // tekens die overgeslagen kunnen worden
      while(z<StringToParse_len && StringToParse[z]==' ')z++;

      x=0; // we komen niet meer terug in de 'for'-loop, daarom kunnen we x hier even gebruiken.
      y=0; // Vlag die aangeeft if een break teken is gevonden
      // Tekens waarbij afgebroken moet worden
      while(z<StringToParse_len && y<100)
        {
        if(StringToParse[z]=='+')
          ResultString[x]=' ';
        else if(StringToParse[z]=='%' && StringToParse[z+1]=='2' && StringToParse[z+2]=='0')
          {
          ResultString[x]=' ';
          z+=2;
          }
        else
          ResultString[x]=StringToParse[z];

        z++;
        x++;
        
        for(y=0;y<strlen(BreakChars);y++)
          if(BreakChars[y]==StringToParse[z])
            y=100; // stop tekens toevoegen aan resultstring
        }        
      ResultString[x]=0;
      
      return true;
      }
    }
  return false;
  }

void ExecuteIP(void)
  {
  char InByte;
  boolean RequestCompleted=false;  
  boolean Completed=false;
  int Protocol=0;
  int InByteCounter;
  boolean RequestEvent=false;
  boolean ExecuteEvent=false;
  int x;
  unsigned long TimeoutTimer=millis() + 60000; //
 
  char *InputBuffer_IP = (char*) malloc(IP_BUFFER_SIZE);
  char *Event          = (char*) malloc(INPUT_LINE_SIZE);
  char *TmpStr1        = (char*) malloc(INPUT_LINE_SIZE);
  char *TmpStr2        = (char*) malloc(40); // 40 is genoeg want er hoeft alleen een MD5 string in 

  Event[0]=0; // maak de string leeg.

  IPClient=IPServer.available();

  if(IPClient)
    {
    IPClient.getRemoteIP(ClientIPAddress);  

    // Controleer of het IP adres van de Client geldig is. 
    if((Settings.Client_IP[0]!=0 && ClientIPAddress[0]!=Settings.Client_IP[0]) ||
      (Settings.Client_IP[1]!=0 && ClientIPAddress[1]!=Settings.Client_IP[1]) ||
      (Settings.Client_IP[2]!=0 && ClientIPAddress[2]!=Settings.Client_IP[2]) ||
      (Settings.Client_IP[3]!=0 && ClientIPAddress[3]!=Settings.Client_IP[3]))
      {
      RaiseMessage(MESSAGE_ACCESS_DENIED,0);
      }
    else
      {
      InByteCounter=0;
      while(IPClient.connected()  && !Completed && TimeoutTimer>millis())
        {
        if(IPClient.available()) 
          {
          InByte=IPClient.read();

          if(isprint(InByte) && InByteCounter<(IP_BUFFER_SIZE-1))
            {
            InputBuffer_IP[InByteCounter++]=InByte;
            }

          else if((InByte==0x0D || InByte==0x0A))
            {
            InputBuffer_IP[InByteCounter]=0;
            InByteCounter=0;

            if(Settings.Debug==VALUE_ON)
              {
              //Serial.print(F("Debug: HTTP-Input="));
              //Serial.println(InputBuffer_IP);
              }

            // Kijk of het een HTTP-request is
            if(Protocol==0)
              {
              if(StringFind(InputBuffer_IP,"GET")!=-1)
                Protocol=VALUE_SOURCE_HTTP;// HTTP-Request
              }

            if(Protocol==VALUE_SOURCE_HTTP)
              {
              if(!RequestCompleted)
                {
                Completed=true;

                // als de beveiliging aan staat, dan kijken of de juiste pin ip meegegeven in het http-request. x is vlag voor toestemming verwerking event
                x=false;
                if(Settings.Password[0]!=0)
                  {
                  sprintf(TmpStr2,"%s:%s",HTTPCookie,Settings.Password);  
                  md5(TmpStr2);

                  if(ParseHTTPRequest(InputBuffer_IP,"key=",TmpStr1,"&? "))
                    {
                    if(strcmp(TmpStr2,TmpStr1)==0)
                      x=true;
                    }
                  }
                else
                  x=true;

                if(x)
                  {                
                  if(ParseHTTPRequest(InputBuffer_IP,"event=",Event,"&? "))
                    {
                    RequestEvent=true;
                    RequestCompleted=true;
                    strcpy(TmpStr1,"HTTP/1.1 200 Ok");
                    IPClient.println(TmpStr1);
                    IPClient.println(TmpStr1);
                    ExecuteEvent=true;
                    }
                  else
                    IPClient.println(F("HTTP/1.1 400 Bad Request"));
                  }
                else                    
                  IPClient.println(F("HTTP/1.1 403 Forbidden"));
                }

              IPClient.println(F("Content-Type: text/html"));
              IPClient.print(F("Server: Nodo/Build="));
              IPClient.println(int2str(NODO_BUILD));             
              #if CLOCK
              if(bitRead(HW_Config,HW_CLOCK))
                {
                IPClient.print(F("Date: "));
                IPClient.println(DateTimeString());             
                }
              #endif clock
              
              IPClient.println(""); // HTTP Request wordt altijd afgesloten met een lege regel

              // Haal nu de resultaten op van het verwerken van de binnengekomen HTTP-regel. Stuur de inhoud als bodytext terug
              // naar de client. De WebApp zal deze content uitparsen. Indien toegang via browser, dan wordt het verwerkings-
              // resultaat getoond in de browser.
              if(ExecuteEvent)
                ExecuteLine(Event, Protocol);                                   // Voer binnengekomen event uit
              } // einde HTTP-request
            }
          else
            {
            // Er is geen geldig teken binnen gekomen. Even wachten en afbreken.
            delay(1000);
            Completed=true;
            }
          }
        }
      }
    IPClient.flush();// Verwijder eventuele rommel in de buffer.
    IPClient.stop();
    }
  free(TmpStr1);
  free(TmpStr2);
  free(InputBuffer_IP);
  free(Event);
  return;
  }  

#endif //ethernetserver_h
#endif //MEGA


/*********************************************************************************************\
 * Deze funktie berekend de CRC-8 checksum uit van een NodoEventStruct. 
 * Als de Checksum al correct gevuld was wordt er een true teruggegeven. Was dit niet het geval
 * dan wordt NodoEventStruct.Checksum gevuld met de juiste waarde en een false teruggegeven.
 \*********************************************************************************************/

boolean Checksum(NodoEventStruct *event)
  {
  byte OldChecksum=event->Checksum;
  byte NewChecksum=NODO_VERSION_MAJOR;  // Verwerk versie in checksum om communicatie tussen verschillende versies te voorkomen

  event->Checksum=0; // anders levert de beginsituatie een andere checksum op

  for(int x=0;x<sizeof(struct NodoEventStruct);x++)
    NewChecksum^(*((byte*)event+x)); 

  event->Checksum=NewChecksum;
  return(OldChecksum==NewChecksum);
  }

/**********************************************************************************************\
 * Deze functie wacht totdat de 433 en de IR band vrij zijn of er een timeout heeft plaats gevonden.
 * Er wordt gemeten hoeveel geldige pulsen ( > MIN_PULSE_LENGTH) er zich voordoen binnen een 
 * vast tijdsframe. Als er spikes zijn (=ruis) of er zijn geen pulsen, dan wordt teruggekeerd.
 \*********************************************************************************************/
void WaitFree(void)
  {
  unsigned long TimeOutTimer=millis()+WAIT_FREE_RX_TIMEOUT;                     // tijd waarna de routine wordt afgebroken in milliseconden
  unsigned long WindowTimer;

  int x,y;
  
  Led(BLUE);

  do
    {
    WindowTimer=millis()+250;
    x=0;
    while(WindowTimer>millis())
      {
      if((*portInputRegister(IRport)&IRbit)==0)                                 // Puls op RF?
      //if((*portInputRegister(IRport)&IRbit)==IRbit)                           // (gebruik bij 2e RF via IR pin)
        {
        x++;
        if(pulseIn(PIN_IR_RX_DATA,LOW) < MIN_PULSE_LENGTH && x>0)               // Was het een spike?
        //if(pulseIn(PIN_IR_RX_DATA,HIGH) < MIN_PULSE_LENGTH && x>0)            // Was het een spike?  (gebruik bij 2e RF via IR pin)
          x=0;
        }
      if((*portInputRegister(RFport)&RFbit)==RFbit)                             // Pulse op IR
        {
        y++;
        if(pulseIn(PIN_RF_RX_DATA,HIGH) < MIN_PULSE_LENGTH && y>0)              // Was het een spike?
          y=0;
        }
      }
    }while(TimeOutTimer>millis() && (x>0 || y>0));                              // Zolang nog pulsen op RF of IR en nog geen timeout

  Led(RED);
  }


  