#include <Arduino.h>
#include <IRremote.hpp>

// ==================== IR BUTTON CODES ====================
uint8_t CMD_START_PAUSE   = 0x43;  // start/pause button
uint8_t CMD_RESET         = 0x44;  // reset button
uint8_t CMD_ADD_10_MIN    = 0x15;  // +10 minutes button
uint8_t CMD_SUB_10_MIN    = 0x07;  // -10 minutes button

// ==================== 7-SEGMENT DISPLAY PINS ====================
#define PIN_A 2
#define PIN_B 3
#define PIN_C 4
#define PIN_D 5
#define PIN_E 6
#define PIN_F 7
#define PIN_G 8

#define DIGIT1 9
#define DIGIT2 10
#define DIGIT3 11
#define DIGIT4 12
#define PIN_DP A5
#define IR_RECEIVE_PIN 13

// Segment patterns for digits 0..9
const bool DIGITS[10][7] = {
  {1,1,1,1,1,1,0}, //0
  {0,1,1,0,0,0,0}, //1
  {1,1,0,1,1,0,1}, //2
  {1,1,1,1,0,0,1}, //3
  {0,1,1,0,0,1,1}, //4
  {1,0,1,1,0,1,1}, //5
  {1,0,1,1,1,1,1}, //6
  {1,1,1,0,0,0,0}, //7
  {1,1,1,1,1,1,1}, //8
  {1,1,1,1,0,1,1}  //9
};

const int segmentPins[7] = {PIN_A, PIN_B, PIN_C, PIN_D, PIN_E, PIN_F, PIN_G};
const int digitPins[4]   = {DIGIT1, DIGIT2, DIGIT3, DIGIT4};

// ==================== TIMER STATE ====================
unsigned long lastUpdate = 0;
int minutes = 0;
int seconds = 0;
bool running = false;
bool initialState = true; // shows horizontal bars at start-up
bool finished = false;    // the countdown has finished
unsigned long blinkTimer = 0;
bool blinkState = false;
unsigned long lastCmdTime = 0;       // time of the last IR command
const unsigned long cmdDelay = 300;

// ==================== DISPLAY FUNCTIONS ====================
void showDigit(int num, int digit, bool dp = false) {
  for (int i = 0; i < 4; i++) digitalWrite(digitPins[i], HIGH);

  for (int s = 0; s < 7; s++) {
    // common cathode
    digitalWrite(segmentPins[s], DIGITS[num][s] ? HIGH : LOW);
  }
  if (dp) digitalWrite(PIN_DP, HIGH); // decimal point on
  else digitalWrite(PIN_DP, LOW);
  digitalWrite(digitPins[digit], LOW);
  delay(2);
  digitalWrite(digitPins[digit], HIGH);
}

void displayNumber(int value) {
  int d[4];
  d[0] = (value / 1000) % 10;
  d[1] = (value / 100) % 10;
  d[2] = (value / 10) % 10;
  d[3] = value % 10;

  for (int i = 0; i < 4; i++) {
    showDigit(d[i], i,i ==1);
  }
}

// Shows horizontal bars (segments A and D on)
void displayBars() {
  for (int i = 0; i < 4; i++) {
    for (int s = 0; s < 7; s++) {
      if (s == 0 || s == 3) digitalWrite(segmentPins[s], HIGH); // A and D on
      else digitalWrite(segmentPins[s], LOW);                     // the rest off
    }
    digitalWrite(digitPins[i], LOW);
    delay(2);
    digitalWrite(digitPins[i], HIGH);
  }
}

// Blinks the whole display when the countdown ends
void displayFinished() {
  if (millis() - blinkTimer >= 500) {
    blinkTimer = millis();
    blinkState = !blinkState;
  }

  for (int i = 0; i < 4; i++) {
    for (int s = 0; s < 7; s++) {
      digitalWrite(segmentPins[s], blinkState ? HIGH : LOW);
    }
    digitalWrite(digitPins[i], LOW);
    delay(2);
    digitalWrite(digitPins[i], HIGH);
  }
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 7; i++) pinMode(segmentPins[i], OUTPUT);
  for (int i = 0; i < 4; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], HIGH);
  }
  pinMode(PIN_DP, OUTPUT);
  digitalWrite(PIN_DP, LOW);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  Serial.println(F("Ready. Use the remote to control the timer."));
}

// ==================== LOOP ====================
void loop() {
  if (finished) {
    displayFinished();
  } else if (initialState) {
    displayBars();
  } else {
    int displayValue = minutes * 100 + seconds;
    displayNumber(displayValue);
  }

  // Countdown
  if (!initialState && running && !finished && millis() - lastUpdate >= 1000) {
    lastUpdate = millis();
    seconds--;
    if (seconds < 0) {
      if (minutes > 0) {
        minutes--;
        seconds = 59;
      } else {
        minutes = 0;
        seconds = 0;
        running = false;
        finished = true;
      }
    }
  }

  // IR input
  if (IrReceiver.decode()) {
    if (IrReceiver.decodedIRData.protocol != UNKNOWN) {
      uint8_t cmd = IrReceiver.decodedIRData.command;
      if (millis() - lastCmdTime < cmdDelay) {
        IrReceiver.resume();
        return;
      }
      lastCmdTime = millis();
      
      Serial.print(F("Command received: 0x"));
      Serial.println(cmd, HEX);

      if (cmd == CMD_START_PAUSE ) {
        if(finished || initialState){
          minutes = 60;
          seconds = 0;
          initialState = false;
          finished = false;
          running = false;
          return;
        }
        running = !running;
        Serial.println(running ? F("Timer started") : F("Timer paused"));
      }
      else if (cmd == CMD_RESET) {
        running = false;
        minutes = 0;
        seconds = 0;
        initialState = true;
        finished = false;
        Serial.println(F("Timer reset"));
      }
      else if (cmd == CMD_ADD_10_MIN) {
        if (initialState) {
          minutes = 60;
          seconds = 0;
          initialState = false;
        } else {
          if (minutes < 10) minutes++;
          else if(minutes < 99 && minutes > 90) minutes ++;
          else minutes += 10;
          if (minutes > 99) minutes = 99;
        }
        finished = false;
        Serial.println(F("+ time"));
      }
      else if (cmd == CMD_SUB_10_MIN) {
        if (initialState) {
          minutes = 60;
          seconds = 0;
          initialState = false;
        } else {
          if (minutes <= 10) minutes--;        // subtract 1 minute below 10
          else if(minutes <= 99 && minutes > 90) minutes --; 
          else minutes -= 10;
          if (minutes < 0) minutes = 0;
        }
        finished = false;
        Serial.println(F("- time"));
      }
    }
    IrReceiver.resume();
  }
}
