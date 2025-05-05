#include <LiquidCrystal.h>
#include "Arduino.h"
#include "uRTCLib.h"
#include <Stepper.h>
#include <DHT.h>


//THRESHOLD TEMP about 24Celcius
//THRESHOLD WATERLVL 








//LCD -------------------
LiquidCrystal lcd(31, 33, 35, 37, 39, 41);

//Real Time Clock -----------------
uRTCLib rtc(0x68);
char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

//Stepper Motor ----------------
const int stepsPerRevolution = 2048;
Stepper myStepper(stepsPerRevolution, 7, 6, 5, 4);

//Humidity Sensor --------------
#define DHTPIN 2         // Pin connected to DHT sensor
#define DHTTYPE DHT11    // Change to DHT22 if needed
DHT dht(DHTPIN, DHTTYPE);


void setup() {
Serial.begin(9600);
 URTCLIB_WIRE.begin();
  //lcd code ---------------
  lcd.begin(16, 2);              // Set up the LCD's number of columns and rows
  lcd.print("Hello, World!");    // Print message to the LCD

  //RTC Code ----------------
  rtc.set(0, 15, 4, 1, 4, 5, 25);
  // rtc.set(second, minute, hour, dayOfWeek, dayOfMonth, month, year)
  // set day of week (1=Sunday, 7=Saturday)

  //Stepper Motor ------------
  myStepper.setSpeed(15);  // You can adjust the speed here
  Serial.begin(9600);

  //LCD W Humid Sensor ----------------
  dht.begin();
  lcd.begin(16, 2);  // Set up 16 columns and 2 rows
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();

  //Water Sensor(Serial Monitor)

}

void loop() {
  //RTC Loop Code ------------------
   rtc.refresh();

  Serial.print("Current Date & Time: ");
  Serial.print(rtc.year());
  Serial.print('/');
  Serial.print(rtc.month());
  Serial.print('/');
  Serial.print(rtc.day());

  Serial.print(" (");
  byte dow = rtc.dayOfWeek();
  if(dow >= 1 && dow <= 7){
    Serial.print(daysOfTheWeek[dow - 1]);
  } else {
    Serial.print("Unknown");
  }
  Serial.print(") ");

  Serial.print(rtc.hour());
  Serial.print(':');
  Serial.print(rtc.minute());
  Serial.print(':');
  Serial.println(rtc.second());

  delay(5000);

  //Stepper Motor Code ----------------
  // Rotate one full revolution clockwise:
  myStepper.step(stepsPerRevolution);
  delay(1000);

  // Rotate one full revolution counterclockwise:
  myStepper.step(-stepsPerRevolution);
  delay(1000);

  //LCD with Humid Sensor -----------------
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // Check if reading failed
  if (isnan(temp) || isnan(hum)) {
    lcd.setCursor(0, 0);
    lcd.print("Sensor error   ");
    return;
  }

  // Display Temperature
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print((char)223); // degree symbol
  lcd.print("C");

  // Display Humidity
  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(hum);
  lcd.print("%");

  delay(500);

  //Water Sensor
  int sensorValue = analogRead(A8);
  Serial.print("Water Level Reading: ");
  Serial.println(sensorValue);

  if(sensorValue > 220){
    Serial.println("Water Level: HIGH");
  } else if (sensorValue > 140){
    Serial.println("Water Level: MEDIUM");
  } else {
    Serial.println("Water Level: LOW");
  }

  delay(500);
}