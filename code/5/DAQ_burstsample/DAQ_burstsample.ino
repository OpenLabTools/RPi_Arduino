static uint8_t data0[1000];
volatile boolean exitFlag =0, samplenow = 0, serialSent = 1;
volatile int i=0;
byte b0=0, b1=0, b2=0;

void setup()
{
  Serial.begin(1000000);
  // Setup adc and timer registers
  adc_Timer_Setup();
  // Set up external interrupt registers
  extIntSetup();
  // Enable interrupts
  adc_Timer_Start(); 
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
      // If the Pi sends the ascii character "c" (ascii 99) then run burst sampling
      else if (b2 == 99)
      {
        burstSample();
      }
    }
  }
}  

void burstSample()
{
  // Infinite loop, continues to run unless external interupt INT1 (on pin3) is triggered
  while(1)
  {
    // Wait until interrupt to start sampling is triggered (this sets samplenow to 1), while sampling data samplenow stays equal to 1, once sampling
    // is finished samplenow is set back to zero so we must wait for this event also before we transmit the buffer
    while (samplenow == 0) if(exitFlag ==1) break;
    while (samplenow == 1) if(exitFlag ==1) break;
    if (exitFlag == 1) 
    {
      // Send "e" across serail to signal to stop sampling
      Serial.println("e");
      exitFlag = 0;
      break;
    }

    // Send "l" to signal to Pi that buffer will be sent next
    Serial.println("l");
    // Write the buffer to the Pi
    Serial.write(data0,1000);
    // Set serialSent to 1 to flag that we can now start a new sampling session
    serialSent = 1;
  }
}

// ADC conversion complete interrupt
ISR(ADC_vect)
{
  // Only store samples if samplenow == 1, otherwise do nothing
  if (samplenow == 1)
  {
    // Store ADC reading in buffer
    data0[i]=ADCH;
    // Once buffer is full set samplenow to false
    if(++i >= 1000)
    { 
      samplenow = 0;
    }
  }
}

ISR(TIMER1_COMPB_vect) // Clear OCF1B
{
}

// External Interrupt 0 (pin2) interrupt
ISR(INT0_vect)
{
  // When the interrupt is triggered by a rising edge on pin 2 then set samplenow = 1 to start the burst sampling,
  // if a conversion is already in process then do nothing
  if (serialSent == 1);
  {
    samplenow = 1;
    serialSent = 0;
    i = 0;
  }
}

// External Interrupt 1 (pin3) interrupt
ISR(INT1_vect)
{
  // When the interrupt is triggered by a rising edge on pin 3 then set exitFlag = true to signal for the sampling to stop
  exitFlag = 1;
}

void adc_Timer_Start ()
{
  cli();
  // Enable the ADC
  ADCSRA |= (1 << ADEN);
  // Set conversion bit to zero
  ADCSRA |= (1 << ADSC);
  // Set auto trigger of adc (adc will trigger on the selected signal, in this
  // case timer1 reaching the value of ICR1)
  ADCSRA |= (1 << ADATE);
  // Enable adc interrupts (this allows an interrupt to be triggered once the adc has
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
  // Set Up timer1 ////////////////////////////
  // TCCR1A to 0, and initialise TCR1B to 0
  TCCR1A = 0;
  TCCR1B = 0;

  // Set timer1 prescaler to 1
  TCCR1B &= ~(1 << CS12);
  TCCR1B &= ~(1 << CS11);
  TCCR1B |= (1 << CS10);

  // Set CTC Mode with TOP value set to be ICR1
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << WGM13);

  // Set TOP value of timer1 required to trigger compare match frequency of 10 000ksps (24)
  ICR1 = 159;

  // Set Up ADC ///////////////////////////////
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

  // Set ADC clock prescaler to 8
  ADCSRA &= ~(1 << ADPS2);
  ADCSRA |=  (1 << ADPS1);
  ADCSRA |=  (1 << ADPS0);

  // Set the trigger source for adc trigger to timer1 compare match B
  ADCSRB |=  (1 << ADTS2);
  ADCSRB &= ~(1 << ADTS1);
  ADCSRB |=  (1 << ADTS0);
  sei();
}

void extIntSetup()
{
  cli();
  // Set up External Interrupts /////////////////
  // Set ISC01 and ISC00 to give interrupt on rising clock edge of ISC0 (pin 2)
  EICRA |= (1 << ISC01);
  EICRA |= (1 << ISC00);
  EICRA |= (1 << ISC11);
  EICRA |= (1 << ISC10);

  // Enable external interrupt ITN0 (pin 2)
  EIMSK |= (1 << INT0);
  // Enable external interrupt ITN1 (pin 3)
  EIMSK |= (1 << INT1);
  sei();
}



