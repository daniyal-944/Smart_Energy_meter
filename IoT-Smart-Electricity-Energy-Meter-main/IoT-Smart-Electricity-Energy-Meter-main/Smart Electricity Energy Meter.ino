#define BLYNK_TEMPLATE_ID "TMPL3Bi5UkH45"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_PRINT Serial
 
#include "EmonLib.h"
#include <EEPROM.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
// #include <LiquidCrystal_I2C.h>
// LiquidCrystal_I2C lcd(0x27, 20, 4);
 
// Constants for calibration
const float vCalibration = 41.5;
const float currCalibration = 0.15;
 
// Blynk and WiFi credentials
const char auth[] = "JAxTxt8ecSeywvcTRpMm12FNS3ncpcd0";
const char ssid[] = "Redmi10";
const char pass[] = "12345678929";
 
// EnergyMonitor instance
EnergyMonitor emon;
 
// Timer for regular updates
BlynkTimer timer;
 
// Variables for energy calculation
float kWh = 0.0;
unsigned long lastMillis = millis();
 
// EEPROM addresses for each variable
const int addrVrms = 0;
const int addrIrms = 4;
const int addrPower = 8;
const int addrKWh = 12;
 
// Function prototypes
void sendEnergyDataToBlynk();
void readEnergyDataFromEEPROM();
void saveEnergyDataToEEPROM();
 
 
void setup()


{
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  pinMode(10,OUTPUT);
  pinMode(9,OUTPUT);
  // Initialize the LCD
  // lcd.init();
  // lcd.backlight();
 
  // Initialize EEPROM with the size of the data to be stored
  EEPROM.begin(32); // Allocate 32 bytes for float values (4 bytes each) and some extra space
 
  // Read the stored energy data from EEPROM
  readEnergyDataFromEEPROM();
 
  // Setup voltage and current inputs
  emon.voltage(13, vCalibration, 1.7); // Voltage: input pin, calibration, phase_shift
  emon.current(14, currCalibration);    // Current: input pin, calibration
 
  // Setup a timer for sending data every 5 seconds
  timer.setInterval(5000L, sendEnergyDataToBlynk);
 
  // A small delay for system to stabilize
  delay(500);
}
 
 
void loop()
{
  Blynk.run();
  timer.run();
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n'); // Read the input until newline
    
    // Trim the input to remove any leading/trailing whitespace
    input.trim();
    
    if (input == "0") { // Check if the input is '0'
      kWh = 0.0; // Reset kWh value
      saveEnergyDataToEEPROM(); // Save the reset value to EEPROM
      
      Serial.println("kWh value has been reset to 0.0");
      

    }
  }
}
 
 
void sendEnergyDataToBlynk()
{
  emon.calcVI(20, 2000); // Calculate all. No.of half wavelengths (crossings), time-out
 
  // Calculate energy consumed in kWh
  unsigned long currentMillis = millis();
  kWh += emon.apparentPower * (currentMillis - lastMillis) / 3600000000.0;
  lastMillis = currentMillis;
 
  // Print data to Serial for debugging
  Serial.printf("Vrms: %.2fV\tIrms: %.4fA\tPower: %.4fW\tkWh: %.7fkWh\n",
                emon.Vrms, emon.Irms, emon.apparentPower, kWh);
 if(kWh>0.0000200)
 {
  digitalWrite(10,HIGH);
  digitalWrite(9,LOW);

 }
 else
 {
  digitalWrite(10,LOW);
  digitalWrite(9,HIGH);
 }
 
  // Save the latest values to EEPROM
  saveEnergyDataToEEPROM();
 
  // Send data to Blynk
  Blynk.virtualWrite(V0, emon.Vrms);
  Blynk.virtualWrite(V1, emon.Irms);
  Blynk.virtualWrite(V2, emon.apparentPower);
  Blynk.virtualWrite(V3, kWh);
 
  // Update the LCD with the new values
  // lcd.clear();
  // lcd.setCursor(0, 0);
  // lcd.print("Vrms: ");
  // lcd.print(emon.Vrms, 2);
  // lcd.print(" V");
 
  // lcd.setCursor(0, 1);
  // lcd.print("Irms: ");
  // lcd.print(emon.Irms, 4);
  // lcd.print(" A");
 
  // lcd.setCursor(0, 2);
  // lcd.print("Power: ");
  // lcd.print(emon.apparentPower, 4);
  // lcd.print(" W");
 
  // lcd.setCursor(0, 3);
  // lcd.print("kWh: ");
  // lcd.print(kWh, 5);
  // lcd.print(" kWh");
}
 
 
void readEnergyDataFromEEPROM()
{
  // Read the stored kWh value from EEPROM
  EEPROM.get(addrKWh, kWh);
 
  // Check if the read value is a valid float. If not, initialize it to zero
  if (isnan(kWh))
  {
    kWh = 0.0;
    saveEnergyDataToEEPROM(); // Save initialized value to EEPROM
  }
}
 
 
void saveEnergyDataToEEPROM()
{
  // Write the current kWh value to EEPROM
  EEPROM.put(addrKWh, kWh);
 
  // Commit changes to EEPROM
  EEPROM.commit();
}