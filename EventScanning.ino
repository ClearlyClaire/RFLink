#define I2C_BUFFERSIZE                32                                        

#if I2C
int  I2C_Received=0;                                                            // Bevat aantal binnengomen bytes op I2C;
byte I2C_ReceiveBuffer[I2C_BUFFERSIZE+1];
#endif

unsigned long RepeatingTimer=0L;
unsigned long EventHashPrevious=0,SignalHash,SignalHashPrevious=0L;

boolean ScanEvent(struct NodoEventStruct *Event)                                // Deze routine maakt deel uit van de hoofdloop en wordt iedere 125uSec. doorlopen
  {
  byte Fetched=0;
  static byte Focus=0;
  static boolean BlockRepeatsStatus=false;
  unsigned long Timer=millis()+SCAN_HIGH_TIME;

  while(Timer>millis() || RepeatingTimer>millis())
    {
    #if I2C
    if(Focus==0 || Focus==VALUE_SOURCE_I2C)
      {
      if(I2C_Received)                                                          // I2C: *************** kijk of er data is binnengekomen op de I2C-bus **********************
        {
        if(I2C_Received==sizeof(struct NodoEventStruct))                        // Er is I2C data binnengekomen maar weten nog niet of het een NodoEventStruct betreft.
          {                                                                     // Het is een NodoEventStruct
          memcpy(Event, &I2C_ReceiveBuffer, sizeof(struct NodoEventStruct));
          if(Checksum(Event))
            {   
            bitWrite(HW_Config,HW_I2C,true);
            I2C_Received=0;
            Fetched=VALUE_SOURCE_I2C;
            Focus=Fetched;
            }
          }
        else
          {                                                                     // Het is geen NodoEventStruct. In dit geval verwerken als reguliere commandline string.
          PluginCall(PLUGIN_I2C_IN,0,0);
          #if NODO_MEGA
          I2C_ReceiveBuffer[I2C_Received]=0;                                    // Sluit buffer af als een string.
    
          for(int q=0;q<I2C_Received;q++)                                       // In een commando bevinden zich geen bijzondere tekens. Is dit wel het geval, dan string leeg maken.
           if(!isprint(I2C_ReceiveBuffer[q]))
             I2C_ReceiveBuffer[0]=0;
            
          if(I2C_ReceiveBuffer[0]!=0)                                           // Is er een geldige string binnen, dan verwerken als commandline.
            ExecuteLine((char*)&I2C_ReceiveBuffer[0],VALUE_SOURCE_I2C);  
          #endif
    
          Fetched=0;
          I2C_Received=0;
          }
        }
      }
    #endif
  
    if(Focus==0 || Focus==VALUE_SOURCE_IR)
      {
      if(FetchSignal(PIN_IR_RX_DATA,LOW))                                       // IR: *************** kijk of er data start **********************
      //if(FetchSignal(PIN_IR_RX_DATA,HIGH))                                    // IR: *************** kijk of er data start ********************** (Bij gebruik 2e RF via IR pin)
        {
        if(AnalyzeRawSignal(Event))
          {
          bitWrite(HW_Config,HW_IR_RX,true);
          Fetched=VALUE_SOURCE_IR;
          Focus=Fetched;
          }
        }
      }
      
    if(Focus==0 || Focus==VALUE_SOURCE_RF)
      {
      if(FetchSignal(PIN_RF_RX_DATA,HIGH))                                      // RF: *************** kijk of er data start **********************
        {
        if(AnalyzeRawSignal(Event))
          {
          bitWrite(HW_Config,HW_RF_RX,true);
          Fetched=VALUE_SOURCE_RF;
          Focus=Fetched;
          }
        }
      }

    if(Fetched)
      {
      HoldTransmission=DELAY_BETWEEN_TRANSMISSIONS+millis();
      SignalHash=(Event->Command<<24 | Event->Type<<16 | Event->Par1<<8) ^ Event->Par2;
      Event->Port=Fetched;
      Event->Direction=VALUE_DIRECTION_INPUT;
      Fetched=0;
      
      if(RawSignal.RepeatChecksum)RawSignal.Repeats=true;
            
      // Er zijn een aantal situaties die moeten leiden te een event. Echter er zijn er ook die (nog) niet mogen leiden 
      // tot een event en waar het binnengekomen signaal moet worden onderdrukt.
      
      // 1. Het is een (niet repeterend) Nodo signaal of is de herkomst I2C => Alle gevallen doorlaten
      if(Event->Type==NODO_TYPE_EVENT || Event->Type==NODO_TYPE_COMMAND || Event->Type==NODO_TYPE_SYSTEM || Event->Port==VALUE_SOURCE_I2C)
        Fetched=1;      

      // 2. Het (mogelijk repeterend) binnenkomende signaal is niet recent eerder binnengekomen, zoals plugin signalen als KAKU, NewKAKU, ... => Herhalingen onderdrukken  
      else if(!RawSignal.RepeatChecksum && (SignalHash!=SignalHashPrevious || RepeatingTimer<millis())) 
        Fetched=2;

      // 3. Het is een herhalend signaal waarbij een herhaling wordt gebruikt als checksum zoals RAwSignals => Pas na twee gelijke signalen een event.
      else if(RawSignal.RepeatChecksum && SignalHash==SignalHashPrevious && (SignalHash!=EventHashPrevious || RepeatingTimer<millis())) 
        Fetched=3;

      // Serial.print(F("DEBUG: Fetched. SignalHash="));Serial.print(SignalHash,HEX);Serial.print(F(", RawSignal.Repeats="));Serial.print(RawSignal.Repeats);Serial.print(F(", RawSignal.RepeatChecksum="));Serial.print(RawSignal.RepeatChecksum);Serial.print(F(", RepeatingTimer>millis()="));Serial.print(RepeatingTimer>millis());Serial.print(F(", Fetched="));Serial.println(Fetched);

      SignalHashPrevious=SignalHash;
      RepeatingTimer=millis()+SIGNAL_REPEAT_TIME;
      
      if(Fetched)
        {
        // Nodo event: Toets of de versienummers corresponderen. Is dit niet het geval, dan zullen er verwerkingsfouten optreden! Dan een waarschuwing tonen en geen verdere verwerking.
        // Er is een uitzondering: De eerste commando/eventnummers zijn stabiel en mogen van oudere versienummers zijn.
        if(Event->Version!=0 && Event->Version!=NODO_VERSION_MINOR && Event->Command>COMMAND_MAX_FIXED)
          {
          #if NODO_MEGA
          Event->Command=CMD_DUMMY;
          Event->Type=NODO_TYPE_EVENT;
          Event->Par1=0;
          Event->Par2=0;
          PrintEvent(Event,VALUE_ALL);
          RaiseMessage(MESSAGE_VERSION_ERROR,Event->Version);
          #endif
          return false;
          }     
    
        // als het informatie uitwisseling tussen Nodo's betreft...
        if(Event->Type==NODO_TYPE_EVENT || Event->Type==NODO_TYPE_COMMAND || Event->Type==NODO_TYPE_SYSTEM)
          {
          // registreer welke Nodo's op welke poort zitten en actualiseer tabel.
          // Wordt gebruikt voor SendTo en I2C communicatie op de Mega.
          // Hiermee kan later automatisch de juiste poort worden geselecteerd met de SendTo en kan in
          // geval van I2C communicatie uitsluitend naar de I2C verbonden Nodo's worden gecommuniceerd.
          NodoOnline(Event->SourceUnit,Event->Port);

          // Een Nodo kan aangeven dat hij Busy is.
          bitWrite(BusyNodo, Event->SourceUnit,(Event->Flags&TRANSMISSION_BUSY)>0);
          }
                  
        // Als het Nodo event voor deze unit bestemd is, dan klaar. Zo niet, dan terugkeren met een false
        // zodat er geen verdere verwerking plaatsvindt.
        if(Event->DestinationUnit==0 || Event->DestinationUnit==Settings.Unit)
          {
          EventHashPrevious=SignalHash;
          // PrintNodoEvent("DEBUG: ScanEvent(): Fetched", Event);
          return true;
          }
        }
      }
    }// while
  Focus=0;
  return false;
  }

#if NODO_MEGA
#if CLOCK
boolean ScanAlarm(struct NodoEventStruct *Event)
  {
  unsigned long Mask;
  
  for(byte x=0;x<ALARM_MAX;x++)
    {
    if((Settings.Alarm[x]>>20)&1)                                               // Als alarm enabled is, dan ingestelde alarmtijd vergelijke met de echte tijd.
      {
      // stel een vergelijkingswaarde op
      unsigned long Cmp=Time.Minutes%10 | (unsigned long)(Time.Minutes/10)<<4 | (unsigned long)(Time.Hour%10)<<8 | (unsigned long)(Time.Hour/10)<<12 | (unsigned long)Time.Day<<16 | 1UL<<20;

                                                                                // In het ingestelde alarm kunnen zich wildcards bevinden. 
                                                                                // Maskeer de posities met 0xF wildcard nibble. 
                                                                                // Loop de acht nibbles van de 32-bit Par2 langs        
      for(byte y=0;y<8;y++)
        {          
        if(((Settings.Alarm[x]>>(y*4))&0xf) == 0xf)                             // als in nibble y een wildcard waarde 0xf staat
          {
          Mask=0xffffffff  ^ (0xFUL <<(y*4));                                   // Mask maken om de nibble positie y te wissen.
          Cmp&=Mask;                                                            // Maak nibble leeg
          Cmp|=(0xFUL<<(y*4));                                                  // vul met wildcard waarde 0xf
          }
        }
      
     if(Settings.Alarm[x]==Cmp)                                                 // Als ingestelde alarmtijd overeen komt met huidige tijd.
       {
       if(AlarmPrevious[x]!=Time.Minutes)                                       // Als alarm niet eerder is afgegaan
         {
         AlarmPrevious[x]=Time.Minutes;

         ClearEvent(Event);
         Event->Direction=VALUE_DIRECTION_INPUT;
         Event->Port=VALUE_SOURCE_CLOCK;
         Event->Type=NODO_TYPE_EVENT;
         Event->Command=EVENT_ALARM;
         Event->Par1=x+1;
         return true;
         }
       }
     else
       AlarmPrevious[x]=0xFF;
     }
   }
 return false; 
 }
#endif clock
#endif
