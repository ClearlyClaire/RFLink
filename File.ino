
#if NODO_MEGA

/*********************************************************************************************\
 * Op het Ethernetshield kunnen de W5100 chip en de SDCard niet gelijktijdig worden gebruikt
 * Deze funktie zorgt voor de juiste chipselect. Default wordt in de Nodo software uitgegaan dat
 * de ethernetchip W5100 is geselecteerd. Met deze routine kan de SDCard worden geselecteerd.
 * input: true = SD_Card geselecteerd.
 \*********************************************************************************************/
void SelectSDCard(boolean sd)
  {
  digitalWrite(EthernetShield_CS_W5100, sd);
  digitalWrite(EthernetShield_CS_SDCard, !sd);
  }

/**********************************************************************************************\
 * Wis een file
 \*********************************************************************************************/
void SDCardInit(void)
  {
  SelectSDCard(true);
  if(SD.begin(EthernetShield_CS_SDCard))
    {
    SD.mkdir(ProgmemString(Text_08));// RAWSIGN
    SD.mkdir(ProgmemString(Text_11));// ALIAS_I 
    SD.mkdir(ProgmemString(Text_12));// ALIAS_O
    bitWrite(HW_Config,HW_SDCARD,1);
    }
  SelectSDCard(false);
  }

 /*********************************************************************************************\
 * Wis een file
 \*********************************************************************************************/
void FileErase(char* Path, char* Filename, char* Extention)
  {
  SelectSDCard(true);
  
  // Serial.print("FileErase(); = ");Serial.println(PathFile(Path, Filename, Extention));

  if(strcmp(Filename,"*")==0)// Alles wissen
    {
    File root;
    File entry;
    char *TempString=(char*)malloc(INPUT_COMMAND_SIZE);
  
    SelectSDCard(true);
    if(root=SD.open(Path))
      {
      root.rewindDirectory();
      while(entry = root.openNextFile())
        {
        if(!entry.isDirectory())
          {
          TempString[0]=0;
          strcpy(TempString,Path);
          strcat(TempString,"/");
          strcat(TempString,entry.name());
          SD.remove(TempString);
          }
        entry.close();
        }
      root.close();
      }
    free(TempString);  
    }
  else
    {
    SD.remove(PathFile(Path, Filename, Extention));
    }
  SelectSDCard(false);
  }

 /*********************************************************************************************\
 * Eenvoudige funktie om pad en bestandsnaam aan elkaar te plakken. Scheelt elders code. 
 \*********************************************************************************************/
char* PathFile(char* Path, char* File, char* Extention)
  {
  static char PathBuffer[INPUT_COMMAND_SIZE];  
  if(strlen(Path)<(INPUT_COMMAND_SIZE-13) && strlen(File)<=8)
    {
    sprintf(PathBuffer,"%s/%s.%s",Path, File, Extention);
    }
  else
    PathBuffer[0]=0;
  
  return PathBuffer;
  }

 /*********************************************************************************************\
 * 
 \*********************************************************************************************/
byte FileWriteLine(char* Path, char* Filename, char* Extention, char *Line, boolean Delete)
  {
  byte error=0;

  // Serial.print("FileWriteLine=");Serial.println(PathFile(Path,Filename,Extention));

  SelectSDCard(true);

  if(Delete)
    SD.remove(PathFile(Path, Filename, Extention));

  if(Line!=0 && Line[0]!=0)
    {
    File LogFile = SD.open(PathFile(Path, Filename, Extention), FILE_WRITE);
    if(LogFile)
      {
      for(int x=0;x<strlen(Line);x++)
        if(isprint(Line[x]))
          LogFile.write(Line[x]);
          
      LogFile.write('\n'); // nieuwe regel
      LogFile.close();
      }
    else
      {
      error=MESSAGE_UNABLE_OPEN_FILE;
      }
    }
      
  SelectSDCard(false);
  return error;
  }

/**********************************************************************************************\
 * Schrijf Eventlist naar SDCard.
 \*********************************************************************************************/
boolean SaveEventlistSDCard(char* Path, char* Filename, char* Extention)
  {
  int x;
  byte TempAliasSetting=Settings.Alias;
  boolean r=true;
  char *TempString=(char*)malloc(INPUT_LINE_SIZE);


  // Aliassen uitschakelen omdat het schrijven naar de EventlistFile en daarbinnen op SDCard naar Aliassen
  // zoeken niet samen gaat. Hier verslikt de SD library zich in. 
  Settings.Alias=VALUE_OFF;

  // SDCard en de W5100 kunnen niet gelijktijdig werken. Selecteer SDCard chip
  SelectSDCard(true);

  SD.remove(PathFile(Path, Filename, Extention)); // eerst bestand wissen, anders wordt de data toegevoegd

  File EventlistFile = SD.open(PathFile(Path, Filename, Extention), FILE_WRITE);

  if(EventlistFile) 
    {
    EventlistFile.println(cmd2str(CMD_EVENTLIST_ERASE));

    x=1;
    while(EventlistEntry2str(x++,0,TempString,true))
      {
      if(TempString[0]!=0)// als de string niet leeg is, dan de regel weg schrijven
        {
        EventlistFile.write((uint8_t*)TempString,strlen(TempString));      
        EventlistFile.write('\n'); // nieuwe regel
        }
      }
    EventlistFile.close();
    }
  else 
    {
    r=false; // niet meer weer proberen weg te schrijven.
    }

  SelectSDCard(false);

  free(TempString);
  Settings.Alias=TempAliasSetting;
  
  return r;
  }


boolean FileList(char *Path, byte Port)
  {
  byte error=0;
  File root;
  File entry;
  char *TempString=(char*)malloc(INPUT_COMMAND_SIZE);

  SelectSDCard(true);
  if(root = SD.open(Path))
    {
    SelectSDCard(false);
    PrintString(ProgmemString(Text_22), Port);
    SelectSDCard(true);
  
    root.rewindDirectory();
    while(entry = root.openNextFile())
      {
      if(!entry.isDirectory())
        {
        TempString[0]=0;
        // Als de funktie is aangeroepen vanuit RawSignalList, dan voor de bestandnamen 0x plakken omdat de bestandsnamen een 
        // hexadecimale waarde representeren. Niet netjes op deze wijze maar bespaart code. 
        if(strcasecmp(Path,ProgmemString(Text_08))==0)
          strcat(TempString,"0x");
        strcat(TempString,entry.name());
        TempString[StringFind(TempString,".")]=0;
        SelectSDCard(false);
        PrintString(TempString,Port);
        SelectSDCard(true);
        }
      entry.close();
      }
    root.close();

    SelectSDCard(false);
    PrintString(ProgmemString(Text_22),Port);
    }
  else
    error=MESSAGE_SDCARD_ERROR;
    
  free(TempString);  
  return error;
  }


byte FileExecute(char* Path, char* Filename, char* Extention, boolean ContinueOnError, byte PrintPort)
  {
  int x,y;
  byte error=0;
  static byte FileExecuteNesting=0;                                             // ter voorkoming nesting van fileexecute      
  char *TmpStr=(char*)malloc(INPUT_LINE_SIZE);

  if(++FileExecuteNesting>3)
    {
    RaiseMessage(MESSAGE_NESTING_ERROR,0);
    }
  else
    {
    Led(RED);
    
    SelectSDCard(true);
    File dataFile=SD.open(PathFile(Path, Filename, Extention));
    
    if(dataFile) 
      {
      y=0;       
      while(dataFile.available() && !error)
        {
        x=dataFile.read();
        if(isprint(x) && y<(INPUT_LINE_SIZE-1))
          TmpStr[y++]=x;
        else
          {
          TmpStr[y]=0;
          y=0;
          SelectSDCard(false);
          if(PrintPort)
            PrintString(TmpStr, PrintPort);
        
          error=ExecuteLine(TmpStr,VALUE_SOURCE_FILE);
          SelectSDCard(true);
  
          if(ContinueOnError)
            error=0;
  
          if(error)
            break;
          }
        }
      dataFile.close();
      }  
    else
      error=MESSAGE_UNABLE_OPEN_FILE;
    }

  FileExecuteNesting=false;
  free(TmpStr);    
  SelectSDCard(false);

  FileExecuteNesting--;

  if(error)                                                                     // Als een error tijdens uitvoer script, dan moet een eventuele SendTo worden uitgezet.
    {
    Transmission_SendToUnit=0;                                                  // Unitnummer waar de events naar toe gestuurd worden. 0=alle.
    Transmission_SendToAll=0;                                                   // Waarde die aangeeft of het SendTo permanent staat ingeschakeld. 0=uit, in andere gevallen het unitnummer.
    }
    
  return error;
  }    


byte FileShow(char* Path, char* Filename, char* Extention, byte Port)
  {
  char *TmpStr2=(char*)malloc(INPUT_LINE_SIZE);
  byte error=0;
  
  SelectSDCard(true);

  File dataFile=SD.open(PathFile(Path, Filename, Extention));
  if(dataFile) 
    {
    // Als de file groter is dan 10K, dan alleen laatste stuk laten zien
    unsigned long a=dataFile.size();
    if(a>100000UL)
      {
      unsigned long w=dataFile.seek(a-100000UL);                    
      while(dataFile.available() && isprint(dataFile.read()));
      SelectSDCard(false);
      PrintString(ProgmemString(Text_09),Port);
      SelectSDCard(true);
      }

    TmpStr2[0]=0;
    int y=0;       
    while(dataFile.available())
      {
      byte x=dataFile.read();
      if(isprint(x) && y<(INPUT_LINE_SIZE-1))
        {
        TmpStr2[y++]=x;
        }
      else
        {
        TmpStr2[y]=0;
        y=0;
        SelectSDCard(false);
        PrintString(TmpStr2,Port);
        SelectSDCard(true);
        }
      }
    dataFile.close();
    }  
  else
    error=MESSAGE_UNABLE_OPEN_FILE;

  SelectSDCard(false);
  free(TmpStr2);
  return error;
  }

#endif
