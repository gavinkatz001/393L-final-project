volatile unsigned long gdMillis = 0;

unsigned long gdMills() {
  return gdMillis;
}

// define ISR protocol to perform an action when timer1 == OCR1A 
//we know to compare timer1 to OCR1A because this is specified by the TIMER1_COMPA_vect vector, which defines the comparison behavior for the processor to execute
// this is the same as saying: trigger an interrupt and add 1 to gdMillis when timer1 has had 2000 ticks (aka 1 ms, has passed) 
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
