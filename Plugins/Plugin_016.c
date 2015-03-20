//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                   Plugin-16: SendHomeEasy EU                                      ##
//#######################################################################################################
/*********************************************************************************************\
 * Dit protocol zorgt voor aansturing van Home Easy EU ontvangers
 * die werken volgens de automatische codering (Ontvangers met leer-knop)
 *
 * LET OP: GEEN SUPPORT VOOR DIRECTE DIMWAARDES!!!
 *
 * Author             : StuntTeam
 * Support            : www.nodo-domotica.nl
 * Date               : 17-11-2014
 * Version            : 1.0
 * Compatibility      : RFLink 1.0
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Syntax             : "HomeEasySend <Adres>,<On|Off|>
 \*********************************************************************************************/
#define PLUGIN_ID 16
#define PLUGIN_NAME "HomeEasySend"

boolean Plugin_016(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef PLUGIN_016_CORE
  case PLUGIN_RAWSIGNAL_IN:
    break;

  case PLUGIN_COMMAND:
    {
    unsigned long bitstream=0L;
    byte address = 0;
    byte channel = 0;
    byte channelcode = 0;
    byte command = 0;
    byte i=1; // bitcounter in stream
    byte y; // size of partial bitstreams

    //==================================================================================
    RawSignal.Multiply=50;

    address = (event->Par2 >> 4) & 0x7;   // 3 bits address (higher bits from HomeEasy address, bit 7 not used
    channel = event->Par2 & 0xF;    // 4 bits channel (lower bits from HomeEasy address
    command = event->Par1 & 0xF;    // 12 = on, 0 = off

    if (channel == 0) channelcode = 0x8E;
    else if (channel == 1) channelcode = 0x96;
    else if (channel == 2) channelcode = 0x9A;
    else if (channel == 3) channelcode = 0x9C;
    else if (channel == 4) channelcode = 0xA6;
    else if (channel == 5) channelcode = 0xAA;
    else if (channel == 6) channelcode = 0xAC;
    else if (channel == 7) channelcode = 0xB2;
    else if (channel == 8) channelcode = 0xB4;
    else if (channel == 9) channelcode = 0xB8;
    else if (channel == 10) channelcode = 0xC6;
    else if (channel == 11) channelcode = 0xCA;
    else if (channel == 12) channelcode = 0xCC;
    else if (channel == 13) channelcode = 0xD2;
    else if (channel == 14) channelcode = 0xD4;
    else if (channel == 15) channelcode = 0xD8;
    //==================================================================================
    // Startbits / Preamble
    y=11; // bit position from the bitstream
    bitstream = 0x63C;
    for (i=1;i<=22;i=i+2) {
        RawSignal.Pulses[i] = HomeEasy_ShortHigh/RawSignal.Multiply;
        if((bitstream>>(y-1))&1)          // bit 1
          RawSignal.Pulses[i+1] = HomeEasy_LongLow/RawSignal.Multiply;
        else                              // bit 0
          RawSignal.Pulses[i+1] = HomeEasy_ShortLow/RawSignal.Multiply;
        y--;
    }
    // ------------------------------
    // Address 
    y=32; // bit position from the bitstream
    bitstream = 0xDAB8F56C + address;
    for (i=23;i<=86;i=i+2) {
        RawSignal.Pulses[i] = HomeEasy_ShortHigh/RawSignal.Multiply;
        if((bitstream>>(y-1))&1)          // bit 1
           RawSignal.Pulses[i+1] = HomeEasy_LongLow/RawSignal.Multiply;
        else                              // bit 0
           RawSignal.Pulses[i+1] = HomeEasy_ShortLow/RawSignal.Multiply;
        y--;
    }
    // ------------------------------
    // Commands etc.
    y=15; // bit position from the bitstream
    bitstream = 0x5C00;  // bit 10 on, bit 11 off indien OFF
    if (event->Par1==VALUE_OFF) bitstream = 0x5A00;

    bitstream = bitstream + channelcode;

    for (i=87;i<=116;i=i+2) {
        RawSignal.Pulses[i] = HomeEasy_ShortHigh/RawSignal.Multiply;
        if((bitstream>>(y-1))&1)          // bit 1
           RawSignal.Pulses[i+1] = HomeEasy_LongLow/RawSignal.Multiply;
        else                              // bit 0
           RawSignal.Pulses[i+1] = HomeEasy_ShortLow/RawSignal.Multiply;
        y--;
    }
    //==================================================================================

    RawSignal.Pulses[116]=0;
    RawSignal.Number=116; // aantal bits*2 die zich in het opgebouwde RawSignal bevinden  unsigned long bitstream=0L;
    event->Port=VALUE_ALL; // Signaal mag naar alle door de gebruiker met [Output] ingestelde poorten worden verzonden.
    RawSignal.Repeats=5;   // vijf herhalingen.
    RawSignal.Delay=20; // Tussen iedere pulsenreeks enige tijd rust.
    SendEvent(event,true,true,Settings.WaitFree==VALUE_ON);
    success=true;
    break;
    }
#endif // PLUGIN_CORE_016

#if NODO_MEGA
  case PLUGIN_MMI_IN:
    {
    char* str=(char*)malloc(INPUT_COMMAND_SIZE);

    if(GetArgv(string,str,1))
      {
      if(strcasecmp(str,PLUGIN_NAME)==0)
        {
        if(GetArgv(string,str,2))
          {
            event->Par2=str2int(str);    
            if(GetArgv(string,str,3))
            {
              // Vul Par1 met het HomeEasy commando. Dit kan zijn: VALUE_ON, VALUE_OFF, Andere waarden zijn ongeldig.
              // haal uit de tweede parameter een 'On' of een 'Off'.
              if(event->Par1=str2cmd(str))
                {
                event->Type  = NODO_TYPE_PLUGIN_COMMAND;
                event->Command = 16; // Plugin nummer  
                success=true;
                } 
             }
          }
        }
      }
    free(str);
    break;
    }

  case PLUGIN_MMI_OUT:
    {
    strcpy(string,PLUGIN_NAME);            // Eerste argument=het commando deel
    strcat(string," ");

    // In Par3 twee mogelijkheden: Het bevat een door gebruiker ingegeven adres 0..255 of een volledig HomeEasy adres.
    if(event->Par2>=0x0ff)
      strcat(string,int2strhex(event->Par2)); 
    else
      strcat(string,int2str(event->Par2)); 

    strcat(string,",");

    if(event->Par1==VALUE_ON)
      strcat(string,"On");  
    else if(event->Par1==VALUE_OFF)
      strcat(string,"Off");
    else
      strcat(string,int2str(event->Par1));

    break;
    }
#endif //NODO_MEGA
  }      
  return success;
}
