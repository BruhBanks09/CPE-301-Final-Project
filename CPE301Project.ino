#include <LiquidCrystal.h>
#include "Arduino.h"
#include "uRTCLib.h"
#include <Stepper.h>
#include <DHT.h>

//LCD -------------------
LiquidCrystal lcd(31, 33, 35, 37, 39, 41);

//Real Time Clock -----------------
uRTCLib rtc(0x68);
char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

//Stepper Motor ----------------
const int stepsPerRevolution = 2048;
Stepper myStepper(stepsPerRevolution, 7, 6, 5, 4);

#define VENT_POT A0
int lastVentPos = -1;


//Humidity Sensor --------------
#define DHTPIN 2         // Pin connected to DHT sensor
#define DHTTYPE DHT11    // Change to DHT22 if needed
DHT dht(DHTPIN, DHTTYPE);


// Fan --------------
#define FAN_PIN 9

// Water Sensor --------------
#define WATER_SENSOR A8

// LEDS --------------
#define LED_DISABLED 22
#define LED_IDLE     23
#define LED_ERROR    24
#define LED_RUNNING  25

// Buttons ----------------
#define START_BUTTON 3
#define STOP_BUTTON  18
#define RESET_BUTTON 19

// Thresholds -------------
#define TEMP_THRESHOLD 24.0
#define WATER_THRESHOLD 140

// States
enum State {DISABLED, IDLE, ERROR, RUNNING};
State currState = DISABLED;

volatile bool startPressed = false;

void startISR() {
  startPressed = true;
}


void setup() {
Serial.begin(9600);
 URTCLIB_WIRE.begin();
  //Startup ---------------
  lcd.begin(16, 2);   
  lcd.print("Initializing...");   
  dht.begin();
  myStepper.setSpeed(15);


  pinMode(FAN_PIN, OUTPUT);
  pinMode(WATER_SENSOR, INPUT);

  pinMode(START_BUTTON, INPUT);
  pinMode(STOP_BUTTON, INPUT);
  pinMode(RESET_BUTTON, INPUT);

  attachInterrupt(digitalPinToInterrupt(START_BUTTON), startISR, FALLING);

  pinMode(LED_DISABLED, OUTPUT);
  pinMode(LED_IDLE, OUTPUT);
  pinMode(LED_ERROR, OUTPUT);
  pinMode(LED_RUNNING, OUTPUT);

  updateState(DISABLED);

}

void loop() {
  rtc.refresh();

  float temp = dht.readTemperature();
  float hum =  dht.readHumidity();
  int waterLvl = analogRead(WATER_SENSOR);

  if (isnan(temp) || isnan(hum)) return;

  // Start button
  if (startPressed && currState == DISABLED) {
    startPressed = false;
    updateState(IDLE);
  }

  // Stop Button
  if (digitalRead(STOP_BUTTON) == LOW && currState != ERROR) {
    updateState(DISABLED);
    return;
  } 

  // Water Level

  if (currState != DISABLED && waterLvl < WATER_THRESHOLD) {
    updateState(ERROR);
    return;
  }

  // Reset Button
  if (digitalRead(RESET_BUTTON) == LOW && currState == ERROR && analogRead(WATER_SENSOR) >= WATER_THRESHOLD) {
    updateState(IDLE);
    return;
  }




  // LCD usual:
  if (currState != ERROR && currState != DISABLED) {
    lcd.setCursor(0, 0);
    lcd.print("Temp: "); lcd.print(temp); lcd.print((char)223); /* degree symbol */ lcd.print("C   ");
    lcd.setCursor(0, 1);
    lcd.print("Hum: "); lcd.print(hum); lcd.print("%    ");
  }

  if (currState != DISABLED) {
    int potVal = analogRead(VENT_POT);
    int targetPos = map(potVal, 0, 1023, 0, stepsPerRevolution);

    if (abs(targetPos - lastVentPos) > 10) {
      int diff = targetPos - lastVentPos;
      myStepper.step(diff);
      lastVentPos = targetPos;

      rtc.refresh();
      
      Serial.print("\n");
      Serial.print("[Vent Moved At @ ");
      Serial.print(rtc.hour()); Serial.print(":");
      Serial.print(rtc.minute()); Serial.print(":");
      Serial.print(rtc.second()); Serial.print("]");
      Serial.print("\n");
    }
  }


  // State Logic
  switch(currState) {
    case IDLE:
      if (temp > TEMP_THRESHOLD) {
        updateState(RUNNING);
      }
      break;
    case RUNNING:
      digitalWrite(FAN_PIN, HIGH);
      if (temp <= TEMP_THRESHOLD) {
        digitalWrite(FAN_PIN, LOW);
        updateState(IDLE);
      }
      break;
    case ERROR:
    case DISABLED:
      digitalWrite(FAN_PIN, LOW);
      break;
  }
}

void updateState(State newState) {
  currState = newState;
  rtc.refresh();

  Serial.print("\n");
  Serial.print("[STATE CHANGE @ ");
  Serial.print(rtc.hour()); Serial.print(":");
  Serial.print(rtc.minute()); Serial.print(":");
  Serial.print(rtc.second()); Serial.print("]");
  Serial.print("\n");

  digitalWrite(LED_DISABLED, newState == DISABLED);
  digitalWrite(LED_IDLE, newState == IDLE);
  digitalWrite(LED_ERROR, newState == ERROR);
  digitalWrite(LED_RUNNING, newState == RUNNING);
  
  switch(newState) {
    case DISABLED:
      lcd.clear();
      lcd.print("System Disabled");
      break;
    case IDLE:
      lcd.clear();
      lcd.print("System Idle");
      break;
    case ERROR:
      lcd.clear();
      lcd.print("System Error: No Water");
      break;
    case RUNNING:
      lcd.clear();
      lcd.print("Cooling Active");
      break;
  }
}

  