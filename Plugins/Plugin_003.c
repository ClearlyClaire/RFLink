//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                        Plugin-03: FA500R                                          ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of sending and receiving the Elro Flamingo FA500 protocol. 
 * Also works with compatible devices like the Mumbi M-FS300 and Silvercrest 91210/60494 RCS AAA3680, Unitec eim 821/art.48111
 * 
 * Auteur             : StuntTeam
 * Support            : www.nodo-domotica.nl
 * Versie             : 18-01-2015, Versie 1.0
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 ***********************************************************************************************
 * Incoming event: "FA500 <adres>,<On | Off>
 * Send          : "FA500Send <Adres>, <On | Off> 
 *
 * Address = A/B/C/D matching the remote control buttons.
 ***********************************************************************************************
 * Technical information:
 * The FA500R remote control sends 3 different protocols.
 * 4 x Method 1 - 28 bit code
 * 6 x method 2 - AC compatible 
 * 5 x method 3 - 24/12 bit code 
 * It appears that the FA500S is only capable to react to the first method.
 * Nodo can only distinguish the 3rd method properly. Method 1 and 2 have to be pulled apart first which
 * is done via plugin 001.
 *
 * Device types:
 * Elro FA500S Flamingo Switch 
 * Elro FA500DSS Flamingo Dimmer 
 * Elro FA500WD Flamingo Outdoor 
 * Elro FA500R Flamingo Remote Control 
 *
 * PCB Markings:
 *   50027.01B FLF-130105 
 * KT50039.01A FLF-13-06-03 Chip marking: S007V0.1 
 * KT50040.01A FLF-13-06-04
 *
 * Sample:
 * 20;60;DEBUG;Pulses=24;Pulses(uSec)=325,800,275,800,825,225,275,800,275,825,275,800,825,225,275,800,825,225,275,800,275,800,275;
 * 20;61;DEBUG;Pulses=58;Pulses(uSec)=200,875,800,250,800,225,200,875,200,875,800,250,200,875,200,875,800,250,200,875,200,875,200,875,200,875,825,250,200,875,200,875,200,875,825,250,200,875,825,250,200,875,200,875,200,875,825,225,825,250,200,875,825,250,200,875,150;
 \*********************************************************************************************/
#define PLUGIN_ID 3

#define FA500RM3_PulseLength    26
#define FA500RM1_PulseLength    58
#define PLUGIN_003_EVENT        "FA500"
#define PLUGIN_003_COMMAND      "FA500Send"

void Flamingo_Send(int funitc, int fcmd);

boolean Plugin_003(byte function, struct NodoEventStruct *event, char *string)
  {
    boolean success=false;
    unsigned long bitstream=0;
  
  switch(function)
    {
    #ifdef PLUGIN_003_CORE

    case PLUGIN_RAWSIGNAL_IN:
      {
      int i,j;
      boolean error=false; 
      byte type=0;                      // 0=KAKU 1=ITK 2=PT2262
      byte housecode=0;
      byte unitcode=0;
      byte command=0;
      char buffer[14]=""; 
      unsigned long address=0;
      // ==========================================================================
      if (RawSignal.Number!=(FA500RM3_PulseLength) && RawSignal.Number!=(FA500RM1_PulseLength)) return false; 

      if (RawSignal.Number==(FA500RM3_PulseLength) ) { 
         // get all 26pulses =>24 manchester bits => 12 actual bits
         type=0;
         for(byte x=2;x <=24;x+=2) {
             if(RawSignal.Pulses[x]*RawSignal.Multiply > 400) {
                bitstream = (bitstream << 1);
            } else {
                bitstream = (bitstream << 1) | 0x1; 
            }
         }
      } else {
         // get all 58pulses =>24 manchester bits => 12 actual bits
         type=1;
         //for(byte x=1;x <=58;x++) {
         for(byte x=1;x <=56;x+=2) {
             if(RawSignal.Pulses[x]*RawSignal.Multiply > 400) {
                bitstream = (bitstream << 1);
            } else {
                bitstream = (bitstream << 1) | 0x1; 
            }
         }
         
      }      
      //==================================================================================
      // perform sanity checks
      if (bitstream==0) return false;           // no bits detected? 
      if (type == 0) {
         housecode=(((bitstream) >> 8) &0x0f);              
         unitcode=(( bitstream >> 1)& 0x7F);
         if (unitcode != 0x0A) {                   // invalid housecode?
            //Serial.println("FA500R error");
            return false;
         }      
         address=housecode;
         command=(bitstream)&1;
      } else {
         address=bitstream;
         address=address >> 22;
         address=address << 2;
         housecode=(bitstream)&3;
         address=address+housecode;
         if (address==0xd9) { // A On/off
            housecode=1;
         } else
         if (address==0xda) { // B On/off
            housecode=4;
         } else
         if (address==0x1a) { // C On/off
            housecode=5;
         } else
         if (address==0x99) { // D On/off
            housecode=0;
         } else {
            return false;
         }
         command=2;
         // Trick: here we use the on/off command from the other packet type as it is not detected in the current packet, it was passed via Pluses[0] in plugin 1
         if (RawSignal.Pulses[0]*RawSignal.Multiply > 1000 && RawSignal.Pulses[0]*RawSignal.Multiply < 1400) {
            command=0;
         } else 
         if (RawSignal.Pulses[0]*RawSignal.Multiply > 100 && RawSignal.Pulses[0]*RawSignal.Multiply < 400) {
            command=1;
         } 
         address=bitstream;
      }
      if (housecode != 0x01 && housecode != 0x04 && housecode != 0x05 && housecode != 0x00) { // invalid button code?
         return false;
      }  
      //==================================================================================
//      if (housecode == 1) housecode = 0x41; // A 0001 0001010 0/1     08   A    1
//      if (housecode == 4) housecode = 0x42; // B 0100 0001010 0/1     20   B    4
//      if (housecode == 5) housecode = 0x43; // C 0101 0001010 0/1     28   C    5
//      if (housecode == 0) housecode = 0x44; // D 0000 0001010 0/1     00   D    0 
      // ----------------------------------
      // All is OK, build event
      // ----------------------------------
      // Output
      // ----------------------------------
      sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
      Serial.print( buffer );
      // ----------------------------------
      Serial.print("FA500;");                           // Label
      if (type == 0) {
         sprintf(buffer, "ID=%02x%02x;", unitcode, housecode); // ID    
      } else {   
         sprintf(buffer, "ID=%02x%02x%02x%02x;", (address>>24)&0xff  ,(address>>16)&0xff,(address>>8)&0xff, address&0xff); // ID    
      }
      Serial.print( buffer );
      if (type == 0) {
         sprintf(buffer, "SWITCH=%02x%02x;", unitcode, housecode); // ID    
      } else {
         sprintf(buffer, "SWITCH=%02x;", housecode); // ID    
      }
      Serial.print( buffer );
      Serial.print("CMD=");                    
      if ( command == 1) {
         Serial.print("ON;");
      } else 
      if ( command == 0) {
         Serial.print("OFF;");
      } else {
         Serial.print("UNKOWN;");
      }
      Serial.println();
      // ----------------------------------
      RawSignal.Repeats    = true;                     // het is een herhalend signaal. Bij ontvangst herhalingen onderdrukken.
      //event->Par2          = bitstream & 0xFF;
      //event->Par1         |= (bitstream >> 11) & 0x01;
      //event->SourceUnit    = 0;                        // Komt niet van een Nodo unit af.
      //event->Type          = NODO_TYPE_PLUGIN_EVENT;
      //event->Command       = 3;                        // nummer van deze plugin
      RawSignal.Repeats    = true;                     // Hiermee geven we aan dat het om een herhalend signaal gaat en vervolgpulsen NIET tot een event moeten leiden.
      success=true;
      break;
      }
    case PLUGIN_COMMAND:
      {
      event->Port=VALUE_ALL;                           // Signaal mag naar alle door de gebruiker met [Output] ingestelde poorten worden verzonden.

      // event->Par1 contains the command (0 = OFF, 1 = ON) 
      // event->Par2 contains the button number (0/1/2/3) when A/B/C/D is entered as command parameter
      Flamingo_Send(event->Par2, event->Par1);
      //SendEvent(event,true,true,Settings.WaitFree==VALUE_ON);
      success=true;
      break;
    }
    #endif //PLUGIN_CORE_003
      
    #if NODO_MEGA
    case PLUGIN_MMI_IN:
      {
        // Reserveer een kleine string en kap voor de zekerheid de inputstring af om te voorkomen dat er
        // buiten gereserveerde geheugen gewerkt wordt.
        char *TempStr=(char*)malloc(INPUT_COMMAND_SIZE);

        // Hier aangekomen bevat string het volledige commando. Test als eerste of het opgegeven commando overeen komt met "Kaku"
        // Dit is het eerste argument in het commando.
        if(GetArgv(string,TempStr,1)) {
            event->Type  = 0;
            if(strcasecmp(TempStr,PLUGIN_003_COMMAND)==0) {
                event->Type  = NODO_TYPE_PLUGIN_COMMAND;
            }
            if(event->Type) {
                byte c;
                byte x=0;                                                 // teller die wijst naar het te behandelen teken
                byte Home=0;                                              // FA500 button A..D
                event->Command = 3;                                       // Plugin nummer  
        
                if(GetArgv(string,TempStr,2)) {                           // eerste parameter bevat adres volgens codering A0..P16 
                    while((c=tolower(TempStr[x++]))!=0) {
                        if(c>='a' && c<='d'){
                            Home=c-'a';
                        }                                                     // FA500 home A is intern 0
                    }
                    event->Par2=Home;
   
                    if(GetArgv(string,TempStr,3)) {                         // Het door de gebruiker ingegeven tweede parameter bevat het on/off commando
                        event->Par1 |= str2cmd(TempStr)==VALUE_ON; 
                        success=true;
                    }
                }
            }
        }
        free(TempStr);
        break;
      }
    #endif //MMI
    }      
    return success;
}
          
void Flamingo_Send(int fbutton, int fcmd) {
    int fpulse = 300;                              // Pulse witdh in microseconds
    int fretrans = 8;                              // Number of code retransmissions
    uint32_t fdatabit;
    uint32_t fdatamask = 0x80000000;
    uint32_t fsendbuff;
    uint32_t fsendbuff1;
    uint32_t fsendbuff2;
    uint32_t fsendbuff3;
    uint32_t fsendbuff4;

//  #define PIN_RF_TX_VCC               15                                          // +5 volt / Vcc spanning naar de zender.
//  #define PIN_RF_TX_DATA              14                                          // Data naar de 433Mhz zender
    if (fcmd == 0) { // OFF 
       fsendbuff1=0xDB2CE650;  // A Off
       fsendbuff2=0xDB9FF710;
       fsendbuff3=0xD9446790;
       fsendbuff4=0xD92B4050;
       //fsendbuff1=0xD86E6650;
       //fsendbuff2=0xDABDF710;
       //fsendbuff3=0xDA42A790;
       //fsendbuff4=0xDA614050;
       
    } else {         // ON
       fsendbuff1=0xDA9E4A90;  // A On
       fsendbuff2=0xDB5C0A10;
       fsendbuff3=0xD84D8490;
       fsendbuff4=0xDBABC5D0;
       //fsendbuff1=0xD97A4A10;
       //fsendbuff2=0xDA9A8490;
       //fsendbuff3=0xDB58C5D0;
       //fsendbuff4=0xDBF40A90;
    }
    digitalWrite(PIN_RF_RX_VCC,LOW);                                              // Spanning naar de RF ontvanger uit om interferentie met de zender te voorkomen.
    digitalWrite(PIN_RF_TX_VCC,HIGH);                                             // zet de 433Mhz zender aan
    delay(TRANSMITTER_STABLE_TIME);                                               // kleine pauze om de zender de tijd te geven om stabiel te worden 

    for (int nRepeat = 0; nRepeat <= fretrans; nRepeat++) {

        if (nRepeat==0 || nRepeat==4) fsendbuff=fsendbuff1;
        if (nRepeat==1 || nRepeat==5) fsendbuff=fsendbuff2;
        if (nRepeat==2 || nRepeat==6) fsendbuff=fsendbuff3;
        if (nRepeat==3 || nRepeat==7) fsendbuff=fsendbuff4;
    
        // send SYNC 1P High, 15P low
        digitalWrite(PIN_RF_TX_DATA, HIGH);
        delayMicroseconds(fpulse * 1);
        digitalWrite(PIN_RF_TX_DATA, LOW);
        delayMicroseconds(fpulse * 15);
        // end send SYNC

        // Send command
        for (int i = 0; i < 28; i++) {                              // Flamingo command is only 28 bits 
            // read data bit
            fdatabit = fsendbuff & fdatamask;                         // Get most left bit
            fsendbuff = (fsendbuff << 1);                             // Shift left

            if (fdatabit != fdatamask) { // Write 0
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse * 3);
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse * 1);
            } else { // Write 1
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse * 1);
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse * 3);
            }
        }
    }
    delay(TRANSMITTER_STABLE_TIME);   // kleine pause zodat de ether even schoon blijft na de stopbit
    digitalWrite(PIN_RF_TX_VCC,LOW);  // zet de 433Mhz zender weer uit
    digitalWrite(PIN_RF_RX_VCC,HIGH); // Spanning naar de RF ontvanger weer aan.
}
