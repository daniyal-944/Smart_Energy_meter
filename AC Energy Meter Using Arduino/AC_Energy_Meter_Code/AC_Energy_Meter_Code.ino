#include "EmonLib.h"           // Include EmonLib for energy monitoring
#include <Wire.h>               // Include Wire library for I2C communication
#include <LiquidCrystal_I2C.h>  // Include LiquidCrystal_I2C library for LCD display

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Initialize LCD with I2C address 0x27 and size 16x2

// Calibration constants for the voltage and current sensors
const float vCalibration = 96;     // Voltage calibration factor
const float currCalibration = 1.33; // Current calibration factor

EnergyMonitor emon;  // Create an instance of EnergyMonitor

// Variables to store energy consumption
float power = 0.0;           // Power consumed by the load in Watts
unsigned long lastMillis = millis(); // Time to calculate energy
float energyConsumed = 0.0; // Energy consumed in Wh
const float voltageThreshold = 5.0; // Threshold for detecting AC input

#define SMOOTHING 10  // Number of samples to average
float voltageSamples[SMOOTHING];
float currentSamples[SMOOTHING];
int sampleIndex = 0;

void setup() {
  // Initialize the LCD
  lcd.init();           // Initialize the LCD
  lcd.backlight();      // Turn on LCD backlight

  // Setup voltage and current inputs for energy monitoring
  emon.voltage(35, vCalibration, 1.7);  // Configure voltage measurement: input pin, calibration, phase shift
  emon.current(34, currCalibration);   // Configure current measurement: input pin, calibration

  // Display zero readings during calibration
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.setCursor(0, 1);
  // Calibration delay
  delay(2000);  // Wait 2 seconds for the system to stabilize
}

void loop() {
  // Calculate voltage and current
  emon.calcVI(100, 2000);  // Number of half wavelengths (crossings), time-out

  // Store current readings in arrays for smoothing
  voltageSamples[sampleIndex] = emon.Vrms;
  currentSamples[sampleIndex] = emon.Irms;
  
  sampleIndex++;
  if (sampleIndex >= SMOOTHING) {
    sampleIndex = 0;  // Wrap around after the buffer is full
  }
  
  // Calculate the moving average for voltage and current
  float avgVoltage = 0.0;
  float avgCurrent = 0.0;
  for (int i = 0; i < SMOOTHING; i++) {
    avgVoltage += voltageSamples[i];
    avgCurrent += currentSamples[i];
  }
  avgVoltage /= SMOOTHING;
  avgCurrent /= SMOOTHING;

  // Check if the voltage is below the threshold (no AC input or load connected)
  if (avgVoltage < voltageThreshold) {
    avgVoltage = 0.0;
    avgCurrent = 0.0;
    power = 0.0;
    energyConsumed = 0.0; // Reset energy consumed
  } else {
    // Calculate power if AC input is detected
    power = avgVoltage * avgCurrent;
    
    // Update energy consumed every second
    unsigned long currentMillis = millis();
    if (currentMillis - lastMillis >= 1000) {
      energyConsumed += power / 3600; // Power in watts, convert to Wh
      lastMillis = currentMillis;
    }
  }

  // Update the LCD display instantly
  lcd.clear();                 // Clear LCD display

  // Display voltage and current on the first row
  lcd.setCursor(0, 0);         // Set cursor to the first row
  lcd.print("V:");             // Print "V:" label
  lcd.print(avgVoltage, 1);    // Print the voltage with 1 decimal precision
  lcd.print("V ");             // Print "V" label with space

  lcd.print("I:");             // Print "I:" label
  lcd.print(avgCurrent, 2);    // Print the current with 1 decimal precision
  lcd.print("A");              // Print "A" label

  // Display power and energy on the second row
  lcd.setCursor(0, 1);         // Set cursor to the second row
  lcd.print("P:");             // Print "P:" label
  lcd.print(power, 1);         // Print the power consumed in watts
  lcd.print("W ");             // Print "W" label with space

  lcd.print("E:");             // Print "E:" label
  lcd.print(energyConsumed, 2); // Print energy consumed in Wh (2 decimal places)
  lcd.print("Wh");             // Print "Wh" label

  delay(100); // Short delay for stability
}
