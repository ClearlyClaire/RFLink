//#######################################################################################################
//#################################### Plugin-255: MyPlugin   ############################################
//#######################################################################################################

/*********************************************************************************************\
* Funktionele beschrijving: Dit is een leeg plugin dat als voorbeeld is opgenomen. Aan de hand
*                           van dit voorbeeld kan een gebruiker zelf een eigen plugin ontwikkelen.
*                           Als je plugin hebt ontwikkeld die voor anderen ook van nut kan zijn,
*                           meld deze dan aan bij het Nodo-team om deze te delen met andere gebruikers
*                           en om een Plugin-ID aan te vragen.
* 
* <Geef hier een beschrijving van de funktionele werking van het plugin en hoe deze door de gebruiker
* moet worden aangeroepen.>
*
* Auteur             : <naam en EMail adres van de ontwikkelaar>
* Support            : <website>
* Datum              : <datum>
* Versie             : <versie>
* Nodo productnummer : <Nodo productnummer. Toegekend door Nodo team>
* Compatibiliteit    : Vanaf Nodo build nummer <build nummer>
* Syntax             : "MijnPlugin <Par1>, <Par2>"
*
***********************************************************************************************
* Technische beschrijving:
*
* Compiled size      : <grootte> bytes voor een Mega en <grootte> voor een Small.
* Externe funkties   : <geef hier aan welke funkties worden gebruikt. 
*
* <Geef hier een technische toelichting op de werking van het plugin en eventueel gebruikte protocol>
* 
* Tips en aandachtspunten voor programmeren van een plugin:
* 
* -  Geheugen is beperkt. Programmeer compact en benut iedere byte RAM en PROGMEM. De Arduino heeft niet de luxe van een PC!
*    Het is verantwoordelijkheid van de programmeur om te bewaken. Vrij geheugen is opvraagbaar met [Status FreeMem].
* -  Bouw geen lange wachtlussen in de code. Dit kan leiden tot timings-promlemen waaronder missen van events.
*    Deze funktie bevindt zich ik een tijdkritische loop. Detecteer zo snel mogelijk of het ontvangen signaal
*    ook bij dit protocol hoort. 
* -  Om uitwisselbaar te blijven met andere Nodo versies en Nodo gebruikers, geen aanpassingen aan de Nodo code maken.
*    Let op dat je bij gebruik van functies uit de Nodo code je je eigen code gevoelig makt voor onderhoud
*    bij uitbrengen van nieuwe releases.
* -  Maak geen gebruik van interrupt driven routines, dit verstoort (mogelijk) de werking van de I2C, Serial en ethernet
*    communicatie.
* -  Maak slechts in uitzonderlijke gevallen gebruik van Globals en pas geen waarden van globals uit de Nodo code aan.
* -  Besteed uitgebreid aandacht aan de documentatie van het protocol. Indien mogelijk verwijzen naar originele
*    specificatie.
*
* Voorbeelden van plugins:
* - Digitale temperatuur sensoren (Zoals Dallas DS18B20)
* - Digitale vochtigheidssensoren (Zoals DTH-11)
* - Vergroten van aantal digitale wired met een multiplexer. Tot 8-ingangen per Wired poort met bv. een 74151.
* - Acht verschillende analoge ingangen meten met eén WiredIn met een LTC1380.
* - WiredOut uitgangen uitbreiden tot 8, 16, 32, 64 verschillende digitale uitgangen met behulp van 74HCT595
* - I2C plugins aansturen via de SLC en SDA lijnen van de arduino.
* - etcetera.
*
* De Wired poorten en de additionele IO poorten op de Mega in uw eigen code gebruiken aan de hand van de naam zoals deze zijn gedefinieerd
* in de code:
*
* PIN_WIRED_OUT_n staat voor WiredOut poort, waarbij n overeen komt met het nummer van de WiredOut poort te beginnen met 1.
* PIN_WIRED_IN_n staat voor WiredOut poort, waarbij n overeen komt met het nummer van de WiredOut poort te beginnen met 1.
* PIN_IO_n staat voor additionele IO-poort, waarbij n overeen komt met het nummer van deze poort, te beginnen met 1 (Alleen beschikbaar op de Mega).
* 
* Voor de verwijzing naar de Arduino pinnummers: zie schema of declaraties in tabblad "Base"
* ANDERE POORTEN NIET GEBRUIKEN OMDAT DEZE (IN DE TOEKOMST) EEN SPECIFIEKE FUNKTIE HEBBEN.
* Let bij het ontwerp van de Hardware en de software op dat geen gebruik wordt gemaakt van de Arduino pinnen die al een voorgedefinieerde
* Nodo funktie hebben, De WiredIn en WiredOut kunnen wel vrij worden gebruikt met die kanttekening dat Nodo commando's de lijnen eveneens 
* Besturen wat kan leiden tot beschadiging van hardware. Let hier in het bijzonder op als een Arduino pin wordt gedefinieerd als een output.
* Op de Nodo Mega zijn nog vier extra communicatielijnen die gebruikt kunnen worden voor User input/output: PIN_IO_1 t/m PIN_IO_4 (Arduino pin 38 t/m 41)
* Besef dat niet alle pennen van de Arduino gebruikt kunnen worden daar vele Arduino pinnen al een voorgedefinieerde
* Nodo funktie hebben, De WiredIn en WiredOut kunnen wel vrij worden gebruikt met die kanttekening dat Nodo commando's de lijnen eveneens 
* Besturen wat kan leiden tot beschadiging van hardware. Let hier in het bijzonder op als een Arduino pin wordt gedefinieerd als een output.
* Op de Nodo Mega zijn nog vier extra communicatielijnen die gebruikt kunnen worden voor User input/output: PIN_IO_1 t/m PIN_IO_4 (Arduino pin 38 t/m 41)
*
* Ben je klaar met het schrijven van je code, kleed dan de code zoveel mogelijk uit om geheugenruimte te
* besparen en stroomlijn op performance.
\*********************************************************************************************/
 

// Ieder plugin heeft een uniek ID. Deze ID's worden onderhouden door het Nodo team. Als je een plugin hebt geprogrammeerd
// die van waarde kan zijn voor andere gebruikers, meldt deze dan aan bij het Nodo team zodat deze kan worden meegenomen
// in de Nodo-releases. Plugin 255 is een "knutsel" plugin voor de gebruiker.

// Een plugin heeft naast een uniek ID ook een eigen MMI string die de gebruiker kan invoeren via Telnet, Serial, HTTP 
// of een script. Geef hier de naam op. De afhandeling is niet hoofdletter gevoelig. Voor pluginnamen geld de volgende conventie:
// voor een commando: ObjectActieOptie (ApparaatZend, ApparaatZendSnel)
// voor een event   : Object           (Apparaat)

#define PLUGIN_NAME "MyPlugin"

// Ieder plugin heeft een uniek ID. De reeks 250..255 zijn vrij te gebruiken. Alle andere ID's worden toegekend door
// het Nodo team. Zelf een nuttige plugin gemaakt, laat het ons weten!

#define PLUGIN_ID   255

// In de funktienaam zit het pluginnummer verwerkt zodat deze eenduidig kan worden geidentificeerd en aangeroepen. De Nodo
boolean Plugin_255(byte function, struct NodoEventStruct *event, char *string)
  {
  boolean success=false;

  // Deze plugin code wordt vanuit meerdere plaatsen in de Nodo code aangeroepen, steeds met een doel. Dit doel bevindt zich
  // in de variabele [function]. De volgende zijn gedefinieerd:
  //
  // PLUGIN_RAWSIGNAL_IN  => Afhandeling van een via RF/IR ontvangen event
  // PLUGIN_COMMAND       => Commando voor afhandelen/uitsturen van een event.
  // PLUGIN_MMI_IN        => Invoer van de gebruiker/script omzetten naar een event. (Alleen voor mega)
  // PLUGIN_MMI_OUT       => Omzetten van een event naar een voor de gebruiker leesbare tekst (Alleen voor Mega)
  // DEVIDE_ONCE_A_SECOND => ongeveer iedere seconde.
  // PLUGIN_INIT          => Eenmalig, direct na opstarten van de Nodo
  // PLUGIN_EVENT_IN      => Vlak voor verwerking van een binnengekomen event.
  // PLUGIN_SERIAL_IN     => Zodra er bytes via de seriele poort zijn ontvangen
  // PLUGIN_ETHERNET_IN   => Zodra er bytes via de seriele poort zijn ontvangen
  
  
  switch(function)
    {    
    #ifdef PLUGIN_255_CORE

    case PLUGIN_INIT:
      {
      // Code hier wordt eenmalig aangeroepen direct na een reboot van de Nodo en voordat
      // er enige event is verwerkt.
      // Heb je geen initialisatie taken, dan mag je deze case weglaten om geheugen te besparen.

      Serial.println(F("*** debug: MyPlugin: PLUGIN_INIT")); //??? Debug
      break;
      }

    case PLUGIN_EVENT_IN:
      {
      // Zodra er een event is binnengekomen wordt dit stuk code aangeroepen. Het mogelijk om zo 
      // een event te 'onderscheppen' en er een alternatieve verwerking plaats te laten vinden. 
      // Events kunnen afkomstig zijn van RF, IR, Serial, Wired, TelNet, HTTP, CLOCK, WIRED 
      // Er komen uitsluitend events langs deze code die van buiten de Nodo komen. Dus geen events die voortkomen   
      // uit verwerking van de eventlist.
      // Maak je geen gebruik van deze functie, dan bij voorkeur deze case geheel verwijderen.
      // Event bevat de gegevens die door de Nodo verwerkt zullen gaan worden. Deze gegevens kunnen worden
      // gebruikt en eventueel naar wens veranderd. De gewijzigde gegevens worden verwerkt als een regulier event.
      // Als event->Command wordt gevuld met 0, dan stopt de verwerking van het binnengekomen event.  
      Serial.println(F("*** debug: MyPlugin: PLUGIN_EVENT_IN"));
      break;      
      }

    case PLUGIN_ONCE_A_SECOND:
      {
      // Dit deel van de code wordt (ongeveer!) eenmaal per seconde aangeroepen. Let op dat deze code zich binnen een 
      // tijdkritisch deel van de hoofdloop van de Nodo bevindt! Gebruik dus alleen om snel een waarde te (re)setten
      // of om de status van een poort/variabele te checken. Zolang de verwerking zich hier plaats vindt zal de
      // Nodo géén IR of RF events kunnen ontvangen.
      // Heb je geen taken die periodiek uitgevoerd moeten worden, dan mag je deze case weglaten om geheugen te besparen.
      // Event bevat bij binnenkomst geen geldige gegevens. Deze gegevens kunnen wel worden gevuld. Als wordt 
      // teruggekeerd met success=true dan worden de gewijzigde gegevens verwerkt als een regulier event.

      // Serial.println(F("*** debug: MyPlugin: PLUGIN_ONCE_A_SECOND"));
      break;
      }

    case PLUGIN_RAWSIGNAL_IN:
      {
      // Code op deze plaats wordt uitgevoerd zodra er een event via RF of IR is binnengekomen
      // De RawSignal buffer is gevuld met pulsen. de struct [event] moet hier worden samengesteld.      
      // -  De array RawSignal.Pulses[] bevat alle Mark en Space tijden in microseconden. Te beginnen vanaf element [1].
      //    Deze array is reeds gevuld bij aankomst. Element [0] bevat een vermenigvuldigingsfactor voor omrekenen naar
      //    echte microseconden.
      // -  RawSignal.Pulses bevat het aantal posities die een mark/space bevatten. Let op dat de waarde RAW_BUFFER_SIZE
      //    nooit overschreden wordt. Anders gegarandeerd vastlopers! Positie [1] bevat de 'mark' van de startbit.  * -  De struct NodoEventStruct bevat alle informatie die nodig is voor verwerking en weergave van het event
      //    dat is ontvangen of moet worden weergegeven. 
      // Als decoderen is gelukt, dan de variabele [success] vullen met een true. De Nodo zal het 
      // event vervolgens als een regulier event afhandelen.
      // Als je het ontvangen signaal hebt gedecodeerd dan moet je vervolgens het Nodo event/commando
      // vullen zodat deze door de Nodo verder verwerkt kan worden. Kijk bij Plugin_001 als voorbeeld.
      // Maak je geen gebruik van deze functie, dan bij voorkeur deze case geheel verwijderen.
      Serial.println(F("*** debug: MyPlugin: PLUGIN_RAWSIGNAL_IN"));
      break;
      }
      
    case PLUGIN_COMMAND:
      {
      // Als er vanuit de gebruiker, script of eventlist dit plugin een event moet uitsturen, dan is op het
      // moment dat de code hier wordt aangeroepen, de struct [event] gevuld en gereed voor verwerking.
      // Als voor verlaten de struct [event] is gevuld met een ander event, dan wordt deze uitgevoerd als een nieuw
      // event. Dit kan bijvoorbeeld worden benut als een variabele wordt uitgelezen en de waarde verder verwerkt
      // moet worden.
      // Maak je geen gebruik van deze functie, dan bij voorkeur deze case geheel verwijderen.

      Serial.print(F("*** debug: MyPlugin: PLUGIN_COMMAND"));
      Serial.print(", Par1=");    Serial.print(event->Par1);
      Serial.print(", Par2=");    Serial.println(event->Par2);
      
      // Als voorbeeld wordt hier variabele 5 gevuld met 123.45
      byte Variable = 5;
      float Value   = 123.45;

      ClearEvent(event);                                      // Ga uit van een default schone event. Oude eventgegevens wissen.      
      event->Type         = NODO_TYPE_COMMAND;                // Het event is een uit te voeren commando
      event->Command      = CMD_VARIABLE_SET;                 // Commando "VariableSet"
      event->Par1         = Variable;                         // Par1 draagt het variabelenummer
      event->Par2         = float2ul(Value);                  // Par2 de waarde. float2ul() zet de waarde m naar een geschikt format.
      
      success=true;                                           // Als verlaten wordt met een true, en er is een nieuw event in de struct geplaatst
                                                              // dan wordt deze automatisch uitgevoerd.
                                                              // Een plugin kan geen andere plugin aanroepen.
                                                              // verder is er geen beperking en kunnen alle events/commando's worden
                                                              // opgegeven voor verdere verwerking.
      break;
      }      

    case PLUGIN_SERIAL_IN:
      {
      // Zodra er een teken op de Serial poort binnenkomt wordt dit deel van de plugin aangeroepen.
      // Deze funktie kan worden gebruikt voor speciale afhandeling van seriele informatie.
      // Let hierbij wel op dat de tekens slechts beschikbaar zijn en nog niet uitgelezen.
      // Bij aankomst hebben zowel Event als string een ongeldige waarde en mogen dus niet worden gebruikt.  
      // Zorg zelf voor binnenhalen van de tekens en verwerking van de gegevens. De binnengekomen input
      // wordt niet meer door de Nodo verwerkt zodra deze hier binnen de plugin is opgehaald.
      // Deze funktie werkt zowel op een Small als een Mega, let echter op dat de MMI voor een Mega 
      // via Serial niet meer werkt omdat alle input hier wordt afgevangen.
      // Maak je geen gebruik van deze functie, dan bij voorkeur deze case geheel verwijderen.
      // Vanuit PLUGIN_INIT kan eventueeel de baudrate (opnieuw) worden ingesteld. (Default 19200)
      // Er kan maar -1- plugin gelijktijdig gebruik van deze plugin-case gebruik maken!

      Serial.println(F("*** debug: MyPlugin: PLUGIN_SERIAL_IN"));

      // Met onderstaande regels kan de input worden opgehaald. In dit voorbeeld weggelaten omdat
      // zoals boven beschreven de MMI van de mega anders niet meer werkt. 
      // 
      // while(Serial.available())
      //   {
      //   Serial.print(F("Received="));
      //   Serial.println(Serial.read());
      //   }                

      break;      
      }
    #endif // CORE
    
    #if NODO_MEGA // alleen relevant voor een Nodo Mega want de Small heeft geen MMI!
    case PLUGIN_MMI_IN:
      {
      Serial.print(F("*** debug: MyPlugin: PLUGIN_MMI_IN, string="));Serial.println(string); //??? Debug
      // Zodra er via een script, HTTP, Telnet of Serial een commando wordt ingevoerd, wordt dit deel van de code langs gelopen.
      // Op deze plek kan de invoer [string] worden geparsed en omgezet naar een struct [event]. Als parsen van de invoerstring [string]
      // is gelukt en de struct is gevuld, dan de variabele [success] vullen met true zodat de Nodo zorg kan dragen voor verdere verwerking van het event.

      char *TempStr=(char*)malloc(INPUT_COMMAND_SIZE);

      // Met PLUGIN_MMI_IN wordt de invoer van de gebruiker (string) omgezet naar een event zodat de Nodo deze intern kan verwerken.
      // Hier aangekomen bevat string het volledige commando. Test als eerste of het opgegeven commando overeen komt met PLUGIN_NAME
      // Dit is het eerste argument in het commando. 
      if(GetArgv(string,TempStr,1))
        {
        // Als het door de gebruiker ingegeven ommando/event overeenkomt met de naam van dit plugin...
        if(strcasecmp(TempStr,PLUGIN_NAME)==0)
          {
          // in dit voorbeeld even laten weten dat de code wordt geraakt. Directe output naar
          // Serial is normaal gesproken NIET wenselijk in een plugin. 

          // Vervolgens tweede parameter gebruiken
          if(GetArgv(string,TempStr,2)) 
            {
            // plaats hier je code die de eerste parameter die zich in [TempStr] bevindt verder uitparsed
            // De byte Par1 en unsigned long Par2 die zich in de struct [event] bevindt kunnen worden gebruikt.
              
            if(GetArgv(string,TempStr,3))
              {
              // indien gewenst kan een tweede parameter worden gebruikt (=derde argument)
              // Plaats wederom de code paar uitparsen van de parameter hier.
              // heb je het laatste parameter geparsen, dan de variabele [success] vullen 
              // met een true zodat verdere verwerking van het event plaats kan vinden.

              // Een plugin kan worden verwerkt als een commando of een event. Geef dit aan want 
              // op moment van invoer moet de Nodo t.b.v. latere verwerking weten hoe de zaken afgehandeld moeten worden
              event->Type    = NODO_TYPE_PLUGIN_COMMAND;
              event->Command = 255; // nummer van dit plugin
              
              // Als success wordt gevuld met true, dan wordt het commando/event
              // geaccepteerd als geldig.
              
              success=true;
              }
            }
          }
        }
      free(TempStr);
      break;
      }

    case PLUGIN_MMI_OUT:
      {
      // De code die zich hier bevindt zorgt er voor dan een event met het unieke ID in de struct [event] weer wordt
      // omgezet naar een leesbaar event voor de gebruiker. het resultaat moet worden geplaatst in de string [string]
      // let op dat het totale commando niet meer dan 25 posities in beslag neemt.
      // Dit deel van de code wordt alleen uitgevoerd door een Nodo Mega, omdat alleen deze over een MMI beschikt.
      Serial.println(F("*** debug: MyPlugin: PLUGIN_MMI_OUT")); //??? Debug

      strcpy(string,PLUGIN_NAME);               // Commando 
      strcat(string," ");
      strcat(string,int2str(event->Par1));      // Parameter-1 (8-bit)
      strcat(string,",");
      strcat(string,int2str(event->Par2));      // Parameter-2 (32-bit)

      break;
      }
    #endif //MMI
    }
    
  return success;
  }

