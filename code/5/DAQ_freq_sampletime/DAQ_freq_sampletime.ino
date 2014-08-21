static uint8_t data0[500];
static uint8_t data1[500];
volatile boolean buffselect=0;
volatile int i=0, q=0;
volatile uint8_t Stop=0, sample =0;
byte b0=0, b1=0, b2=0, b3=0, b4=0, b5=0;


struct sdata {
  uint8_t mode;        // Stores sampling mode
  uint8_t prescaler;   // Stores timer1 prescaler
  uint8_t adcp;        // Stores adc timer prescaler
  uint16_t timerTop;   // Stores value of TOP register
  uint16_t p;          // Stores number of times to transmit buffer
};

static struct sdata sdata_1;

void setup()
{
  // Begin serial at 1000000 baud
  Serial.begin(1000000);
  // Set sdata_1 to dummy values and then configure the adc / timer registers using adcSetup(sdata_1). Then if we recieve a command
  // to sample data before we recieve a command to configure registers the Arduino will still function correctly
  sdata_1.mode = 0;
  sdata_1.prescaler = 2;
  sdata_1.adcp = 7;
  sdata_1.timerTop = 199999;
  sdata_1.p = 1;
  adcSetup(sdata_1);
}

void loop()
{
  if (Serial.available())
  {
    // Store the last 6 bytes of data from serial
    b5=b4;
    b4=b3;
    b3=b2;
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
      // "b" tells the Arduino to reconfigure the sampling mode, to do this we read the three bytes sent before b2 (the fourth - sixth bytes
      // of the message) which contain the information on what sampling mode, and how long to sample
      else if (b2 == 98)
      {
        // Send b3 b4 b5 to grabvalues which returns sdata_1 which contains the information needed to set the ADC / timer registers
        grabvalues(&sdata_1, b3, b4, b5);
        // Set the ADC / timer registers for desired opperation
        adcSetup(sdata_1);
      }
      // If the Pi sends the ascii character "c" (ascii 99) then log data
      else if (b2 == 99)
      {
        // Begin conversions
        adcStart();
        logData();
      }
    }
  }
}

// Function to log data and transmit to the Pi from the ADC
void logData()
{
  // Enable sampling
  sample = 1;
  // Set i to 0 so that we place data into the start of the buffer
  i=0;
  // Loop to continuously transmit buffers until sample is set to false
  while (sample == 1)
  {
    // Wait until the buffer is full, Stop = 1 is set in the ADC ISR once the value of i reaches a value of 1000
    while(Stop==0);
    // Set Stop = 0 in preparation for next loop iteration
    Stop=0;
    // If buffselect = true then the buffer data0 has just been filled so we transmit this buffer while data1 is filled
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
  // Stop the ADC
  ADCSRA = 0;
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
    if (buffselect==1) data1[i]=ADCH;

    // check if i is greater then 500, if it is then set i back to 0, set Stop = 1 to signal in void loop() that we are ready to send a buffer, and finally toggle
    // buffselect so that data is placed into the buffer not being transmitted. If i is not greater or equal to 500 then increment its value by 1
    if(++i >= 500)
    { 
      i=0;
      Stop=1;
      buffselect = !buffselect;
      q++;
      if (q==sdata_1.p)
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

void adcSetup(struct sdata sdata_1)
{
  cli();
  // Set Up timer1 //
  // TCCR1A to 0 (no pwm), and initialise TCR1B to 0
  TCCR1A = 0;
  TCCR1B = 0;
  ADCSRA = 0;

  // Set timer1 prescaler
  TCCR1B |= sdata_1.prescaler; 

  // Set CTC Mode with TOP value set to be ICR1
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << WGM13);

  // Set TOP value of timer1 to give desired frequency
  ICR1 = sdata_1.timerTop;

  // Set ADC //
  // Set ADC ref voltage to Vcc
  ADMUX &= ~(1 << REFS1);
  ADMUX |=  (1 << REFS0);
  // Set left adjusted results so we can read the most significant byte directly from ADCH
  ADMUX |= (1 << ADLAR);
  // Set input as A0 pin
  ADMUX &= ~(1 << MUX3);
  ADMUX &= ~(1 << MUX2);
  ADMUX &= ~(1 << MUX1);
  ADMUX &= ~(1 << MUX0);

  // Set ADC clock prescaler
  ADCSRA |= sdata_1.adcp;

  // Set the trigger source for adc trigger to timer1 compare match B
  ADCSRB |=  (1 << ADTS2);
  ADCSRB &= ~(1 << ADTS1);
  ADCSRB |=  (1 << ADTS0);
  sei();
}

void grabvalues(struct sdata* sdata_1, byte byte0, byte byte1, byte byte2)
{
  sdata_1->mode = byte2;
  sdata_1->p = (byte1 << 8) | (byte0);

  if (sdata_1->mode == 0)
  {
    sdata_1->prescaler = 2;
    sdata_1->adcp = 7; // Sets division factor to 128
    sdata_1->timerTop = 19999;
  }
  else if (sdata_1->mode == 1)
  {
    sdata_1->prescaler = 1;
    sdata_1->adcp = 7;    // Sets division factor to 128
    sdata_1->timerTop = 15999;
  }
  else if (sdata_1->mode == 2)
  {
    sdata_1->prescaler = 1;
    sdata_1->adcp = 5;    // Sets division factor to 32
    sdata_1->timerTop = 1599;
  }
  else if (sdata_1->mode == 3)
  {
    sdata_1->prescaler = 1;
    sdata_1->adcp = 4;    // Sets division factor to 16
    sdata_1->timerTop = 639;
  }
}




