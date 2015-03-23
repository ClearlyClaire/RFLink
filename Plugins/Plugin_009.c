//#######################################################################################################
//##                    This Plugin is only for use with the RFLink software package                   ##
//##                                        Plugin-06: X10 RF                                          ##
//#######################################################################################################
/*********************************************************************************************\
 * This plugin takes care of sending and receiving the X10 RF protocol. 
 * 
 * Auteur             : Voyager
 * Support            : www.nodo-domotica.nl
 * Versie             : 18-01-2015, Versie 1.0
 * License            : This code is free for use in any open source project when this header is included.
 *                      Usage of any parts of this code in a commercial application is prohibited!
 ***********************************************************************************************
 * Incoming event: "X10 <adres>, <On | Off>
 * Send          : "X10Send <Adres>, <On | Off> 
 *
 * Address = A1 - P16
 ***********************************************************************************************
 * Technical information:
 * RF packets are 68 bits long transferring 64 manchester encoded bits resulting in 32 bits / 4 bytes.
 *
 * address  address^  data     data^  
 * 01100000 10011111 00000000 11111111      609F00FF                                 
 * 10011111 01100000 11111111 00000000      9F60FF00
 * 
 * 4 bytes are transmitted, the second and fourth are the inverse of the first and third byte.
 * So the actual data is only 2 bytes
 * 
 * 01100000 00000000
 * AAAABBBB CDEDDDDD
 *
 * A = Housecode    0110 a  6    0111 b  7    0100 c  4    0101 d  5     1000 e  8       1001 f  9       1010 g  a       1011 h  b
 *                 1110 i  e    1111 j  f    1100 k  c    1101 l  d     0000 m  0       0001 n  1       0010 o  2       0011 p  3      
 * B = Unitcode 1-8 / 9-16 indicator
 * C = Group/Dimmer indicator
 * D = Unitcode 
 * E = on/off indicator
 *
 * on
 * 20;06;DEBUG;Pulses=68;Pulses(uSec)=3300,4225,400,375,400,1325,400,1325,400,1325,400,375,400,375,400,375,400,375,400,1325,400,375,400,375,400,375,400,1350,400,1350,375,1350,400,1350,400,375,400,375,400,375,400,1325,400,1325,400,375,400,375,400,375,400,1350,400,1325,400,1325,400,375,400,375,400,1325,400,1325,400,1325,400
 * off
 * 20;10;DEBUG;Pulses=68;Pulses(uSec)=3300,4225,400,375,400,1350,400,1350,400,1325,400,375,400,375,400,375,400,375,400,1325,400,375,400,375,400,375,400,1325,400,1325,400,1325,400,1325,400,375,400,375,400,1325,400,1350,400,1350,400,375,400,375,400,375,375,1350,400,1350,400,375,400,375,400,375,400,1325,400,1350,400,1325,400;
 * 20;20;DEBUG;Pulses=66;Pulses(uSec)=425,350,375,1300,375,1300,375,1350,375,375,375,1350,375,375,375,375,375,1350,375,375,375,375,375,375,400,1350,375,375,400,1350,375,1350,400,1325,400,375,400,375,400,375,400,375,400,375,400,375,400,375,400,375,400,1325,400,1325,400,1325,400,1325,400,1325,400,1350,375,1350,375;
 \*********************************************************************************************/
#define PLUGIN_ID 9

#define X10_PulseLength         66
#define PLUGIN_009_EVENT        "X10"
#define PLUGIN_009_COMMAND      "X10Send"

void X10_Send(uint32_t address);

boolean Plugin_009(byte function, struct NodoEventStruct *event, char *string)
  {
    boolean success=false;
    unsigned long bitstream=0;
  
  switch(function)
    {
    #ifdef PLUGIN_009_CORE

    case PLUGIN_RAWSIGNAL_IN:
      {
      int i,j;
      byte housecode=0;
      byte unitcode=0;
      byte command=0;
      char buffer[14]=""; 
	  byte data[4]; 
      byte start=0;
      // ==========================================================================
      if ( (RawSignal.Number != (X10_PulseLength )) && (RawSignal.Number != (X10_PulseLength+2)) ) return false; 
      if (RawSignal.Number == 68) {
         if ( (RawSignal.Pulses[1]*RawSignal.Multiply > 3000) && (RawSignal.Pulses[2]*RawSignal.Multiply > 3000) ) {
            start=2;
         } else {
            return false;                           // not an X10 packet
         }
      }
      // get all 24 bits
      for(byte x=2+start;x < ((X10_PulseLength)+start) ;x+=2) {
          if(RawSignal.Pulses[x]*RawSignal.Multiply > 600) {
             bitstream = (bitstream << 1) | 0x1; 
         } else {
             bitstream = (bitstream << 1);
         }
      }
      //==================================================================================
      // order received data
      data[0]=((bitstream)>>24)&0xff;
      data[1]=((bitstream)>>16)&0xff;
      data[2]=((bitstream)>>8)&0xff;
      data[3]=(bitstream)&0xff;

	  data[1]=data[1]^0xff;
	  data[3]=data[3]^0xff;
      // ----------------------------------
      // perform sanity checks
      if (data[0] != data[1]) return false;
      if (data[2] != data[3]) return false;
      // ----------------------------------
      data[1]=data[1]&0x0f;                         // lower nibble only
      data[0]=data[0]&0xf0;                         // upper nibble only      

      housecode=0;
	  if (data[0]==0x60) housecode=0;
	  if (data[0]==0x70) housecode=1;
	  if (data[0]==0x40) housecode=2;
	  if (data[0]==0x50) housecode=3;
	  if (data[0]==0x80) housecode=4;
	  if (data[0]==0x90) housecode=5;
	  if (data[0]==0xa0) housecode=6;
	  if (data[0]==0xb0) housecode=7;
	  if (data[0]==0xe0) housecode=8;
	  if (data[0]==0xf0) housecode=9;
	  if (data[0]==0xc0) housecode=10;
	  if (data[0]==0xd0) housecode=11;
	  if (data[0]==0x00) housecode=12;
	  if (data[0]==0x10) housecode=13;
	  if (data[0]==0x20) housecode=14;
	  if (data[0]==0x30) housecode=15;
      
	  if (data[2]==0x00) { unitcode=1; command=1;}	  
      if (data[2]==0x20) { unitcode=1; command=0;}	  
      if (data[2]==0x10) { unitcode=2; command=1;}	  
      if (data[2]==0x30) { unitcode=2; command=0;}	  
      if (data[2]==0x08) { unitcode=3; command=1;}	  
      if (data[2]==0x28) { unitcode=3; command=0;}	  
      if (data[2]==0x18) { unitcode=4; command=1;}	  
      if (data[2]==0x38) { unitcode=4; command=0;}	  
      if (data[2]==0x40) { unitcode=5; command=1;}	  
      if (data[2]==0x60) { unitcode=5; command=0;}	  
      if (data[2]==0x50) { unitcode=6; command=1;}	  
      if (data[2]==0x70) { unitcode=6; command=0;}	    
      if (data[2]==0x48) { unitcode=7; command=1;}	  
      if (data[2]==0x68) { unitcode=7; command=0;}	  
      if (data[2]==0x58) { unitcode=8; command=1;}	  
      if (data[2]==0x78) { unitcode=8; command=0;}	 
      if (data[2]==0x88) { unitcode=0; command=2;}	  
      if (data[2]==0x98) { unitcode=0; command=3;}	  
      if (data[2]==0x80) { unitcode=0; command=4;}	  
      if (data[2]==0x90) { unitcode=0; command=5;}	  

 	  if ( (data[1]==0x04) && (command < 2) ) {  
         unitcode=unitcode+8;	  
	  } 
      //==================================================================================
      // ----------------------------------
      // All is OK, build event
      // ----------------------------------
      // Output
      // ----------------------------------
      sprintf(buffer, "20;%02X;", PKSequenceNumber++); // Node and packet number 
      Serial.print( buffer );
      // ----------------------------------
      Serial.print("X10;");                         // Label
      sprintf(buffer, "ID=%02x;", 0x41+housecode);  // ID    
      Serial.print( buffer );
      sprintf(buffer, "SWITCH=%d;", unitcode);   
      Serial.print( buffer );
      Serial.print("CMD=");                    
      if ( command == 0) {
         Serial.print("OFF;");
      } else 
      if ( command == 1) {
         Serial.print("ON;");
      } else 
      if ( command == 2) {
         Serial.print("BRIGHT;");
      } else 
      if ( command == 3) {
         Serial.print("DIM;");
      } else 
      if ( command == 4) {
         Serial.print("ALLOFF;");
      } else 
      if ( command == 5) {
         Serial.print("ALLON;");
      }
      Serial.println();
      // ----------------------------------
      RawSignal.Repeats    = true;                  // het is een herhalend signaal. Bij ontvangst herhalingen onderdrukken.
      RawSignal.Number=0; 
      success=true;
      break;
      }
    case PLUGIN_COMMAND:
      {
      event->Port=VALUE_ALL;                        // Signaal mag naar alle door de gebruiker met [Output] ingestelde poorten worden verzonden.
      X10_Send(event->Par2);                        // event->Par2 contains the full bitstream to send
      success=true;
      break;
    }
    #endif //PLUGIN_CORE_009
      
    #if NODO_MEGA
    case PLUGIN_MMI_IN:
      {
      char *TempStr=(char*)malloc(INPUT_COMMAND_SIZE);
      if(GetArgv(string,TempStr,1)) {
        event->Type  = 0;
        
        if(strcasecmp(TempStr,PLUGIN_009_COMMAND)==0) {
          event->Type  = NODO_TYPE_PLUGIN_COMMAND;
        }

        if(event->Type) {
          byte c;
          byte x=0;                                 // teller die wijst naar het te behandelen teken
          byte Home=0;                              // home A..P
          byte Address=0;                           // unit 1..16
          byte command=0;
          event->Command = PLUGIN_ID;               // Plugin nummer  
          uint32_t newadd=0;
        
          if (GetArgv(string,TempStr,2)) {          // contains adres according to A1..P16 
             while((c=tolower(TempStr[x++]))!=0) {
                  if(c>='0' && c<='9'){Address=Address*10;Address=Address+c-'0';}
                  if(c>='a' && c<='p'){Home=c-'a';} // home A is intern 0
             }

             if (GetArgv(string,TempStr,3)) {       // Het door de gebruiker ingegeven tweede parameter bevat het on/off commando
                if (Home == 0) event->Par1 = 0x60;  
                if (Home == 1) event->Par1 = 0x70;  
                if (Home == 2) event->Par1 = 0x40;  
                if (Home == 3) event->Par1 = 0x50;  
                if (Home == 4) event->Par1 = 0x80;  
                if (Home == 5) event->Par1 = 0x90;  
                if (Home == 6) event->Par1 = 0xa0;  
                if (Home == 7) event->Par1 = 0xb0;  
                if (Home == 8) event->Par1 = 0xe0;  
                if (Home == 9) event->Par1 = 0xf0;  
                if (Home ==10) event->Par1 = 0xc0;  
                if (Home ==11) event->Par1 = 0xd0;  
                if (Home ==12) event->Par1 = 0x00;  
                if (Home ==13) event->Par1 = 0x10;  
                if (Home ==14) event->Par1 = 0x20;  
                if (Home ==15) event->Par1 = 0x30;  
                if (Address > 7) { 
                   event->Par1 = event->Par1 + 4;
                   Address=Address-8;
                }
                // ---------------
                Home=str2cmd(TempStr);
                if (Home == 0) {                    // DIM/BRIGHT command
                    if (strcasecmp(TempStr,"DIM")==0) { 
                       command=3;
                    } else
                    if (strcasecmp(TempStr,"BRIGHT")==0) { 
                       command=2;
                    } 
                    event->Par1 = event->Par1 + 4;
                } else {
                    if (Home==VALUE_ON) { 
                       command=1;
                    } else
                    if (Home==VALUE_OFF) { 
                       command=0;
                    } else
                    if (Home==VALUE_ALLOFF) { 
                       command=4;
                       event->Par1 = event->Par1 + 4;
                    } else
                    if (Home==VALUE_ALLON) { 
                       command=5;
                       event->Par1 = event->Par1 + 4;
                    } 
                }
                if (Address == 1 && command == 1) event->Par2=0x00; 
                if (Address == 1 && command == 0) event->Par2=0x20; 
                if (Address == 2 && command == 1) event->Par2=0x10; 
                if (Address == 2 && command == 0) event->Par2=0x30; 
                if (Address == 3 && command == 1) event->Par2=0x08; 
                if (Address == 3 && command == 0) event->Par2=0x28; 
                if (Address == 4 && command == 1) event->Par2=0x18; 
                if (Address == 4 && command == 0) event->Par2=0x38; 
                if (Address == 5 && command == 1) event->Par2=0x40; 
                if (Address == 5 && command == 0) event->Par2=0x60; 
                if (Address == 6 && command == 1) event->Par2=0x50; 
                if (Address == 6 && command == 0) event->Par2=0x70; 
                if (Address == 7 && command == 1) event->Par2=0x48; 
                if (Address == 7 && command == 0) event->Par2=0x68; 
                if (Address == 8 && command == 1) event->Par2=0x58; 
                if (Address == 8 && command == 0) event->Par2=0x78; 
                if (command == 2) event->Par2=0x88; 
                if (command == 3) event->Par2=0x98; 
                if (command == 4) event->Par2=0x80; 
                if (command == 5) event->Par2=0x90; 
                success=true;
                // -----------------------------
                newadd=event->Par2 <<8;
                event->Par2=event->Par2^0xff;
                newadd=newadd+event->Par2;
                event->Par2=event->Par1^0xff;
                event->Par2=event->Par2<<16;
                newadd=newadd+event->Par2;
                event->Par2=event->Par1;
                event->Par2=event->Par2<<24;
                newadd=newadd+event->Par2;
                event->Par2=newadd;
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

void X10_Send(uint32_t address) {
    int fpulse  = 375;                              // Pulse witdh in microseconds
    int fretrans = 4;                               // Number of code retransmissions
    uint32_t fdatabit;
    uint32_t fdatamask = 0x80000000;
    uint32_t fsendbuff;

    digitalWrite(PIN_RF_RX_VCC,LOW);                // Disable RF receiver
    digitalWrite(PIN_RF_TX_VCC,HIGH);               // Enable RF transmitter
    delay(TRANSMITTER_STABLE_TIME);                 // Short delay
    
    for (int nRepeat = 0; nRepeat <= fretrans; nRepeat++) {
        fsendbuff=address;

        // send SYNC 12P High, 10P low
        digitalWrite(PIN_RF_TX_DATA, HIGH);
        delayMicroseconds(fpulse * 12);
        digitalWrite(PIN_RF_TX_DATA, LOW);
        delayMicroseconds(fpulse * 10);
        // end send SYNC
        // Send command
        for (int i = 0; i < 32; i++) {              // 32 bits
            // read data bit
            fdatabit = fsendbuff & fdatamask;       // Get most left bit
            fsendbuff = (fsendbuff << 1);           // Shift left
            if (fdatabit != fdatamask) {            // Write 0
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse * 1);
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse * 1);
            } else {                                // Write 1
                digitalWrite(PIN_RF_TX_DATA, HIGH);
                delayMicroseconds(fpulse * 1);
                digitalWrite(PIN_RF_TX_DATA, LOW);
                delayMicroseconds(fpulse * 4);
            }
        }
        // Send Stop/delay
        digitalWrite(PIN_RF_TX_DATA, HIGH);
        delayMicroseconds(fpulse * 1);
        digitalWrite(PIN_RF_TX_DATA, LOW);
        delayMicroseconds(fpulse * 20);
    }
     delay(TRANSMITTER_STABLE_TIME);                // Short delay
     digitalWrite(PIN_RF_TX_VCC,LOW);               // Disable RF transmitter
     digitalWrite(PIN_RF_RX_VCC,HIGH);              // Enable RF receiver
     return;
}
