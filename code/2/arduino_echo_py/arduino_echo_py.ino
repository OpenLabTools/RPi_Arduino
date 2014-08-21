void setup()
{
  // Start Serial communication at 250 000 baud
  Serial.begin(250000);
}

void loop()
{
  // Infinite loop
  while(1)
  {
    // If there are bytes available in the serial input buffer
    if (Serial.available()) 
    {
      // Read a single byte and write it back over serial
      Serial.write(Serial.read());
    }
  }
}

