#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <EEPROM.h>

// ===== PIN DEFINITIONS =====
#define HX711_DOUT 3
#define HX711_SCK 2
#define RELAY_PIN 7
#define BUTTON_PIN 4
#define LDR_PIN A0
#define LASER_PIN 8

// ===== COMPONENT SETUP =====
LiquidCrystal_I2C lcd(0x27, 16, 2);
HX711 scale;

// ===== CALIBRATION VARIABLES =====
float calibration_factor = 0;
const int EEPROM_ADDR = 0;
const float KNOWN_WEIGHT = 978.0;

// ===== WEIGHT DETECTION THRESHOLD =====
const float WEIGHT_THRESHOLD = 10.0;

// ===== WEIGHT & CHARGING VARIABLES =====
float currentWeight = 0;
int chargingMinutes = 0;
unsigned long chargingStartTime = 0;
bool isCharging = false;

// ===== BUTTON DEBOUNCE =====
bool buttonState = HIGH;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// ===== SYSTEM STATES =====
enum State {
  IDLE,
  BOTTLE_DETECTED,
  WEIGHING,
  READY_TO_CHARGE,
  CHARGING,
  COMPLETE
};
State currentState = IDLE;

// ===== DIAGNOSTIC VARIABLES =====
struct DiagnosticResults {
  bool hx711Communication;
  bool wireIntegrity;
  bool sensorResponsive;
  bool noiseLevel;
  String errorMessage;
};
DiagnosticResults lastDiagnostic;

void setup() {
  Serial.begin(9600);
  Serial.println(F("\n========================================"));
  Serial.println(F("SOLAR VENDING CHARGING MACHINE"));
  Serial.println(F("========================================"));
  
  // Initialize LCD
  initLCD();
  
  // Initialize Load Cell
  initWeight();
  
  // Initialize LDR & Laser
  initLDR();
  
  // Initialize Pins
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  delay(100); 
  Serial.println(F("Relay: OFF"));

  pinMode(BUTTON_PIN, INPUT_PULLUP);
   Serial.println(F("Button initialized"));
  Serial.println(F("All pins initialized"));
  
  // Initial state
  currentState = IDLE;
  displayIdle();
  
  Serial.println(F("\n=== COMMANDS ==="));
  Serial.println(F("CAL - Start calibration"));
  Serial.println(F("printCAL - View calibration"));
  Serial.println(F("readWt - Read current weight"));
  Serial.println(F("clearEEPROM - Clear calibration"));
  Serial.println(F("DIAG - Full diagnostic test"));
  Serial.println(F("QUICKDIAG - Quick hardware check"));
  Serial.println(F("testLDR - check LDR"));
  Serial.println(F("================\n"));
  Serial.println(F("System ready. Place bottle on scale...\n"));
}

void loop() {
  // Check for serial commands
  checkSerialCommands();
  
  // Read button with debouncing
  readButton();
  
  // Check LDR for laser detection
  checkLDRStatus();

   // ===== DEBUG: Print relay state every 5 seconds =====
  // static unsigned long lastDebugPrint = 0;
  // if (millis() - lastDebugPrint > 5000) {
  //   Serial.print(F("DEBUG - Relay Pin "));
  //   Serial.print(RELAY_PIN);
  //   Serial.print(F(": "));
  //   Serial.println(digitalRead(RELAY_PIN) == HIGH ? "HIGH" : "LOW");
  //   lastDebugPrint = millis();
  // }
  // ====================================================
  
  // State machine
  switch (currentState) {
    case IDLE:
      handleIdle();
      break;
      
    case BOTTLE_DETECTED:
      handleBottleDetected();
      break;
      
    case WEIGHING:
      handleWeighing();
      break;
      
    case READY_TO_CHARGE:
      // Waiting for button press
      break;
      
    case CHARGING:
      handleCharging();
      break;
      
    case COMPLETE:
      handleComplete();
      break;
  }
  
  delay(100);
}

// ===== STATE HANDLERS =====
void handleIdle() {
  float weight = getWeight();
  
  if (weight > WEIGHT_THRESHOLD) {
    Serial.print(F("\n>>> Bottle detected! Weight: "));
    Serial.print(weight, 1);
    Serial.println(F(" g"));
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Bottle Detected!"));
    lcd.setCursor(0, 1);
    lcd.print(F("Weight: "));
    lcd.print(weight, 0);
    lcd.print(F("g"));
    delay(1000);
    
    currentState = BOTTLE_DETECTED;
  } else {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 3000) {
      displayIdle();
      lastUpdate = millis();
    }
  }
}

void handleBottleDetected() {
  Serial.println(F("Stabilizing weight..."));
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Stabilizing..."));
  delay(1500);
  currentState = WEIGHING;
}

// void handleWeighing() {
//   Serial.println(F(">>> Weighing..."));
//   lcd.clear();
//   lcd.setCursor(0, 0);
//   lcd.print(F("Weighing..."));
//   delay(1000);
  
//   currentWeight = scale.get_units(10);
  
//   Serial.print(F("Weight measured: "));
//   Serial.print(currentWeight, 2);
//   Serial.println(F(" g"));
  
//   if (currentWeight < WEIGHT_THRESHOLD) {
//     Serial.println(F("Bottle removed during weighing!"));
//     lcd.clear();
//     lcd.setCursor(0, 0);
//     lcd.print(F("Bottle Removed!"));
//     lcd.setCursor(0, 1);
//     lcd.print(F("Try again"));
//     delay(2000);
//     currentState = IDLE;
//     scale.tare();
//     displayIdle();
//     return;
//   }
  
//   chargingMinutes = calculateChargingTime(currentWeight);
  
//   Serial.print(F(">>> Weight Classification:"));
//   Serial.print(F("\n    Weight: "));
//   Serial.print(currentWeight, 1);
//   Serial.println(F(" g"));
//   Serial.print(F("    Charging Time: "));
//   Serial.print(chargingMinutes);
//   Serial.println(F(" minutes"));
  
//   displayWeightResult(currentWeight, chargingMinutes);
  
//   if (chargingMinutes > 0) {
//     lcd.clear();
//     lcd.setCursor(0, 0);
//     lcd.print(F("Press Button"));
//     lcd.setCursor(0, 1);
//     lcd.print(F("to Start Charge"));
    
//     Serial.println(F(">>> Ready to charge. Press button to start."));
//     currentState = READY_TO_CHARGE;
//   } else {
//     lcd.setCursor(0, 1);
//     lcd.print(F("Too Light!"));
//     Serial.println(F(">>> Bottle too light (< 100g). No charging."));
//     delay(3000);
//     currentState = IDLE;
//     scale.tare();
//     displayIdle();
//   }
// }

// handle weight with LDR
void handleWeighing() {
  Serial.println(F(">>> Weighing..."));
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Weighing..."));
  delay(1000);
  
  // Get accurate weight reading (average of 10 readings)
  currentWeight = scale.get_units(10);
  
  Serial.print(F("Weight measured: "));
  Serial.print(currentWeight, 2);
  Serial.println(F(" g"));
  
  // Check if bottle is still on scale
  if (currentWeight < WEIGHT_THRESHOLD) {
    Serial.println(F("Bottle removed during weighing!"));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Bottle Removed!"));
    lcd.setCursor(0, 1);
    lcd.print(F("Try again"));
    delay(2000);
    currentState = IDLE;
    scale.tare();
    displayIdle();
    return;
  }
  
  // ===== NEW: WATER DETECTION CHECK =====
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Checking Water"));
  lcd.setCursor(0, 1);
  lcd.print(F("Please wait..."));
  
  delay(500);  // Brief pause before laser check
  
  if (bottleHasWater()) {
    // Water detected - REJECT the bottle
    displayWaterDetected();
    
    // Reset system
    currentState = IDLE;
    scale.tare();
    displayIdle();
    return;  // EXIT - don't proceed to charging
  }
  
  // Water check passed - show confirmation
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Bottle Empty"));
  lcd.setCursor(0, 1);
  lcd.print(F("Accepted!"));
  delay(1500);
  // ===== END WATER DETECTION =====
  
  // Calculate charging time based on weight
  chargingMinutes = calculateChargingTime(currentWeight);
  
  Serial.print(F(">>> Weight Classification:"));
  Serial.print(F("\n    Weight: "));
  Serial.print(currentWeight, 1);
  Serial.println(F(" g"));
  Serial.print(F("    Charging Time: "));
  Serial.print(chargingMinutes);
  Serial.println(F(" minutes"));
  
  // Display weight and charging time
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Wt: "));
  lcd.print(currentWeight, 1);
  lcd.print(F("g"));
  
  if (chargingMinutes > 0) {
    lcd.setCursor(0, 1);
    lcd.print(F("Time: "));
    lcd.print(chargingMinutes);
    lcd.print(F(" min"));
    delay(2000);
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Press Button"));
    lcd.setCursor(0, 1);
    lcd.print(F("to Start Charge"));
    
    Serial.println(F(">>> Ready to charge. Press button to start."));
    currentState = READY_TO_CHARGE;
  } else {
    lcd.setCursor(0, 1);
    lcd.print(F("Too Light!"));
    Serial.println(F(">>> Bottle too light (< 100g). No charging."));
    delay(3000);
    currentState = IDLE;
    scale.tare();
    displayIdle();
  }
}

// Add this TEMPORARILY for testing
void testLDR() {
  Serial.println(F("\n=== LDR CALIBRATION TEST ==="));
  
  // Test with EMPTY bottle
  Serial.println(F("\nPlace EMPTY bottle, press 'e'"));
  while (Serial.read() != 'e') { delay(10); }
  
  int emptyReadings[10];
  for (int i = 0; i < 10; i++) {
    emptyReadings[i] = analogRead(LDR_PIN);
    Serial.print(F("Empty reading "));
    Serial.print(i+1);
    Serial.print(F(": "));
    Serial.println(emptyReadings[i]);
    delay(200);
  }
  
  // Calculate average for empty
  int emptySum = 0;
  for (int i = 0; i < 10; i++) {
    emptySum += emptyReadings[i];
  }
  int emptyAvg = emptySum / 10;
  
  Serial.print(F("\n>>> Empty Bottle Average: "));
  Serial.println(emptyAvg);
  
  // Test with WATER-FILLED bottle
  Serial.println(F("\nFill bottle with water, press 'w'"));
  while (Serial.read() != 'w') { delay(10); }
  
  int waterReadings[10];
  for (int i = 0; i < 10; i++) {
    waterReadings[i] = analogRead(LDR_PIN);
    Serial.print(F("Water reading "));
    Serial.print(i+1);
    Serial.print(F(": "));
    Serial.println(waterReadings[i]);
    delay(200);
  }
  
  // Calculate average for water
  int waterSum = 0;
  for (int i = 0; i < 10; i++) {
    waterSum += waterReadings[i];
  }
  int waterAvg = waterSum / 10;
  
  Serial.print(F("\n>>> Water Bottle Average: "));
  Serial.println(waterAvg);
  
  // Calculate recommended threshold
  int recommendedThreshold = (emptyAvg + waterAvg) / 2;
  
  Serial.println(F("\n╔════════════════════════════════╗"));
  Serial.print(F("║ Empty Avg:  "));
  Serial.print(emptyAvg);
  Serial.println(F("               ║"));
  Serial.print(F("║ Water Avg:  "));
  Serial.print(waterAvg);

  Serial.println(F("               ║"));
  Serial.print(F("║ RECOMMENDED THRESHOLD: "));
  Serial.print(recommendedThreshold);
  Serial.println(F("   ║"));
  Serial.println(F("╚════════════════════════════════╝"));
  Serial.println(F("\nUpdate LDR_THRESHOLD in Ldr.ino!"));
}

void handleComplete() {
  float weight = getWeight();
  
  if (weight < WEIGHT_THRESHOLD) {
    Serial.println(F("Bottle removed. System resetting..."));
    delay(1000);
    resetSystem();
  }
}

// ===== BUTTON HANDLING =====
void readButton() {
  int reading = digitalRead(BUTTON_PIN);
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      
      if (buttonState == LOW) {
        Serial.println(F("\n>>> Button pressed!"));
        handleButtonPress();
      }
    }
  }
  
  lastButtonState = reading;
}

void handleButtonPress() {
  if (currentState == READY_TO_CHARGE) {
    startCharging();
  }
}

void resetSystem() {
  stopCharging();
  scale.tare();
  currentState = IDLE;
  displayIdle();
  Serial.println(F(">>> System reset - Ready for next bottle\n"));
}

// ===== SERIAL COMMANDS =====
void checkSerialCommands() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toUpperCase();
    
    Serial.print(F("\n>>> Command: "));
    Serial.println(command);
    
    if (command == F("CAL")) {
      calibrateLoadCell();
    } else if (command == F("PRINTCAL")) {
      printCalibration();
    } else if (command == F("READWT")) {
      readWeight();
    } else if (command == F("CLEAREEPROM")) {
      clearEEPROM();
    } else if (command == F("DIAG")) {
      comprehensiveLoadCellCheck();
    } else if (command == F("QUICKDIAG")) {
      quickDiagnostic();
    } else if (command == F("TESTLDR")) {
      testLDR(); 
    } else if (command == F("TESTRELAY")) {  // NEW!
      testRelay(); // Add this line
    } else {
      Serial.print(F("Unknown command: "));
      Serial.println(command);
    }
  }
}

void waitForUserConfirmation() {
  Serial.println(F("Waiting for 'y'..."));
  while (true) {
    if (Serial.available() > 0) {
      char input = Serial.read();
      if (input == 'y' || input == 'Y') {
        Serial.println(F("Confirmed!"));
        while (Serial.available() > 0) {
          Serial.read();
        }
        delay(100);
        break;
      }
    }
    delay(50);
  }
}