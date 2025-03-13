// global variables 
volatile unsigned long gdMillis = 0;
const int interruptPin = 2;
const byte LSB = 12;
const byte MSB = 13;
const int p1LED_1 = 5;
const int p1LED_2 = 6;
const int p2LED_1 = 8;
const int p2LED_2 = 9;
bool scale = false;
unsigned long LEDStartTimeP1 = 0;
unsigned long LEDStartTimeP2 = 0;
int randomLEDTimerP1 = 0;
int randomLEDTimerP2 = 0;
bool LEDTimerExpiredP1 = false;
bool LEDTimerExpiredP2 = false;
bool roundInProgress = false;
volatile int randomLEDP1 = 0;
volatile int randomLEDP2 = 0;
int numRounds = 1;
int P1RxnTime = 0;
int P2RxnTime = 0;
volatile unsigned long rxnStartTimeP1 = 0;
volatile unsigned long rxnStartTimeP2 = 0;
int totalRxnTimeP1 = 0;
int totalRxnTimeP2 = 0;
bool P1Lost = false;
bool P2Lost = false;


void ISRTrigger() { 
  int button = getButton(MSB, LSB); // get button that was pushed 
  validateButtonPress(button, randomLEDP1, randomLEDP2); // make sure the button was correct and not pushed premautrely  
  if (!P1Lost && !P2Lost) { // clock reaction time 
    if (button == 0 || button == 1) {totalRxnTimeP1 += gdMills() - rxnStartTimeP1;}
    else if (button == 2 || button == 3) {totalRxnTimeP2 += gdMills() - rxnStartTimeP2;}
  }
}

void validateButtonPress(int button, int ledP1, int ledP2) { // general function to validate button press from any player 
  if (button == 0 || button == 1) {
    validateP1ButtonPress(button, ledP1);
  }
  else if (button == 2 || button == 3) {
    validateP2ButtonPress(button, ledP2);
  }
}

void validateP1ButtonPress(int button, int led) { 
  if (!LEDTimerExpiredP1) {P1Lost = true;}
  else if (led - button != 5) {P1Lost = true;}
}

void validateP2ButtonPress(int button, int led) { 
  if (!LEDTimerExpiredP2) {P2Lost = true;}  
  else if (led - button != 6) {P2Lost = true;}
}

float randomFloat(float min, float max) {
  if (scale) { // only start scaling after the first round
    float intervalWidth = max - min; // get width/range of where we want our value to lie (ie if range is 0.1 to 0.99 width = 0.89)
    float randomFraction = random(0, 101) / 100.0; // get a number between 0.0 and 1.0
    // return the random number. ie: 
    // min + (width * randomFraction) 
    // = 0.1 + 0.89*[0.0 to 1.0]
    // = 0.1 + [0.0, 0.89]
    // = [0.1 to 0.99]
    return min + (intervalWidth * randomFraction); 
  }
  else{return 1.0;}
}

int pickRandomLED (int led1, int led2) {
  return random(led1, led2 + 1);
}

int getButton(byte msb, byte lsb) {
  // Read the state of the two encoder pins
  bool A = digitalRead(msb);
  bool B = digitalRead(lsb);

  // Combine A and B into a 2-bit binary number
  int value = (A << 1) | B;  // A is the MSB, B is the LSB

  return value;
}

void pause(int cycles) {
  for (int i = 0; i < cycles; i++) {asm volatile("nop");} // do nothing to kill time. include assembly no operation instruction to avoid being optimized away
}

uint16_t generateRandomSeed(int analogPin = 0, int cycleCount = 200) {
  uint16_t seed = 0;
  for (int i = 0; i < 6; i++) {
    seed = (seed << 3) | (analogRead(analogPin) & 0x07); 
    pause(cycleCount); // give time for the lower bits of the voltage reading to fluctuate (about 1ms)
  }
  return seed;
}

void startNewRound() {
  int roundSeed = generateRandomSeed();
  randomSeed(roundSeed);

  float randomScalerP1 = randomFloat(0.5, 0.99);
  float randomScalerP2 = randomFloat(0.5, 0.99);

  randomLEDTimerP1 = random(200, 2001 * randomScalerP1); // should we do the same scaler for both players so that one player isnt at a disadvantage if their scaler is very small?
  randomLEDTimerP2 = random(200, 2001 * randomScalerP2);

  randomLEDP1 = pickRandomLED(p1LED_1, p1LED_2);
  randomLEDP2 = pickRandomLED(p2LED_1, p2LED_2);

  LEDStartTimeP1 = gdMills();
  LEDStartTimeP2 = gdMills();
  
  digitalWrite(randomLEDP1, LOW);
  digitalWrite(randomLEDP2, LOW);

  scale = true; // enable scaling for next round
  roundInProgress = true;
}


unsigned long gdMills() {return gdMillis;}

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

  startNewRound();
}

void loop() {
  if (roundInProgress) {
    if (!LEDTimerExpiredP1 && gdMills() - LEDStartTimeP1 >= randomLEDTimerP1) {
      LEDTimerExpiredP1 = true; // set flag 
      digitalWrite(randomLEDP1, HIGH); // turn LED on
      rxnStartTimeP1 = gdMills(); // start reaction timer
      // do more stuff
    }

    if (!LEDTimerExpiredP2 && gdMills() - LEDStartTimeP2 >= randomLEDTimerP2) {
      LEDTimerExpiredP2 = true;
      digitalWrite(randomLEDP2, HIGH); // turn LED on
      rxnStartTimeP2 = gdMills(); // start reaction timer
      // do more stuff 
    }
  }

  if (LEDTimerExpiredP1 && LEDTimerExpiredP2) {
      pause(100000); // delay between rounds (dont really need this)
      roundInProgress = false;
      startNewRound(); // start next round
    }

    // TODO: 1. exit game if a player loses 2. save times for round, game, and individual rxn times of each player 3. idle state 4. single player mode 5. class????
   
}

















