#include <LiquidCrystal.h>


volatile unsigned long gdMillis = 0;

// Match pin mapping of LCD Keypad Shield:
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);  // RS, E, D4, D5, D6, D7

const int dataInPin = 2;

// Placeholder variables
uint16_t player1_score_raw = 0;
uint16_t player2_score_raw = 0;
uint16_t total_rounds_played = 0;

float player1_score = 0.0;
float player2_score = 0.0;


unsigned long gdMills() {return gdMillis;}

void pause(int millisTopause) {
  unsigned long startTime = gdMills();
  while (gdMills() - startTime < millisTopause) {}  // do nothing to kill time. include assembly no operation instruction to avoid being optimized away
}

// define ISR protocol to perform an action when timer1 == OCR1A 
//we know to compare timer1 to OCR1A because this is specified by the TIMER1_COMPA_vect vector, which defines the comparison behavior for the processor to execute
// this is the same as saying: trigger an interrupt and add 1 to gdMillis when timer1 has had 2000 ticks (aka 1 ms, has passed) 
ISR(TIMER1_COMPA_vect){gdMillis++;}

void setup() {
  noInterrupts(); // disable interrupts during setup
   
  // Normal mode (no PWM)
  TCCR1A = 0;
  
  // reset timer1 = 0
  TCNT1 = 0;
   
  // clear register before setting bits 
  TCCR1B = 0;

  // set up timer1 with prescale = 8 
  TCCR1B |= 1 << CS11;

  // set value of output compare register A to compare timer1 against
  OCR1A = 1999;

  // set arduino to CTC mode (clear timer on compare)
  TCCR1B |= 1 << WGM12;

  // set control bit to enable compare match A interrupt (comparing against OCR1A)
  TIMSK1 |= (1 << OCIE1A);

  pinMode(dataInPin, INPUT);         // Set input pin for receiving data bits
  lcd.begin(16, 2);                  // Initialize 16x2 LCD display
  lcd.print("Waiting...");          // Initial placeholder message

  interrupts(); // enable interrupts
  
  Serial.begin(9600);               // Optional: for serial debugging output
}


// ------------------------------------------------------------------------
// readBit(): Reads one bit from the data line at the correct time.
// Returns: HIGH (1) or LOW (0)
// ------------------------------------------------------------------------
bool readBit() {
  bool bit = digitalRead(dataInPin);
  pause(2);  // Complete bit duration (2 ms total)
  return bit;
}

// ------------------------------------------------------------------------
// readByte(): Reads 8 bits (MSB-first) and constructs a full byte (0–255).
// Calls readBit() 8 times and assembles a single byte.
// ------------------------------------------------------------------------
byte readByte() {
  byte result = 0;
  for (int i = 7; i >= 0; i--) {
    bool bitVal = readBit();
    result |= (bitVal << i);  // Place the bit at the correct position
  }
  return result;
}

// ------------------------------------------------------------------------
// readUint16(): Reads two bytes (MSB first, then LSB) and reconstructs a
// full 16-bit unsigned integer (uint16_t: 0–65535).
// ------------------------------------------------------------------------
uint16_t readUint16() {
  byte msb = readByte();         // First byte (high-order bits)
  byte lsb = readByte();         // Second byte (low-order bits)
  return (msb << 8) | lsb;       // Combine to form 16-bit value
}


// ------------------------------------------------------------------------
// waitForStartBit(): Waits for a HIGH start pulse from Arduino A,
// then synchronizes timing before beginning to read bits.
// ------------------------------------------------------------------------ 
// --> tells Arduino B “NOW is the time to start reading the actual data bits”. 
// --> You and Arduino A have no shared clock, so you must use a timing-based “handshake”, and this function handles that.
void waitForStartBit() {
  // Wait for rising edge of start bit
  while (digitalRead(dataInPin) != HIGH);
  pause(5);  // Wait full duration of start bit
  // Wait for falling edge (start bit ends)
  while (digitalRead(dataInPin) == HIGH);
  pause(1);  // Stabilize before first bit
}


// ------------------------------------------------------------------------
// updateLCD(): Updates the 16x2 LCD display with the received values.
// Line 1: "P1:<score>  P2:<score>"
// Line 2: "Rounds:<round_number>"
// ------------------------------------------------------------------------
void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);  // Line 1, start
  lcd.print("P1:" + String(player1_score, 1));
  lcd.setCursor(8, 0);  // Line 1, middle
  lcd.print("P2:" + String(player2_score, 1));
  lcd.setCursor(0, 1);  // Line 2
  lcd.print("Rounds:" + String(total_rounds_played));
}



void loop() {

  waitForStartBit();  // Sync with transmission from Arduino A

  player1_score_raw = readUint16();       // Score x10
  player2_score_raw = readUint16();       // Score x10
  total_rounds_played = readUint16();

  // Convert raw integer scores back to float
  player1_score = player1_score_raw / 10.0;
  player2_score = player2_score_raw / 10.0;


  updateLCD();  // Show data on LCD


  // Debug output
  Serial.print("P1: "); Serial.println(player1_score, 1);
  Serial.print("P2: "); Serial.println(player2_score, 1);
  Serial.print("Rounds: "); Serial.println(total_rounds_played);

  pause(100);


}






