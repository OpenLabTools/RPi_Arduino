static uint16_t data;
uint8_t byte0, byte1;
volatile boolean sampled;

void setup()
{
  // Start serial at 9600 baud
  Serial.begin(9600);
  // Setup timer and adc registers
  adc_Timer_Setup();
  // Begin adc interrupts
  adc_Timer_Start();
}

// Infinite loop to print sampled values
void loop()
{ 
  // Wait until sampled flag is set to signal a new sample has been taken
  while(sampled == false);
  sampled = false;
  // Print time and value of adc sample
  Serial.print("Sample time: ");
  Serial.print(millis()/1000.0);
  Serial.print(" ADC value: ");
  Serial.println(data);
}

// ADC conversion complete ISR
ISR(ADC_vect)
{
  // Set sampled flag true to signal to loop() that a new sample is available
  sampled = true;
  byte0 = ADCL;
  byte1 = ADCH;
  // Store the value of the sample in data
  data = (byte1 << 8) | byte0;
}

// This interrupt is required in order to clear the OCF1B bit in register TIFR1. This bit is set when Timer1 equals the value of ICR1. 
ISR(TIMER1_COMPB_vect) 
{
}

void adc_Timer_Start ()
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


void adc_Timer_Setup()
{
  cli();
  // Set Up timer1 ///////////////////////////////////////
  // First set both TCCR1A and TCCR1B to 0, then we will set required bits equal to 1
  TCCR1A = 0;
  TCCR1B = 0;

  // Set timer1 prescaler (to 1024)
  TCCR1B |= 5; 

  // Set CTC Mode with TOP value set to be ICR1
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << WGM13);

  // Set TOP value of timer1 to give desired frequency (1Hz)
  ICR1 = 15624;

  // Set Up ADC /////////////////////////////////////////
  // Initialize ADCSRA to 0
  ADCSRA = 0;

  // Set ADC ref voltage to Vcc
  ADMUX &= ~(1 << REFS1);
  ADMUX |=  (1 << REFS0);

  // Set input as A0 pin
  ADMUX &= ~(1 << MUX3);
  ADMUX &= ~(1 << MUX2);
  ADMUX &= ~(1 << MUX1);
  ADMUX &= ~(1 << MUX0);

  // Set ADC clock prescaler to 16
  ADCSRA |= 7;

  // Set the trigger source for adc trigger to timer1 compare match B
  ADCSRB |=  (1 << ADTS2);
  ADCSRB &= ~(1 << ADTS1);
  ADCSRB |=  (1 << ADTS0);
  sei();
}





