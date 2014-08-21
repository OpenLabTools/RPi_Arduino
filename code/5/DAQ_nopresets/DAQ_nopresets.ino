static uint8_t data0[500];
static uint8_t data1[500];
volatile boolean buffselect=0;
volatile int i=0, q=0;
volatile uint8_t Stop=0, sample = 0;
byte b0=0, b1=0, b2=0;

void setup()
{
  // Begin serial communication at 1000000 baud
  Serial.begin(1000000);
  // Function to set the registers needed for correct adc and timer interrupt operation
  adcSetup();
  adcStart();
}


void loop()
{
  if (Serial.available())
  {
    // Store the last 3 bytes of data from serial
    b2=b1;
    b1=b0;
    b0=Serial.read();
    // if the last two characters read in are "\E" (ascii of 92 and 69) then we have recieved a message across serial
    if (b0 == 69 && b1 == 92)
    {
      // The byte (b2) before "\E" tells the Arduino which mode of opperation is required, "a" (ascii 97) checks if the Arduino
      // is ready, the Arduino sends back an "r" across serial to signal it is ready
      if (b2 == 97)
      {
        Serial.println("r");
      }
      // If the Pi sends the ascii character "c" (ascii 99) then log data
      else if (b2 == 99)
      {
        logData();
      }
    }
  }
}

void logData()
{
  // Enable sampling
  sample = 1;
  // Set i to 0 so that we begin filling buff0 from its first element
  i=0;
  // Loop to continuously transmit buffers until sample is set to false (this is done inside the ADC ISR when 250 buffers
  // have been transmitted)
  while (sample == 1)
  {
    // Wait until the buffer is full, Stop = 1 is set in the ADC ISR once the value of i reaches a value of 1000
    while(Stop==0);
    Stop=0;

    // If buffselect = 1 then the buffer data0 has just been filled so we transmit this buffer while data1 is now filled
    if (buffselect) 
    {    
      Serial.write(data0,500);
    }
    // If buffselect = 0 then the buffer data1 has just been filled so we transmit this buffer while data0 is now filled
    else 
    {
      Serial.write(data1,500);
    }
  }
}

// This ISR is called once the adc finishes conversion. When a conversion is completed the ADIF bit in the ADCSRA register is set equal to 1. 
// When ADIF is set to 1 the interupt is triggered.
ISR(ADC_vect)
{
  if (sample == 1)
  {
    // If buffselect is 0 then write data to data0 buffer
    if(buffselect==0) data0[i]=ADCH;
    // If buffselect is 1 then write data to data1 buffer
    else if (buffselect==1) data1[i]=ADCH;

    // check if i is greater then 500, if it is then set i back to 0, set Stop = 1 to signal in void loop() that we are ready to send a buffer, and finally toggle
    // buffselect so that data is placed into the buffer not being transmitted. If i is not greater or equal to 500 then increment its value by 1
    if(++i >= 500)
    { 
      i=0;
      Stop=1;
      buffselect = !buffselect;
      q++;
      // Once we have transmitted 250 buffers stop sampling
      if (q==250)
      { 
        sample = 0;
        q=0;
      }
    }
  }
}

// This interrupt is required in order to clear the OCF1B bit in register TIFR1. This bit is set when Timer1 equals the value of ICR1. 
ISR(TIMER1_COMPB_vect) 
{
}

void adcStart ()
{
  cli();
  // Enable the ADC
  ADCSRA |= (1 << ADEN);
  // Set conversion bit to zero
  ADCSRA |= (1 << ADSC);
  // Set auto trigger of adc (adc will trigger on the selected signal, in this
  // case timer1 reaching a certain value)
  ADCSRA |= (1 << ADATE);
  // Enable adc interrupts (this allows an interrupt to be called once the adc has
  // finished a conversion
  ADCSRA |= (1 << ADIE);

  // Allow timer interrupts for timer1 B
  TIMSK1 |= (1<<OCIE1B);
  // Set timer value to 0
  TCNT1 = 0; 
  sei(); 
}

void adcSetup()
{
  cli();
  // Set Up timer1 /////////////////////////////
  // TCCR1A to 0, and initialise TCR1B to 0
  TCCR1A = 0;
  TCCR1B = 0;

  // Set timer1 prescaler
  TCCR1B |= 1; 

  // Set CTC Mode with TOP value set to be ICR1
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << WGM13);

  // Set TOP value of timer1 to give desired frequency (25ksps)
  ICR1 = 639;

  // Set ADC//////////////////////////////////////
  // Set ADCSRA to 0
  ADCSRA = 0;

  // Set ADC ref voltage to Vcc
  ADMUX &= ~(1 << REFS1);
  ADMUX |=  (1 << REFS0);

  // Set left adjusted results for easy reading of a single byte from ADCH
  ADMUX |= (1 << ADLAR);

  // Set input as A0 pin
  ADMUX &= ~(1 << MUX3);
  ADMUX &= ~(1 << MUX2);
  ADMUX &= ~(1 << MUX1);
  ADMUX &= ~(1 << MUX0);

  // Set ADC clock prescaler to 16
  ADCSRA |= 4;

  // Set the trigger source for adc trigger to timer1 compare match B
  ADCSRB |=  (1 << ADTS2);
  ADCSRB &= ~(1 << ADTS1);
  ADCSRB |=  (1 << ADTS0);
  sei();
}






