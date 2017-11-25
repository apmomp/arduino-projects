#include <LiquidCrystal.h>
#include "DHT.h"
#define DHT_TYPE DHT11

/* pin configuration */
const int dhtPin = 4;
const int relePin = 5;

/* constants */
const int onOffBtn = 2;
const int tempBtn = 3;
const int graceTemp = 1;

/* Control variables */
long lastButtonPressTimestamp = millis();
int desiredTemp = 18;
boolean boilerEnabled = false;
boolean active = false;
int tempC;
boolean tempCInitialized = false;

/* init dht and Liquid Cristal */
DHT dht(dhtPin, DHT_TYPE); // Configure DHT sensor
LiquidCrystal lcd(7,8,9,10,11,12);

void setup() {
  lcd.begin(16,2);
  lcd.print("Thermostat Ctl");
  lcd.setCursor(0,1);
  lcd.print("by Apmomp :D");
  delay(2000);
  
  pinMode(onOffBtn, INPUT_PULLUP);
  pinMode(tempBtn, INPUT_PULLUP);
  pinMode(relePin, OUTPUT);
}

void readTemperature() {
  if (tempCInitialized) {
    handleControlsFor(2000); // We should wait 2s between measurements
  }

  float t = dht.readTemperature();

  if (!isnan(t)) {
    if (!tempCInitialized) {
      tempCInitialized = true;
    }
    tempC = t;
  }
  
}

void updateLcd() {
  lcd.noBlink();
  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,0);
  if (active) {
    lcd.print("Desired: ");
    lcd.print(desiredTemp);
    lcd.print((char)223);
    lcd.print("C");
    
  } else {
    lcd.print("OFF ");
  }

  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("Current: ");
  lcd.print(tempC);
  lcd.print((char)223);
  lcd.print("C");

  /* Print blink cursor when boiler is turned on */
  if (boilerEnabled) {
    lcd.setCursor(15,0);
    lcd.blink();
  }
}

void handleBoiler() {
  if (active) {
    if (boilerEnabled && tempC >= desiredTemp) {
      boilerEnabled = false;
    } else if (!boilerEnabled && tempC < desiredTemp - graceTemp) {
      boilerEnabled = true;
    }  
  } else if (boilerEnabled) {
    boilerEnabled = false;
  }

  if (boilerEnabled) {
    digitalWrite(relePin, HIGH);
  } else {
    digitalWrite(relePin, LOW);  
  }
}

/* Avoid press buttons multiple times */
boolean areButtonsLocked() {
  return millis() > lastButtonPressTimestamp && millis() < lastButtonPressTimestamp + 500;
}

/* We want wait some seconds and listen input buttons */
void handleControlsFor(int ms) {
  int i;
    for(i=0;i<ms;i++) {
      btnControl();
      delay(1);
    }
}

void btnControl() {
  if (digitalRead(onOffBtn) == LOW && !areButtonsLocked()) {
    lastButtonPressTimestamp = millis();
    active = !active;
    updateLcd();
  }
  
  
  if (active && digitalRead(tempBtn) == LOW && !areButtonsLocked()) {
    lastButtonPressTimestamp = millis();
    desiredTemp++;
    if (desiredTemp > 24) {
      desiredTemp = 18;
    }
    updateLcd();
  }
  
}

void loop() {
  readTemperature();
  if (tempCInitialized) {
    updateLcd();
    handleBoiler();
  }  
}
