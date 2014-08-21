int analogValue;
double sampletime;

void setup()
{
  // Begin serial communication at 9600 baud
  Serial.begin(9600);
}


void loop()
{
  // Read a single ADC value
  analogValue = analogRead(0);
  // Find system time
  sampletime = micros()/1000000.0;
  
  // Print the ADC value and time over serial
  Serial.print(analogValue);
  Serial.print("  ");
  Serial.println(sampletime,6);    // Print to 6 significant figures
  
  // Delay for 1 second
  delay(1000); 
}


