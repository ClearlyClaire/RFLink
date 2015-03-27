//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                     Plugin-082 Mertik Maxitrol                                    ##
//#######################################################################################################
/*********************************************************************************************\
 * This Plugin takes care of reception of Mertik Maxitrol / DRU for fireplaces
 * PCB markings: G6R H4T1.
 *
 * Auteur             : Maurice Ruiter (Dodge)
 * Support            : www.nodo-domotica.nl
 * Datum              : 3-3-2015
 * Versie             : 1.0 
 *********************************************************************************************
 * Changelog: v1.0 initial release
 *********************************************************************************************
 * Technical information:
 *
 * 0001100101101001011001101
 *   ----------------------- data bits (10=1 01=0)
 * --                        preamble, always 00?   
 * Shortened: (10=1 01=0)
 * 01100101101001011001101
 * 0 1 0 0 1 1 0 0 1 0 1 1 
 * 
 * 010011001011  
 *         ----   command => 4 bits
 * --------       address => 8 bits 
 *
 * command bits:
 * 0111 7 off 
 * 0011 3 on  
 * 1011 b up
 * 1101 d down
 * 1000 8 stop
 * 1010 a go up
 * 1100 c go down
 *
 * Sample RF packet: 
 * Pulses=26;Pulses(uSec)=475,300,325,700,325,700,325,700,325,700,725,300,725,300,725,300,725,300,725,300,325,700,725,300,725;
 \*********************************************************************************************/
#define PLUGIN_ID 82
#define PLUGIN_NAME "maxitrol"
#define MAXITROL_PULSECOUNT     26
#define PLUGIN_082_EVENT        "Maxitrol"
#define PLUGIN_082_COMMAND      "MaxiSend"
#define PLUGIN_082_ON           "On"
#define PLUGIN_082_OFF          "Off"
#define PLUGIN_082_UP           "Up"
#define PLUGIN_082_DOWN         "Down"
#define PLUGIN_082_STOP         "Stop"
#define PLUGIN_082_GO_UP       "Go_Up"
#define PLUGIN_082_GO_DOWN     "Go_Down"

#define PLUGIN_082_RFSTART      100
#define PLUGIN_082_RFSPACE      250
#define PLUGIN_082_RFLOW        400
#define PLUGIN_082_RFHIGH       750

byte Plugin_082_ProtocolCheckID(byte checkID);
byte Plugin_082_ProtocolValidID[2];
byte Plugin_082_ProtocolVar[2];

boolean Plugin_082(byte function, struct NodoEventStruct *event, char *string)
{
  boolean success=false;

  switch(function)
  {
#ifdef PLUGIN_082_CORE
  case PLUGIN_RAWSIGNAL_IN:
    {
      if (RawSignal.Number !=MAXITROL_PULSECOUNT) return false;
      unsigned int bitstream=0;                   // holds first 16 bits
      byte address=0;
      byte command=0;
      byte status=0;
      char buffer[14]="";
      //==================================================================================
      // get bytes

            for(int x=3;x<=RawSignal.Number-1;x=x+2) {
         if (RawSignal.Pulses[x]*RawSignal.Multiply < 550) {
            bitstream = (bitstream << 1);           // 0
         } else {
            bitstream = (bitstream << 1) | 0x1;     // 1
         }
      }
      //==================================================================================
      // all bytes received, make sure checksum is okay
     if (RawSignal.Pulses[1]*RawSignal.Multiply > 550) return false;
      if (RawSignal.Pulses[2]*RawSignal.Multiply > 550) return false;
      //==================================================================================
      command=(bitstream) & 0x0f;                   // get address from pulses
      address=(bitstream)>>4;
      if (command == 0xB) status=1;                 // up
      else if (command == 0xD) status=2;            // down
      else if (command == 0x7) status=3;            // off
      else if (command == 0x3) status=4;            // on
      else if (command == 0x8) status=5;            // stop
      else if (command == 0xa) status=6;            // go up
      else if (command == 0xc) status=7;            // go down
      else {
        return false;
      }
      //==================================================================================
      // Output
      // ----------------------------------
      sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
      Serial.print( buffer );
      Serial.print("Mertik;");                      // Label
      sprintf(buffer, "ID=%u;", address,DEC);       // ID
      Serial.print( buffer );
      sprintf(buffer, "SWITCH=%02x;", status);
      Serial.print( buffer );
      Serial.print("CMD=");                         
      if (status==1) Serial.print("UP;");           
      if (status==2) Serial.print("DOWN;");         
      if (status==3) Serial.print("OFF;");          
      if (status==4) Serial.print("ON;");           
      if (status==5) Serial.print("STOP;");         
      if (status==6) Serial.print("GOUP;");       
      if (status==7) Serial.print("GODOWN;");     
      Serial.println();
      //==================================================================================
      RawSignal.Repeats=true;                       // suppress repeats of the same RF packet
      RawSignal.Number=0;
      success = true;
      break;
    }  //einde ontvangen

  case PLUGIN_COMMAND:
    {
        unsigned int bitstream=0;                   // holds first 16 bits
        unsigned int bitstream2=0;                  // holds last 8 bits
        if (event->Par2 == 1) bitstream2=0xB;
        else if (event->Par2 == 2) bitstream2=0xD;
        else if (event->Par2 == 3) bitstream2=0x7;
        else if (event->Par2 == 4) bitstream2=0x3;
        else if (event->Par2 == 5) bitstream2=0x8;
        else if (event->Par2 == 6) bitstream2=0xA;
        else if (event->Par2 == 7) bitstream2=0xC;
        else {
            return false;
        }
        byte address=0;
        address=event->Par1,HEX;
        bitstream= address;
        RawSignal.Multiply=50;
        RawSignal.Repeats=10;
        RawSignal.Delay=20;
        RawSignal.Pulses[1]=PLUGIN_082_RFLOW/RawSignal.Multiply;
        RawSignal.Pulses[2]=PLUGIN_082_RFLOW/RawSignal.Multiply;
        for(byte x=18;x>=3;x=x-2) {
           if ((bitstream & 1) == 1) {
              RawSignal.Pulses[x] = PLUGIN_082_RFLOW/RawSignal.Multiply;
              RawSignal.Pulses[x-1] = PLUGIN_082_RFHIGH /RawSignal.Multiply;
           } else {
              RawSignal.Pulses[x] = PLUGIN_082_RFHIGH /RawSignal.Multiply;
              RawSignal.Pulses[x-1] = PLUGIN_082_RFLOW/RawSignal.Multiply;
           }
           bitstream = bitstream >> 1;
        }
        for(byte x=26;x>=19;x=x-2) {
           if ((bitstream2 & 1) == 1) {
              RawSignal.Pulses[x] = PLUGIN_082_RFLOW/RawSignal.Multiply;
              RawSignal.Pulses[x-1] = PLUGIN_082_RFHIGH /RawSignal.Multiply;
           } else {
              RawSignal.Pulses[x] = PLUGIN_082_RFHIGH /RawSignal.Multiply;
              RawSignal.Pulses[x-1] = PLUGIN_082_RFLOW/RawSignal.Multiply;
           }
           bitstream2 = bitstream2 >> 1;
        }
        RawSignal.Pulses[27]=PLUGIN_082_RFSTART/RawSignal.Multiply;
        RawSignal.Number=27;
        RawSendRF();
        success=true;
        break;
    }
#endif // PLUGIN_082_CORE

#if NODO_MEGA
  case PLUGIN_MMI_IN:
    {
    char *TempStr=(char*)malloc(INPUT_COMMAND_SIZE);

    if (GetArgv(string,TempStr,1)) {
       if (strcasecmp(TempStr,PLUGIN_082_EVENT)==0) {
          event->Type = NODO_TYPE_PLUGIN_EVENT;
          event->Command = 82; // Plugin nummer
          success=true;
       }
       if (strcasecmp(TempStr,PLUGIN_082_COMMAND)==0) {
          event->Type = NODO_TYPE_PLUGIN_COMMAND;
          event->Command = 82; // Plugin nummer
          success=true;
       }
    }
    // char *TempStr=(char*)malloc(INPUT_COMMAND_SIZE);
    if (GetArgv(string,TempStr,3)) {
       if(strcasecmp(TempStr,PLUGIN_082_STOP)==0) event->Par2=5;
       if(strcasecmp(TempStr,PLUGIN_082_ON)==0) event->Par2=4;
       if(strcasecmp(TempStr,PLUGIN_082_OFF)==0) event->Par2=3;
       if(strcasecmp(TempStr,PLUGIN_082_UP)==0) event->Par2=1;
       if(strcasecmp(TempStr,PLUGIN_082_DOWN)==0) event->Par2=2;
       if(strcasecmp(TempStr,PLUGIN_082_GO_UP)==0) event->Par2=6;
       if(strcasecmp(TempStr,PLUGIN_082_GO_DOWN)==0) event->Par2=7;
    }
    free(TempStr);
    break;
    }

  case PLUGIN_MMI_OUT:
    {
    if (event->Type==NODO_TYPE_PLUGIN_COMMAND)
       strcpy(string,PLUGIN_082_COMMAND);           // Command
       strcat(string," ");
       strcat(string,int2str(event->Par1));
       strcat(string,",");
       if (event->Par2==1) strcat(string,"Up");
       else if (event->Par2==2) strcat(string,"Down");
       else if (event->Par2==3) strcat(string,"Off");
       else if (event->Par2==4) strcat(string,"On");
       else if (event->Par2==5) strcat(string,"Stop");
       else if (event->Par2==6) strcat(string,"Go Up");
       else if (event->Par2==7) strcat(string,"Go Down");
       else {
           return false;
       }
       break;
    }
#endif //NODO_MEGA
  }      
  return success;
}
