// Flag that is set when the interrupt triggers
volatile boolean sendnow = false;

void setup()
{
  // Start serial communication and configure registers to enable timer interrupts
  Serial.begin(9600);
  // Set up registers to give desired operation of timer interrupts
  timerIntSetup();
  // Begin timer interrupts
  timerIntStart();
}

// Continuously check the flag 'sendnow', if the flag is set and interrupt has triggered
// so print the system time to serial
void loop()
{
  if (sendnow == true)
  {
    Serial.println(micros()/1000000.0, 3);
    // Set sendnow back to false in preparation for next interrupt
    sendnow = false;
  }
}

// Timer ISR (triggers at 1Hz)
ISR(TIMER1_COMPA_vect)
{
  sendnow = true;
}

void timerIntSetup()
{
  // Disable global interrupts
  cli();
  // First set both TCCR1A and TCCR1B to 0, then we will set required bits equal to 1
  // Set TCCR1A = 0
  TCCR1A = 0;
  // Set TCCR1B =0
  TCCR1B = 0;

  // Enable CTC mode with TOP value set by OCR1A register
  TCCR1B |= (1 << WGM12);

  // Set timer1 clock prescaler to 1024
  TCCR1B |= (1 << CS12);
  TCCR1B |= (1 << CS10);

  // Set OCR1A register to 15624 (the TOP register, this is the value the timer will count up to
  // before it resets, this gives a frequency with TCCR1B of 16000000 / ((1024) * (15624+1)) = 1Hz
  OCR1A = 15624;
  // Enable global interrupts
  sei();
}

void timerIntStart()
{
  // Disable global interrupts
  cli();
  // Enable Timer1 compare match A interrupts
  TIMSK1 |= (1 << OCIE1A);
  // Enable global interrupts
  sei();
}



