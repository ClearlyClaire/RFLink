/*********************************************************************************************/
boolean ScanEvent(void) {                                         // Deze routine maakt deel uit van de hoofdloop en wordt iedere 125uSec. doorlopen
  unsigned long Timer=millis()+SCAN_HIGH_TIME;

  while(Timer>millis() || RepeatingTimer>millis()) {
       if (FetchSignal(PIN_RF_RX_DATA,HIGH)) {                    // RF: *** data start ***
          if ( PluginRXCall(0,0) ) {                                // Check all plugins to see which plugin can handle the received signal.
             RepeatingTimer=millis()+SIGNAL_REPEAT_TIME;
             return true;
          }
       }
  }// while
  return false;
}
/**********************************************************************************************\
 * Haal de pulsen en plaats in buffer. 
 * bij de TSOP1738 is in rust is de uitgang hoog. StateSignal moet LOW zijn
 * bij de 433RX is in rust is de uitgang laag. StateSignal moet HIGH zijn
 * 
 \*********************************************************************************************/
const unsigned long LoopsPerMilli=345;
const unsigned long Overhead=0;  

// Because this is a time critical routine, we use global variables so that the variables 
// do not need to be initialized at each function call. 
int RawCodeLength=0;
unsigned long PulseLength=0L;
unsigned long numloops=0L;
unsigned long maxloops=0L;

boolean Ftoggle=false;
uint8_t Fbit=0;
uint8_t Fport=0;
uint8_t FstateMask=0;
/*********************************************************************************************/
boolean FetchSignal(byte DataPin, boolean StateSignal) {
   uint8_t Fbit = digitalPinToBitMask(DataPin);
   uint8_t Fport = digitalPinToPort(DataPin);
   uint8_t FstateMask = (StateSignal ? Fbit : 0);

   if ((*portInputRegister(Fport) & Fbit) == FstateMask) {                       // Als er signaal is
     // Als het een herhalend signaal is, dan is de kans groot dat we binnen hele korte tijd weer in deze
     // routine terugkomen en dan midden in de volgende herhaling terecht komen. Daarom wordt er in dit
     // geval gewacht totdat de pulsen voorbij zijn en we met het capturen van data beginnen na een korte 
     // rust tussen de signalen.Op deze wijze wordt het aantal zinloze captures teruggebracht.
     if (RawSignal.Time) {                                                       //  Eerst een snelle check, want dit bevindt zich in een tijdkritisch deel...
        if (RawSignal.Repeats && (RawSignal.Time+SIGNAL_REPEAT_TIME)>millis()) { // ...want deze check duurt enkele micro's langer!
           PulseLength=micros()+SIGNAL_TIMEOUT*1000;                             // Wachttijd
           while ((RawSignal.Time+SIGNAL_REPEAT_TIME)>millis() && PulseLength>micros())
           if ((*portInputRegister(Fport) & Fbit) == FstateMask)
                  PulseLength=micros()+SIGNAL_TIMEOUT*1000;
           while((RawSignal.Time+SIGNAL_REPEAT_TIME)>millis() &&  (*portInputRegister(Fport) & Fbit) != FstateMask);
        }
     }
     RawCodeLength=1;                                                            // We starten bij 1, dit om legacy redenen. Vroeger had element 0 een speciaal doel.
     Ftoggle=false;                  
     maxloops = (SIGNAL_TIMEOUT * LoopsPerMilli);  
     do{                                                                         // lees de pulsen in microseconden en plaats deze in de tijdelijke buffer RawSignal
       numloops = 0;
       while (((*portInputRegister(Fport) & Fbit) == FstateMask) ^ Ftoggle)      // while() loop *A*
       if (numloops++ == maxloops) break;                                        // timeout opgetreden
       PulseLength=((numloops + Overhead)* 1000) / LoopsPerMilli;                // Bevat nu de pulslengte in microseconden
       if (PulseLength<MIN_PULSE_LENGTH) break;
       Ftoggle=!Ftoggle;    
       RawSignal.Pulses[RawCodeLength++]=PulseLength/(unsigned long)(RAWSIGNAL_SAMPLE_DEFAULT); // sla op in de tabel RawSignal
    } while (RawCodeLength<RAW_BUFFER_SIZE && numloops<=maxloops);               // Zolang nog ruimte in de buffer, geen timeout en geen stoorpuls
    if (RawCodeLength>=MIN_RAW_PULSES) {
       RawSignal.Repeats=0;                                                      // Op dit moment weten we nog niet het type signaal, maar de variabele niet ongedefinieerd laten.
       RawSignal.Multiply=RAWSIGNAL_SAMPLE_DEFAULT;                              // Ingestelde sample groote.
       RawSignal.Number=RawCodeLength-1;                                         // Aantal ontvangen tijden (pulsen *2)
       RawSignal.Pulses[RawSignal.Number]=0;                                     // Laatste element bevat de timeout. Niet relevant.
       RawSignal.Time=millis();
       return true;
    } else {
      RawSignal.Number=0;    
    }
  }
  return false;
}
/*********************************************************************************************/
// RFLink Board specific: Generate a short pulse to switch the Aurel Transceiver from TX to RX mode.
void RFLinkHW( void ) {
     delayMicroseconds(36);
     digitalWrite(PIN_BSF_0,LOW);
     delayMicroseconds(16);
     digitalWrite(PIN_BSF_0,HIGH);
     return;
}
/*********************************************************************************************\
 * Send rawsignal buffer to RF
\*********************************************************************************************/
void RawSendRF(void) {
  int x;
  digitalWrite(PIN_RF_RX_VCC,LOW);                                        // Spanning naar de RF ontvanger uit om interferentie met de zender te voorkomen.
  digitalWrite(PIN_RF_TX_VCC,HIGH);                                       // zet de 433Mhz zender aan
  delayMicroseconds(TRANSMITTER_STABLE_DELAY);                            // short delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms)
  
  // LET OP: In de Arduino versie 1.0.1 zit een bug in de funktie delayMicroSeconds(). Als deze wordt aangeroepen met een nul dan zal er
  // een pause optreden van 16 milliseconden. Omdat het laatste element van RawSignal af sluit met een nul (omdat de space van de stopbit 
  // feitelijk niet bestaat) zal deze bug optreden. Daarom wordt deze op 1 gezet om de bug te omzeilen. 
  RawSignal.Pulses[RawSignal.Number]=1;

  for(byte y=0; y<RawSignal.Repeats; y++) {                               // herhaal verzenden RF code
     x=1;
     noInterrupts();
     while(x<RawSignal.Number) {
        digitalWrite(PIN_RF_TX_DATA,HIGH);
        delayMicroseconds(RawSignal.Pulses[x++]*RawSignal.Multiply-5);    // min een kleine correctie  
        digitalWrite(PIN_RF_TX_DATA,LOW);
        delayMicroseconds(RawSignal.Pulses[x++]*RawSignal.Multiply-7);    // min een kleine correctie
     }
     interrupts();
     if (y+1 < RawSignal.Repeats) delay(RawSignal.Delay);                 // Delay buiten het gebied waar de interrupts zijn uitgeschakeld! Anders werkt deze funktie niet.
  }

  delayMicroseconds(TRANSMITTER_STABLE_DELAY);                            // short delay to let the transmitter become stable (Note: Aurel RTX MID needs 500µS/0,5ms)
  digitalWrite(PIN_RF_TX_VCC,LOW);                                        // zet de 433Mhz zender weer uit
  digitalWrite(PIN_RF_RX_VCC,HIGH);                                       // Spanning naar de RF ontvanger weer aan.
  RFLinkHW();
}
/*********************************************************************************************/

