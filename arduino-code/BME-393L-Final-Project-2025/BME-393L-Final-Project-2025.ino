volatile unsigned long gdMillis = 0;

unsigned long gdMills() {
  return gdMillis;
}

ISR(TIMER1_COMPA_vect){
  gdMillis++; 
}

void setup() {
  noInterrupts(); // disable interrupts during setup
   
  // Normal mode (no PWM)
  TCCR1A = 0;
  
  // reset timer1 = 0
  TCNT1 = 0;
   
  // for good practice, clear register before setting bits 
  TCCR1B = 0;

  // set up timer1 with prescale = 8 
  TCCR1B |= 1 << CS11;

  // set value to compare timer1 against
  OCR1A = 1999;

  // set arduino to CTC mode (clear timer on compare)
  TCCR1B |= 1 << WGM12;

  // set control bit to enable compare match A interrupt (comparing against OCR1A)
  TIMSK1 |= (1 << OCIE1A);  

  interrupts(); // enable interrupts

  Serial.begin(9600);
}

void loop() {

  static unsigned long lastPrint = 0;

  if (gdMillis - lastPrint >= 1000) { // every 1 second
    lastPrint = gdMillis;
    Serial.print("gdMillis = ");
    Serial.println(gdMillis);
  }
}
