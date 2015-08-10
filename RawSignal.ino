
/**********************************************************************************************\
 * Haal de pulsen en plaats in buffer. 
 * bij de TSOP1738 is in rust is de uitgang hoog. StateSignal moet LOW zijn
 * bij de 433RX is in rust is de uitgang laag. StateSignal moet HIGH zijn
 * 
 \*********************************************************************************************/

volatile unsigned long LastPulse = 0L;
/*********************************************************************************************/
inline void RXInterruptHandlerHelper(const unsigned long &now)
{
  //TODO: handle repeat timer
  static int RawCodeLength=0;
  static RawSignalStruct *front = &RawSignalA, *back = &RawSignalB;

  RawSignalStruct &Rawsignal(*front);
  unsigned long PulseLength = now - LastPulse;
  LastPulse = now;

  if (PulseLength > SIGNAL_TIMEOUT * 1000)
  {
    // Timeout
    Rawsignal.Pulses[RawCodeLength++] = (SIGNAL_TIMEOUT * 1000) / (unsigned long) (RAWSIGNAL_SAMPLE_RATE);
  }
  else if (PulseLength >= MIN_PULSE_LENGTH)
  {
    Rawsignal.Pulses[RawCodeLength++] = PulseLength / (unsigned long) (RAWSIGNAL_SAMPLE_RATE);
    if (RawCodeLength < RAW_BUFFER_SIZE)
      return;
  }

  // This is the end of a signal
  if (RawCodeLength >= MIN_RAW_PULSES) {
    //Rawsignal.Repeats = 0;
    //Rawsignal.Multiply = RAWSIGNAL_SAMPLE_RATE;
    Rawsignal.Number = RawCodeLength; // This will get decremented later
    //Rawsignal.Pulses[RawCodeLength] = 0;
    //XXX: Rawsignal.Time = millis();
    RawCodeLength = 0;

    // Swap RawSignal structures and mark the recently-filled one as
    // available
    if (!RawSignalPtr)
    {
        RawSignalPtr = front;
        front = back;
        back = RawSignalPtr;
    }
  }
  else
  {
    RawCodeLength = 0;
  }
}

void RXInterruptHandler()
{
  RXInterruptHandlerHelper(micros());
}

boolean ScanEvent(void) {                                         // Deze routine maakt deel uit van de hoofdloop en wordt iedere 125uSec. doorlopen
  RawSignalStruct *MyRawSignal;

  noInterrupts();
      // Timeout, trigger signal processing
      if (micros() >= LastPulse + SIGNAL_TIMEOUT * 1000)
        RXInterruptHandlerHelper(LastPulse + SIGNAL_TIMEOUT * 1000);

      // Get RawSignal
      MyRawSignal = RawSignalPtr;
  interrupts();

  if (MyRawSignal)
  {
    // First, fill up what we know we can fill up
    MyRawSignal->Repeats = 0;
    MyRawSignal->Multiply = RAWSIGNAL_SAMPLE_RATE;
    MyRawSignal->Pulses[MyRawSignal->Number] = 0;
    MyRawSignal->Number--;
    MyRawSignal->Time = millis(); // Not too precise, but well

    // For now this is super slow, but at least it's safe
    memcpy(&RawSignal, MyRawSignal, sizeof(RawSignalStruct));

    // Mark signal as consumed
    RawSignalPtr = 0;

    if (PluginRXCall(0, 0))
    {
        // TODO: handle repeat timer
        return true;
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

  detachInterrupt(1);

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

  attachInterrupt(1, RXInterruptHandler, CHANGE);

  RFLinkHW();
}
/*********************************************************************************************/

