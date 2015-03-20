
/****************************************************************************************************************************\
* In dit tabblad staan voorzieningen die nodig zijn om de plugins te integreren in de Nodo code op een zodanige manier dat
* uitsluitend DIE plugins worden meegecompileerd die ook daadwerklijk door de gebruiker zijn opgegeven. Niet opgegeven
* plugins worden dus niet meegecompileerd om geheugen te besparen. Dit tabblad bevat geen code die door de gebruiker 
* aangepast hoeft te worden.
*
* Devics worden uniek genummerd. Deze nummers worden uitgegeven door het Nodo team. Plugin_255 is een template voor 
* een plugin dat kan worden gebruikt als je zelf een plugin wilt toevoegen aan de Nodo. 
\*************************************************************************************************************************/

#define PLUGINFILE2(a, b) stringify(a/Plugins/b)
#define PLUGINFILE(a, b) PLUGINFILE2(a, b)

#ifdef PLUGIN_001
#include PLUGINFILE(SKETCH_PATH,Plugin_001.c)
#endif

#ifdef PLUGIN_002
#include PLUGINFILE(SKETCH_PATH,Plugin_002.c)
#endif

#ifdef PLUGIN_003
#include PLUGINFILE(SKETCH_PATH,Plugin_003.c)
#endif

#ifdef PLUGIN_004
#include PLUGINFILE(SKETCH_PATH,Plugin_004.c)
#endif

#ifdef PLUGIN_005
#include PLUGINFILE(SKETCH_PATH,Plugin_005.c)
#endif

#ifdef PLUGIN_006
#include PLUGINFILE(SKETCH_PATH,Plugin_006.c)
#endif

#ifdef PLUGIN_007
#include PLUGINFILE(SKETCH_PATH,Plugin_007.c)
#endif

#ifdef PLUGIN_008
#include PLUGINFILE(SKETCH_PATH,Plugin_008.c)
#endif

#ifdef PLUGIN_009
#include PLUGINFILE(SKETCH_PATH,Plugin_009.c)
#endif

#ifdef PLUGIN_010
#include PLUGINFILE(SKETCH_PATH,Plugin_010.c)
#endif

#ifdef PLUGIN_011
#include PLUGINFILE(SKETCH_PATH,Plugin_011.c)
#endif

#ifdef PLUGIN_012
#include PLUGINFILE(SKETCH_PATH,Plugin_012.c)
#endif

#ifdef PLUGIN_013
#include PLUGINFILE(SKETCH_PATH,Plugin_013.c)
#endif

#ifdef PLUGIN_014
#include PLUGINFILE(SKETCH_PATH,Plugin_014.c)
#endif

#ifdef PLUGIN_015
#include PLUGINFILE(SKETCH_PATH,Plugin_015.c)
#endif

#ifdef PLUGIN_016
#include PLUGINFILE(SKETCH_PATH,Plugin_016.c)
#endif

#ifdef PLUGIN_017
#include PLUGINFILE(SKETCH_PATH,Plugin_017.c)
#endif

#ifdef PLUGIN_018
#include PLUGINFILE(SKETCH_PATH,Plugin_018.c)
#endif

#ifdef PLUGIN_019
#include PLUGINFILE(SKETCH_PATH,Plugin_019.c)
#endif

#ifdef PLUGIN_020
#include PLUGINFILE(SKETCH_PATH,Plugin_020.c)
#endif

#ifdef PLUGIN_021
#include PLUGINFILE(SKETCH_PATH,Plugin_021.c)
#endif

#ifdef PLUGIN_022
#include PLUGINFILE(SKETCH_PATH,Plugin_022.c)
#endif

#ifdef PLUGIN_023
#include PLUGINFILE(SKETCH_PATH,Plugin_023.c)
#endif

#ifdef PLUGIN_024
#include PLUGINFILE(SKETCH_PATH,Plugin_024.c)
#endif

#ifdef PLUGIN_025
#include PLUGINFILE(SKETCH_PATH,Plugin_025.c)
#endif

#ifdef PLUGIN_026
#include PLUGINFILE(SKETCH_PATH,Plugin_026.c)
#endif

#ifdef PLUGIN_027
#include PLUGINFILE(SKETCH_PATH,Plugin_027.c)
#endif

#ifdef PLUGIN_028
#include PLUGINFILE(SKETCH_PATH,Plugin_028.c)
#endif

#ifdef PLUGIN_029
#include PLUGINFILE(SKETCH_PATH,Plugin_029.c)
#endif

#ifdef PLUGIN_030
#include PLUGINFILE(SKETCH_PATH,Plugin_030.c)
#endif

#ifdef PLUGIN_031
#include PLUGINFILE(SKETCH_PATH,Plugin_031.c)
#endif

#ifdef PLUGIN_032
#include PLUGINFILE(SKETCH_PATH,Plugin_032.c)
#endif

#ifdef PLUGIN_033
#include PLUGINFILE(SKETCH_PATH,Plugin_033.c)
#endif

#ifdef PLUGIN_034
#include PLUGINFILE(SKETCH_PATH,Plugin_034.c)
#endif

#ifdef PLUGIN_035
#include PLUGINFILE(SKETCH_PATH,Plugin_035.c)
#endif

#ifdef PLUGIN_036
#include PLUGINFILE(SKETCH_PATH,Plugin_036.c)
#endif

#ifdef PLUGIN_037
#include PLUGINFILE(SKETCH_PATH,Plugin_037.c)
#endif

#ifdef PLUGIN_038
#include PLUGINFILE(SKETCH_PATH,Plugin_038.c)
#endif

#ifdef PLUGIN_039
#include PLUGINFILE(SKETCH_PATH,Plugin_039.c)
#endif

#ifdef PLUGIN_040
#include PLUGINFILE(SKETCH_PATH,Plugin_040.c)
#endif

#ifdef PLUGIN_041
#include PLUGINFILE(SKETCH_PATH,Plugin_041.c)
#endif

#ifdef PLUGIN_042
#include PLUGINFILE(SKETCH_PATH,Plugin_042.c)
#endif

#ifdef PLUGIN_043
#include PLUGINFILE(SKETCH_PATH,Plugin_043.c)
#endif

#ifdef PLUGIN_044
#include PLUGINFILE(SKETCH_PATH,Plugin_044.c)
#endif

#ifdef PLUGIN_045
#include PLUGINFILE(SKETCH_PATH,Plugin_045.c)
#endif

#ifdef PLUGIN_046
#include PLUGINFILE(SKETCH_PATH,Plugin_046.c)
#endif

#ifdef PLUGIN_047
#include PLUGINFILE(SKETCH_PATH,Plugin_047.c)
#endif

#ifdef PLUGIN_048
#include PLUGINFILE(SKETCH_PATH,Plugin_048.c)
#endif

#ifdef PLUGIN_049
#include PLUGINFILE(SKETCH_PATH,Plugin_049.c)
#endif

#ifdef PLUGIN_050
#include PLUGINFILE(SKETCH_PATH,Plugin_050.c)
#endif

#ifdef PLUGIN_051
#include PLUGINFILE(SKETCH_PATH,Plugin_051.c)
#endif

#ifdef PLUGIN_052
#include PLUGINFILE(SKETCH_PATH,Plugin_052.c)
#endif

#ifdef PLUGIN_053
#include PLUGINFILE(SKETCH_PATH,Plugin_053.c)
#endif

#ifdef PLUGIN_054
#include PLUGINFILE(SKETCH_PATH,Plugin_054.c)
#endif

#ifdef PLUGIN_055
#include PLUGINFILE(SKETCH_PATH,Plugin_055.c)
#endif

#ifdef PLUGIN_056
#include PLUGINFILE(SKETCH_PATH,Plugin_056.c)
#endif

#ifdef PLUGIN_057
#include PLUGINFILE(SKETCH_PATH,Plugin_057.c)
#endif

#ifdef PLUGIN_058
#include PLUGINFILE(SKETCH_PATH,Plugin_058.c)
#endif

#ifdef PLUGIN_059
#include PLUGINFILE(SKETCH_PATH,Plugin_059.c)
#endif

#ifdef PLUGIN_060
#include PLUGINFILE(SKETCH_PATH,Plugin_060.c)
#endif

#ifdef PLUGIN_061
#include PLUGINFILE(SKETCH_PATH,Plugin_061.c)
#endif

#ifdef PLUGIN_062
#include PLUGINFILE(SKETCH_PATH,Plugin_062.c)
#endif

#ifdef PLUGIN_063
#include PLUGINFILE(SKETCH_PATH,Plugin_063.c)
#endif

#ifdef PLUGIN_064
#include PLUGINFILE(SKETCH_PATH,Plugin_064.c)
#endif

#ifdef PLUGIN_065
#include PLUGINFILE(SKETCH_PATH,Plugin_065.c)
#endif

#ifdef PLUGIN_066
#include PLUGINFILE(SKETCH_PATH,Plugin_066.c)
#endif

#ifdef PLUGIN_067
#include PLUGINFILE(SKETCH_PATH,Plugin_067.c)
#endif

#ifdef PLUGIN_068
#include PLUGINFILE(SKETCH_PATH,Plugin_068.c)
#endif

#ifdef PLUGIN_069
#include PLUGINFILE(SKETCH_PATH,Plugin_069.c)
#endif

#ifdef PLUGIN_070
#include PLUGINFILE(SKETCH_PATH,Plugin_070.c)
#endif

#ifdef PLUGIN_071
#include PLUGINFILE(SKETCH_PATH,Plugin_071.c)
#endif

#ifdef PLUGIN_072
#include PLUGINFILE(SKETCH_PATH,Plugin_072.c)
#endif

#ifdef PLUGIN_073
#include PLUGINFILE(SKETCH_PATH,Plugin_073.c)
#endif

#ifdef PLUGIN_074
#include PLUGINFILE(SKETCH_PATH,Plugin_074.c)
#endif

#ifdef PLUGIN_075
#include PLUGINFILE(SKETCH_PATH,Plugin_075.c)
#endif

#ifdef PLUGIN_076
#include PLUGINFILE(SKETCH_PATH,Plugin_076.c)
#endif

#ifdef PLUGIN_077
#include PLUGINFILE(SKETCH_PATH,Plugin_077.c)
#endif

#ifdef PLUGIN_078
#include PLUGINFILE(SKETCH_PATH,Plugin_078.c)
#endif

#ifdef PLUGIN_079
#include PLUGINFILE(SKETCH_PATH,Plugin_079.c)
#endif

#ifdef PLUGIN_080
#include PLUGINFILE(SKETCH_PATH,Plugin_080.c)
#endif

#ifdef PLUGIN_081
#include PLUGINFILE(SKETCH_PATH,Plugin_081.c)
#endif

#ifdef PLUGIN_082
#include PLUGINFILE(SKETCH_PATH,Plugin_082.c)
#endif

#ifdef PLUGIN_083
#include PLUGINFILE(SKETCH_PATH,Plugin_083.c)
#endif

#ifdef PLUGIN_084
#include PLUGINFILE(SKETCH_PATH,Plugin_084.c)
#endif

#ifdef PLUGIN_085
#include PLUGINFILE(SKETCH_PATH,Plugin_085.c)
#endif

#ifdef PLUGIN_086
#include PLUGINFILE(SKETCH_PATH,Plugin_086.c)
#endif

#ifdef PLUGIN_087
#include PLUGINFILE(SKETCH_PATH,Plugin_087.c)
#endif

#ifdef PLUGIN_088
#include PLUGINFILE(SKETCH_PATH,Plugin_088.c)
#endif

#ifdef PLUGIN_089
#include PLUGINFILE(SKETCH_PATH,Plugin_089.c)
#endif

#ifdef PLUGIN_090
#include PLUGINFILE(SKETCH_PATH,Plugin_090.c)
#endif

#ifdef PLUGIN_091
#include PLUGINFILE(SKETCH_PATH,Plugin_091.c)
#endif

#ifdef PLUGIN_092
#include PLUGINFILE(SKETCH_PATH,Plugin_092.c)
#endif

#ifdef PLUGIN_093
#include PLUGINFILE(SKETCH_PATH,Plugin_093.c)
#endif

#ifdef PLUGIN_094
#include PLUGINFILE(SKETCH_PATH,Plugin_094.c)
#endif

#ifdef PLUGIN_095
#include PLUGINFILE(SKETCH_PATH,Plugin_095.c)
#endif

#ifdef PLUGIN_096
#include PLUGINFILE(SKETCH_PATH,Plugin_096.c)
#endif

#ifdef PLUGIN_097
#include PLUGINFILE(SKETCH_PATH,Plugin_097.c)
#endif

#ifdef PLUGIN_098
#include PLUGINFILE(SKETCH_PATH,Plugin_098.c)
#endif

#ifdef PLUGIN_099
#include PLUGINFILE(SKETCH_PATH,Plugin_099.c)
#endif

#ifdef PLUGIN_100
#include PLUGINFILE(SKETCH_PATH,Plugin_100.c)
#endif

#ifdef PLUGIN_250
#include PLUGINFILE(SKETCH_PATH,Plugin_250.c)
#endif

#ifdef PLUGIN_251
#include PLUGINFILE(SKETCH_PATH,Plugin_251.c)
#endif

#ifdef PLUGIN_252
#include PLUGINFILE(SKETCH_PATH,Plugin_252.c)
#endif

#ifdef PLUGIN_253
#include PLUGINFILE(SKETCH_PATH,Plugin_253.c)
#endif

#ifdef PLUGIN_254
#include PLUGINFILE(SKETCH_PATH,Plugin_254.c)
#endif

#ifdef PLUGIN_255
#include PLUGINFILE(SKETCH_PATH,Plugin_255.c)
#endif

void PluginInit(void)
  {
  byte x;

  // Wis te pointertabel voor de plugins.
  for(x=0;x<PLUGIN_MAX;x++)
    {
    Plugin_ptr[x]=0;
    Plugin_id[x]=0;
    }
    
  x=0;
        
  #ifdef PLUGIN_001
  Plugin_id[x]=1;Plugin_ptr[x++]=&Plugin_001;
  #endif
  
  #ifdef PLUGIN_002
  Plugin_id[x]=2;Plugin_ptr[x++]=&Plugin_002;
  #endif
  
  #ifdef PLUGIN_003
  Plugin_id[x]=3;Plugin_ptr[x++]=&Plugin_003;
  #endif
  
  #ifdef PLUGIN_004
  Plugin_id[x]=4;Plugin_ptr[x++]=&Plugin_004;
  #endif
  
  #ifdef PLUGIN_005
  Plugin_id[x]=5;Plugin_ptr[x++]=&Plugin_005;
  #endif
  
  #ifdef PLUGIN_006
  Plugin_id[x]=6;Plugin_ptr[x++]=&Plugin_006;
  #endif
  
  #ifdef PLUGIN_007
  Plugin_id[x]=7;Plugin_ptr[x++]=&Plugin_007;
  #endif
  
  #ifdef PLUGIN_008
  Plugin_id[x]=8;Plugin_ptr[x++]=&Plugin_008;
  #endif
  
  #ifdef PLUGIN_009
  Plugin_id[x]=9;Plugin_ptr[x++]=&Plugin_009;
  #endif
  
  #ifdef PLUGIN_010
  Plugin_id[x]=10;Plugin_ptr[x++]=&Plugin_010;
  #endif
  
  #ifdef PLUGIN_011
  Plugin_id[x]=11;Plugin_ptr[x++]=&Plugin_011;
  #endif
  
  #ifdef PLUGIN_012
  Plugin_id[x]=12;Plugin_ptr[x++]=&Plugin_012;
  #endif
  
  #ifdef PLUGIN_013
  Plugin_id[x]=13;Plugin_ptr[x++]=&Plugin_013;
  #endif
  
  #ifdef PLUGIN_014
  Plugin_id[x]=14;Plugin_ptr[x++]=&Plugin_014;
  #endif
  
  #ifdef PLUGIN_015
  Plugin_id[x]=15;Plugin_ptr[x++]=&Plugin_015;
  #endif
  
  #ifdef PLUGIN_016
  Plugin_id[x]=16;Plugin_ptr[x++]=&Plugin_016;
  #endif
  
  #ifdef PLUGIN_017
  Plugin_id[x]=17;Plugin_ptr[x++]=&Plugin_017;
  #endif
  
  #ifdef PLUGIN_018
  Plugin_id[x]=18;Plugin_ptr[x++]=&Plugin_018;
  #endif
  
  #ifdef PLUGIN_019
  Plugin_id[x]=19;Plugin_ptr[x++]=&Plugin_019;
  #endif
  
  #ifdef PLUGIN_020
  Plugin_id[x]=20;Plugin_ptr[x++]=&Plugin_020;
  #endif
  
  #ifdef PLUGIN_021
  Plugin_id[x]=21;Plugin_ptr[x++]=&Plugin_021;
  #endif
  
  #ifdef PLUGIN_022
  Plugin_id[x]=22;Plugin_ptr[x++]=&Plugin_022;
  #endif
  
  #ifdef PLUGIN_023
  Plugin_id[x]=23;Plugin_ptr[x++]=&Plugin_023;
  #endif
  
  #ifdef PLUGIN_024
  Plugin_id[x]=24;Plugin_ptr[x++]=&Plugin_024;
  #endif
  
  #ifdef PLUGIN_025
  Plugin_id[x]=25;Plugin_ptr[x++]=&Plugin_025;
  #endif
  
  #ifdef PLUGIN_026
  Plugin_id[x]=26;Plugin_ptr[x++]=&Plugin_026;
  #endif
  
  #ifdef PLUGIN_027
  Plugin_id[x]=27;Plugin_ptr[x++]=&Plugin_027;
  #endif
  
  #ifdef PLUGIN_028
  Plugin_id[x]=28;Plugin_ptr[x++]=&Plugin_028;
  #endif
  
  #ifdef PLUGIN_029
  Plugin_id[x]=29;Plugin_ptr[x++]=&Plugin_029;
  #endif
  
  #ifdef PLUGIN_030
  Plugin_id[x]=30;Plugin_ptr[x++]=&Plugin_030;
  #endif
  
  #ifdef PLUGIN_031
  Plugin_id[x]=31;Plugin_ptr[x++]=&Plugin_031;
  #endif
  
  #ifdef PLUGIN_032
  Plugin_id[x]=32;Plugin_ptr[x++]=&Plugin_032;
  #endif
  
  #ifdef PLUGIN_033
  Plugin_id[x]=33;Plugin_ptr[x++]=&Plugin_033;
  #endif
  
  #ifdef PLUGIN_034
  Plugin_id[x]=34;Plugin_ptr[x++]=&Plugin_034;
  #endif
  
  #ifdef PLUGIN_035
  Plugin_id[x]=35;Plugin_ptr[x++]=&Plugin_035;
  #endif
  
  #ifdef PLUGIN_036
  Plugin_id[x]=36;Plugin_ptr[x++]=&Plugin_036;
  #endif
  
  #ifdef PLUGIN_037
  Plugin_id[x]=37;Plugin_ptr[x++]=&Plugin_037;
  #endif
  
  #ifdef PLUGIN_038
  Plugin_id[x]=38;Plugin_ptr[x++]=&Plugin_038;
  #endif
  
  #ifdef PLUGIN_039
  Plugin_id[x]=39;Plugin_ptr[x++]=&Plugin_039;
  #endif
  
  #ifdef PLUGIN_040
  Plugin_id[x]=40;Plugin_ptr[x++]=&Plugin_040;
  #endif
  
  #ifdef PLUGIN_041
  Plugin_id[x]=41;Plugin_ptr[x++]=&Plugin_041;
  #endif
  
  #ifdef PLUGIN_042
  Plugin_id[x]=42;Plugin_ptr[x++]=&Plugin_042;
  #endif
  
  #ifdef PLUGIN_043
  Plugin_id[x]=43;Plugin_ptr[x++]=&Plugin_043;
  #endif
  
  #ifdef PLUGIN_044
  Plugin_id[x]=44;Plugin_ptr[x++]=&Plugin_044;
  #endif
  
  #ifdef PLUGIN_045
  Plugin_id[x]=45;Plugin_ptr[x++]=&Plugin_045;
  #endif
  
  #ifdef PLUGIN_046
  Plugin_id[x]=46;Plugin_ptr[x++]=&Plugin_046;
  #endif
  
  #ifdef PLUGIN_047
  Plugin_id[x]=47;Plugin_ptr[x++]=&Plugin_047;
  #endif
  
  #ifdef PLUGIN_048
  Plugin_id[x]=48;Plugin_ptr[x++]=&Plugin_048;
  #endif
  
  #ifdef PLUGIN_049
  Plugin_id[x]=49;Plugin_ptr[x++]=&Plugin_049;
  #endif
  
  #ifdef PLUGIN_050
  Plugin_id[x]=50;Plugin_ptr[x++]=&Plugin_050;
  #endif
  
  #ifdef PLUGIN_051
  Plugin_id[x]=51;Plugin_ptr[x++]=&Plugin_051;
  #endif
  
  #ifdef PLUGIN_052
  Plugin_id[x]=52;Plugin_ptr[x++]=&Plugin_052;
  #endif
  
  #ifdef PLUGIN_053
  Plugin_id[x]=53;Plugin_ptr[x++]=&Plugin_053;
  #endif
  
  #ifdef PLUGIN_054
  Plugin_id[x]=54;Plugin_ptr[x++]=&Plugin_054;
  #endif
  
  #ifdef PLUGIN_055
  Plugin_id[x]=55;Plugin_ptr[x++]=&Plugin_055;
  #endif
  
  #ifdef PLUGIN_056
  Plugin_id[x]=56;Plugin_ptr[x++]=&Plugin_056;
  #endif
  
  #ifdef PLUGIN_057
  Plugin_id[x]=57;Plugin_ptr[x++]=&Plugin_057;
  #endif
  
  #ifdef PLUGIN_058
  Plugin_id[x]=58;Plugin_ptr[x++]=&Plugin_058;
  #endif
  
  #ifdef PLUGIN_059
  Plugin_id[x]=59;Plugin_ptr[x++]=&Plugin_059;
  #endif
  
  #ifdef PLUGIN_060
  Plugin_id[x]=60;Plugin_ptr[x++]=&Plugin_060;
  #endif
  
  #ifdef PLUGIN_061
  Plugin_id[x]=61;Plugin_ptr[x++]=&Plugin_061;
  #endif
  
  #ifdef PLUGIN_062
  Plugin_id[x]=62;Plugin_ptr[x++]=&Plugin_062;
  #endif
  
  #ifdef PLUGIN_063
  Plugin_id[x]=63;Plugin_ptr[x++]=&Plugin_063;
  #endif
  
  #ifdef PLUGIN_064
  Plugin_id[x]=64;Plugin_ptr[x++]=&Plugin_064;
  #endif
  
  #ifdef PLUGIN_065
  Plugin_id[x]=65;Plugin_ptr[x++]=&Plugin_065;
  #endif
  
  #ifdef PLUGIN_066
  Plugin_id[x]=66;Plugin_ptr[x++]=&Plugin_066;
  #endif
  
  #ifdef PLUGIN_067
  Plugin_id[x]=67;Plugin_ptr[x++]=&Plugin_067;
  #endif
  
  #ifdef PLUGIN_068
  Plugin_id[x]=68;Plugin_ptr[x++]=&Plugin_068;
  #endif
  
  #ifdef PLUGIN_069
  Plugin_id[x]=69;Plugin_ptr[x++]=&Plugin_069;
  #endif
  
  #ifdef PLUGIN_070
  Plugin_id[x]=70;Plugin_ptr[x++]=&Plugin_070;
  #endif
  
  #ifdef PLUGIN_071
  Plugin_id[x]=71;Plugin_ptr[x++]=&Plugin_071;
  #endif
  
  #ifdef PLUGIN_072
  Plugin_id[x]=72;Plugin_ptr[x++]=&Plugin_072;
  #endif
  
  #ifdef PLUGIN_073
  Plugin_id[x]=73;Plugin_ptr[x++]=&Plugin_073;
  #endif
  
  #ifdef PLUGIN_074
  Plugin_id[x]=74;Plugin_ptr[x++]=&Plugin_074;
  #endif
  
  #ifdef PLUGIN_075
  Plugin_id[x]=75;Plugin_ptr[x++]=&Plugin_075;
  #endif
  
  #ifdef PLUGIN_076
  Plugin_id[x]=76;Plugin_ptr[x++]=&Plugin_076;
  #endif
  
  #ifdef PLUGIN_077
  Plugin_id[x]=77;Plugin_ptr[x++]=&Plugin_077;
  #endif
  
  #ifdef PLUGIN_078
  Plugin_id[x]=78;Plugin_ptr[x++]=&Plugin_078;
  #endif
  
  #ifdef PLUGIN_079
  Plugin_id[x]=79;Plugin_ptr[x++]=&Plugin_079;
  #endif
  
  #ifdef PLUGIN_080
  Plugin_id[x]=80;Plugin_ptr[x++]=&Plugin_080;
  #endif
  
  #ifdef PLUGIN_081
  Plugin_id[x]=81;Plugin_ptr[x++]=&Plugin_081;
  #endif
  
  #ifdef PLUGIN_082
  Plugin_id[x]=82;Plugin_ptr[x++]=&Plugin_082;
  #endif
  
  #ifdef PLUGIN_083
  Plugin_id[x]=83;Plugin_ptr[x++]=&Plugin_083;
  #endif
  
  #ifdef PLUGIN_084
  Plugin_id[x]=84;Plugin_ptr[x++]=&Plugin_084;
  #endif
  
  #ifdef PLUGIN_085
  Plugin_id[x]=85;Plugin_ptr[x++]=&Plugin_085;
  #endif
  
  #ifdef PLUGIN_086
  Plugin_id[x]=86;Plugin_ptr[x++]=&Plugin_086;
  #endif
  
  #ifdef PLUGIN_087
  Plugin_id[x]=87;Plugin_ptr[x++]=&Plugin_087;
  #endif
  
  #ifdef PLUGIN_088
  Plugin_id[x]=88;Plugin_ptr[x++]=&Plugin_088;
  #endif
  
  #ifdef PLUGIN_089
  Plugin_id[x]=89;Plugin_ptr[x++]=&Plugin_089;
  #endif
  
  #ifdef PLUGIN_090
  Plugin_id[x]=90;Plugin_ptr[x++]=&Plugin_090;
  #endif
  
  #ifdef PLUGIN_091
  Plugin_id[x]=91;Plugin_ptr[x++]=&Plugin_091;
  #endif
  
  #ifdef PLUGIN_092
  Plugin_id[x]=92;Plugin_ptr[x++]=&Plugin_092;
  #endif
  
  #ifdef PLUGIN_093
  Plugin_id[x]=93;Plugin_ptr[x++]=&Plugin_093;
  #endif
  
  #ifdef PLUGIN_094
  Plugin_id[x]=94;Plugin_ptr[x++]=&Plugin_094;
  #endif
  
  #ifdef PLUGIN_095
  Plugin_id[x]=95;Plugin_ptr[x++]=&Plugin_095;
  #endif
  
  #ifdef PLUGIN_096
  Plugin_id[x]=96;Plugin_ptr[x++]=&Plugin_096;
  #endif
  
  #ifdef PLUGIN_097
  Plugin_id[x]=97;Plugin_ptr[x++]=&Plugin_097;
  #endif
  
  #ifdef PLUGIN_098
  Plugin_id[x]=98;Plugin_ptr[x++]=&Plugin_098;
  #endif
  
  #ifdef PLUGIN_099
  Plugin_id[x]=99;Plugin_ptr[x++]=&Plugin_099;
  #endif
  
  #ifdef PLUGIN_100
  Plugin_id[x]=100;Plugin_ptr[x++]=&Plugin_100;
  #endif
  
  #ifdef PLUGIN_250
  Plugin_id[x]=250;Plugin_ptr[x++]=&Plugin_250;
  #endif

  #ifdef PLUGIN_251
  Plugin_id[x]=251;Plugin_ptr[x++]=&Plugin_251;
  #endif

  #ifdef PLUGIN_252
  Plugin_id[x]=252;Plugin_ptr[x++]=&Plugin_252;
  #endif

  #ifdef PLUGIN_253
  Plugin_id[x]=253;Plugin_ptr[x++]=&Plugin_253;
  #endif

  #ifdef PLUGIN_254
  Plugin_id[x]=254;Plugin_ptr[x++]=&Plugin_254;
  #endif

  #ifdef PLUGIN_255
  Plugin_id[x]=255;Plugin_ptr[x++]=&Plugin_255;
  #endif

  // Initialiseer alle plugins door aanroep met verwerkingsparameter PLUGIN_INIT
  PluginCall(PLUGIN_INIT,0,0);
  }

 /*********************************************************************************************\
 * Met deze functie worden de plugins aangeroepen. In Event->Command zit het nummer van het plugin dat moet
 * worden aangeroepen. Deze functie doorzoekt de ID en pointertabel en roept van hieruit het
 * juiste plugin aan. Als resultaat komt er true bij succes.
 * Als er een verzoek wordt gedaan om alle plugins het Rawsignal te onderzoeken, dan wordt teruggekeerd
 * met een true als het eerste plugin een true geretourneerd heeft.
 \*********************************************************************************************/

byte PluginCall(byte Function, struct NodoEventStruct *Event, char *str)
  {
  int x;

  switch(Function)
    {
    // Alle plugins langslopen, geen messages genereren.
    case PLUGIN_ONCE_A_SECOND:
    case PLUGIN_EVENT_IN:
    case PLUGIN_EVENT_OUT:
    case PLUGIN_I2C_IN:
    case PLUGIN_INIT:
      for(x=0; x<PLUGIN_MAX; x++)
        if(Plugin_id[x]!=0)
          Plugin_ptr[x](Function,Event,str);
      return true;
      break;
    
    // Alle plugins langslopen. Na de eerste hit direct terugkeren met returnwaarde true, geen messages genereren.
    case PLUGIN_MMI_IN:
    case PLUGIN_RAWSIGNAL_IN:
    case PLUGIN_SERIAL_IN:
    case PLUGIN_ETHERNET_IN:
      for(x=0; x<PLUGIN_MAX; x++)
        if(Plugin_id[x]!=0)
          if(Plugin_ptr[x](Function,Event,str))
            return true;
      break;

    // alleen specifieke plugin aanroepen zoals opgegeven in Event->Command. Bij terugkeer false, geef messsage en return met false;
    case PLUGIN_COMMAND:
      for(x=0; x<PLUGIN_MAX; x++)
        {
        if(Plugin_id[x]==Event->Command)
          {
          if(!Plugin_ptr[x](Function,Event,str))
            {
            RaiseMessage(MESSAGE_PLUGIN_ERROR,Plugin_id[x]);
            return false;
            }
          else
            {
            if(Event->Command && Event->Type!=NODO_TYPE_PLUGIN_COMMAND)
              {
              Event->Direction    = VALUE_DIRECTION_INPUT;
              Event->Port         = VALUE_SOURCE_PLUGIN;
              return ProcessEvent(Event);
              }
            return true;
            }
          }
        }
      RaiseMessage(MESSAGE_PLUGIN_UNKNOWN,Event->Command);
      return false;
      break;

    // alleen plugin aanroepen zoals opgegeven in Event->Command. Keer terug zonder message
    case PLUGIN_MMI_OUT:
      for(x=0; x<PLUGIN_MAX; x++)
        if(Plugin_id[x]==Event->Command)
          {
          Plugin_ptr[x](Function,Event,str);
          return true; // Plugin gevonden
          }

      break;
    }// case
  return false;
  }
