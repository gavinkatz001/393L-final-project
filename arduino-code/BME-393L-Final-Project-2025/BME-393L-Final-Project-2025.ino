// global variables 
volatile unsigned long gdMillis = 0; // variable to keep track of time

// pins to be used on the arduino board 
const int interruptPin = 2;
const byte LSB = 11;
const byte MSB = 12;
const int p1LED_1 = 5;
const int p1LED_2 = 6;
const int p2LED_1 = 8;
const int p2LED_2 = 9;
const int idleLED = 10;
const int idleButton = 4;
const int dataOutPin = 7;  // communication pin to talk to ard B


// flags
bool LEDTimerExpiredP1 = false;
bool LEDTimerExpiredP2 = false;
bool P1Lost = false;
bool P2Lost = false;
bool roundInProgress = false; 
bool P1HasReacted = false;
bool P2HasReacted = false;
bool gameIsOver = true;

// time-related variables 
unsigned long LEDStartTimeP1 = 0;
unsigned long LEDStartTimeP2 = 0;
unsigned long roundStartTime = 0;
unsigned long gameStartTime = 0;
int randomLEDTimerP1 = 0;
int randomLEDTimerP2 = 0;
int P1RxnTimes[maxRounds] = {0}; // this is the per-round rxn time. 
int P2RxnTimes[maxRounds] = {0}; // this is the per-round rxn time. 
int roundTimes[maxRounds] = {0}; // time the rounds 
unsigned long rxnStartTimeP1 = 0;
unsigned long rxnStartTimeP2 = 0;
int totalRxnTimeP1 = 0;
int totalRxnTimeP2 = 0;
int totalGameTime = 0;



// volatile variables 
volatile int buttonValue = -1;
volatile bool buttonPressed = false;

// other
const int maxRounds = 7;
bool scale = false;
int numRounds = 0;
float P1Score = 0.0; // TO BE IMPLEMENTED ON ARD2
float P2Score = 0.0; // TO BE IMPLEMENTED ON ARD2
int randomLEDP1 = 0;
int randomLEDP2 = 0;
int windowEnd = 2001;


// --------------------------------------------- //
// GAME FUNCTIONS 
// --------------------------------------------- //
void ISRTrigger() { 
  buttonValue = getButton(MSB, LSB); // get button that was pushed 
  buttonPressed = true;
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

void checkForDoublePress(int button) {
  if (button == 0 || button == 1) {
    if (P1HasReacted) {
      P1Lost = true;
      roundInProgress = false;
    } 
    else {
      P1HasReacted = true;
    }
  }
  if (button == 2 || button == 3) {
    if (P2HasReacted) {
      P2Lost = true;
      roundInProgress = false;
    } 
    else {
      P2HasReacted = true;
    }
  }
}

void handleButtonPress() {
  if (buttonPressed) {
      // reset flag
      buttonPressed = false;

      checkForDoublePress(buttonValue); // check if button for a given player was already pressed this round

      // check that neither player has lost via double clicking 
      if (!P1Lost && !P2Lost) {
        // process button press
        validateButtonPress(buttonValue, randomLEDP1, randomLEDP2); // make sure the button was correct and not pushed premautrely  

        // clock reaction time 
        if (!P1Lost && !P2Lost) { 
          if (buttonValue == 0 || buttonValue == 1) {
            int roundRxnTimeP1 = gdMills() - rxnStartTimeP1;
            P1RxnTimes[numRounds - 1] = roundRxnTimeP1;
            totalRxnTimeP1 += roundRxnTimeP1;
            digitalWrite(randomLEDP1, LOW);
            }
          else if (buttonValue == 2 || buttonValue == 3) {
            int roundRxnTimeP2 = gdMills() - rxnStartTimeP2;
            P2RxnTimes[numRounds - 1] = roundRxnTimeP2;
            totalRxnTimeP2 += roundRxnTimeP2;
            digitalWrite(randomLEDP2, LOW);
            }
        }
        else {
          roundInProgress = false;
          gameIsOver = true;
        }
      }
      else {
        roundInProgress = false;
        gameIsOver = true;
      } 
    }
  else {return;}
}

bool isGameOver() {
  if (numRounds == maxRounds) {
    return true;
  }
  else if (P1Lost || P2Lost) {
    return true;
  }
  return false;  
}

void getScores() {
  P1Score = (1.0 / (totalRxnTimeP1 / numRounds)) * 100.0;
  P2Score = (1.0 / (totalRxnTimeP2 / numRounds)) * 100.0;
} // to be implemented on ard2

void blinkFast(int ledPin) {
  digitalWrite(ledPin, LOW);
  pause(100);
  digitalWrite(ledPin, HIGH);
  pause(100);
  digitalWrite(ledPin, LOW);
  pause(100);
  digitalWrite(ledPin, HIGH);
  pause(100);
  digitalWrite(ledPin, LOW);
  pause(100);
  digitalWrite(ledPin, HIGH);
  pause(100);
  digitalWrite(ledPin, LOW);
  pause(100);
  digitalWrite(ledPin, HIGH);
  pause(100);
  digitalWrite(ledPin, LOW);
}

void idle() {
  if (!gameIsOver) {return;}  // do nothing if game is not over
  digitalWrite(p1LED_1, LOW);
  digitalWrite(p1LED_2, LOW);
  digitalWrite(p2LED_1, LOW);
  digitalWrite(p2LED_2, LOW);
  while (true) {
    digitalWrite(idleLED, HIGH);
    if (digitalRead(idleButton) == HIGH) {
      blinkFast(idleLED);
      resetGame();
      break;
    }
  }
}

void resetGame() {
  // Reset flags
  P1Lost = false;
  P2Lost = false;
  LEDTimerExpiredP1 = false;
  LEDTimerExpiredP2 = false;
  P1HasReacted = false;
  P2HasReacted = false;
  roundInProgress = false;
  gameIsOver = false;


  // Reset time-related variables
  LEDStartTimeP1 = 0;
  LEDStartTimeP2 = 0;
  roundStartTime = 0;
  rxnStartTimeP1 = 0;
  rxnStartTimeP2 = 0;

  // Reset scores and round counter
  numRounds = 0;
  P1Score = 0;
  P2Score = 0;
  totalRxnTimeP1 = 0;
  totalRxnTimeP2 = 0;
  totalGameTime = 0;
  windowEnd = 2001; 

  // Clear per-round arrays
  for (int i = 0; i < maxRounds; i++) {
    P1RxnTimes[i] = 0;
    P2RxnTimes[i] = 0;
    roundTimes[i] = 0;
  }

  // Turn off all LEDs
  digitalWrite(p1LED_1, LOW);
  digitalWrite(p1LED_2, LOW);
  digitalWrite(p2LED_1, LOW);
  digitalWrite(p2LED_2, LOW);

  // set game timer
  gameStartTime = gdMills();

  // Start fresh game
  startNewRound();
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
  else {return 1.0;}
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

void pause(int millisToDelay) {
  unsigned long startTime = gdMills();
  while (gdMills() - startTime < millisToDelay) {}  // do nothing to kill time. include assembly no operation instruction to avoid being optimized away
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

  if (isGameOver()) { // check if game needs to be ended before starting a new round
    roundInProgress = false;
    gameIsOver = true;
    totalGameTime = gdMills() - gameStartTime; // get the total game time
    sendPacket(totalRxnTimeP1, totalRxnTimeP2, numRounds); // if a game has been played, send data to ArdB
    printGameData(); // print the timing arrays 
    return;
  }
  
  // increase number of rounds
  numRounds++;

  // reset flags
  LEDTimerExpiredP1 = false; 
  LEDTimerExpiredP2 = false;
  P1HasReacted = false;
  P2HasReacted = false;
  
  // get random seed for this round
  int roundSeed = generateRandomSeed();
  randomSeed(roundSeed);
  
  // get random scalers
  float randomScaler = randomFloat(0.5, 0.99);

  // make the end of the time window smaller each round
  windowEnd = windowEnd * randomScaler;

  // reset timers
  randomLEDTimerP1 = random(200, windowEnd); 
  randomLEDTimerP2 = random(200, windowEnd);

  // pick a random LED for each player for the round
  randomLEDP1 = pickRandomLED(p1LED_1, p1LED_2);
  randomLEDP2 = pickRandomLED(p2LED_1, p2LED_2);

  // set the timers for the round
  LEDStartTimeP1 = gdMills();
  LEDStartTimeP2 = gdMills();
  roundStartTime = gdMills();

  scale = true; // enable scaling for next round
  roundInProgress = true;
}
// --------------------------------------------- //
// END OF GAME FUNCTIONS 
// --------------------------------------------- //


// --------------------------------------------- //
// COMMUNICATION PROTOCOL FUNCTIONS 
// --------------------------------------------- //

void sendBit(bool bitVal) {
  digitalWrite(dataOutPin, bitVal ? HIGH : LOW); // if bitVal = 1, send HIGH. else (aka bitVal = 0), send LOW
  pause(2);  // 2ms per bit
}

void sendByte(byte data) {
  for (int i = 7; i >= 0; i--) {
    bool bitVal = (data >> i) & 0x01;
    sendBit(bitVal);
  }
}

void sendUint16(uint16_t value) {
  byte msb = (value >> 8) & 0xFF;
  byte lsb = value & 0xFF;
  sendByte(msb);
  sendByte(lsb);
}

void sendStartBit() {
  digitalWrite(dataOutPin, HIGH);
  pause(5);  // 5ms HIGH = start pulse
  digitalWrite(dataOutPin, LOW);
  pause(1);  // settle
}


void sendPacket(uint16_t scaledP1, uint16_t scaledP2, uint16_t rounds) {
  // send start bit to initiate/sync communication between boards 
  sendStartBit();
  
  // send data 
  sendUint16(scaledP1);
  sendUint16(scaledP2);
  sendUint16(rounds);
}


void printGameData() {
  Serial.println("=== START GAME LOG ===");

  // Log P1 reaction times
  Serial.println("P1 Reaction Times (ms):");
  for (int i = 0; i < numRounds; i++) {
    if (P1RxnTimes[i] != 0) {
      Serial.println(P1RxnTimes[i]);
    }
  }

  // Log P2 reaction times
  Serial.println("\nP2 Reaction Times (ms):");
  for (int i = 0; i < numRounds; i++) {
    if (P2RxnTimes[i] != 0) {
      Serial.println(P2RxnTimes[i]);
    }
  }

  // Log round times
  Serial.println("\nRound Times (ms):");
  for (int i = 0; i < numRounds; i++) {
    if (roundTimes[i] != 0) {
      Serial.println(roundTimes[i]);
    }
  }

  // Log total game time
  Serial.println("\nTotal Game Time (s):");
  Serial.println( (float)(totalGameTime/1000) );

  Serial.println("=== END GAME LOG ===");
}

// --------------------------------------------- //
// END OF COMMUNICATION PROTOCOL FUNCTIONS 
// --------------------------------------------- //


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

  // set up pin modes
  pinMode(MSB, INPUT);
  pinMode(LSB, INPUT);
  pinMode(p1LED_1, OUTPUT);
  pinMode(p1LED_2, OUTPUT);
  pinMode(p2LED_1, OUTPUT);
  pinMode(p2LED_2, OUTPUT);
  pinMode(idleButton, INPUT);
  pinMode(idleLED, OUTPUT);
  pinMode(dataOutPin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPin), ISRTrigger, RISING); 

  interrupts(); // enable interrupts
  
  Serial.begin(9600); 

  // and we're off
  idle();
}

void loop() {

  if (gameIsOver) {
    idle();
    return;
  }
  if (roundInProgress) {

    handleButtonPress(); // in case players click button before light turns on

    if ((!LEDTimerExpiredP1) && (gdMills() - LEDStartTimeP1 >= randomLEDTimerP1)) {
      LEDTimerExpiredP1 = true; // set flag 
      rxnStartTimeP1 = gdMills(); // start reaction timer
      digitalWrite(randomLEDP1, HIGH); // turn LED on
      // do more stuff
    }

    handleButtonPress();

    if ((!LEDTimerExpiredP2) && (gdMills() - LEDStartTimeP2 >= randomLEDTimerP2)) {
      LEDTimerExpiredP2 = true; // set flag
      rxnStartTimeP2 = gdMills(); // start reaction timer
      digitalWrite(randomLEDP2, HIGH); // turn LED on
      // do more stuff 
    }

     handleButtonPress();
     // each time we call handleButtonPress(), we are checking if a button was pressed, validating the press if it happened, and logging rxn time after validation. otherwise, continue the loop function
  }

  if (LEDTimerExpiredP1 && LEDTimerExpiredP2 && P1HasReacted && P2HasReacted) {
      roundInProgress = false;
      roundTimes[numRounds - 1] = gdMills() - roundStartTime;
      startNewRound(); // start next round
    }
    // TODO: 1. exit game if a player loses 2. save times for round, game, and individual rxn times of each player 3. idle state 4. single player mode 5. class???? 6. scoring 7. debounce handling
}

/* 

to send to ard2:
- numRounds -> total rounds played
- totalRxnTimeP1 -> cumulative rxn time
- totalRxnTimeP2 -> cumulative rxn time

to write to a file:

- P1RxnTimes array -> this is the per-round rxn time. 
- P2RxnTimes array -> this is the per-round rxn time. 
- roundTimes array -> round times
- totalGameTime -> total elasped game time
*/ 



 

