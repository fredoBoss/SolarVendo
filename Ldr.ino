// // ===== LDR (LASER DETECTION) FUNCTIONS =====

// const int LDR_THRESHOLD = 800;  // Adjust based on your LDR (0-1023)
// bool laserDetected = false;
// bool lastLaserState = false;

// void initLDR() {
//   pinMode(LDR_PIN, INPUT);
//   pinMode(LASER_PIN, OUTPUT);
  
//   // Turn on laser module
//   digitalWrite(LASER_PIN, HIGH);
  
//   Serial.println(F("LDR initialized"));
//   Serial.println(F("Laser module: ON"));
//   Serial.print(F("LDR Threshold: "));
//   Serial.println(LDR_THRESHOLD);
// }

// int readLDR() {
//   return analogRead(LDR_PIN);
// }

// void checkLDRStatus() {
//   int ldrValue = readLDR();
  
//   // When laser hits LDR, value is HIGH (bright)
//   // When laser is blocked, value is LOW (dark)
//   if (ldrValue > LDR_THRESHOLD) {
//     laserDetected = true;
//   } else {
//     laserDetected = false;
//   }
  
//   // Trigger event on state change
//   if (laserDetected != lastLaserState) {
//     if (laserDetected) {
//       Serial.println(F("\n>>> Laser DETECTED (LDR contacted)"));
//       Serial.print(F("LDR Value: "));
//       Serial.println(ldrValue);
//       onLaserDetected();
//     } else {
//       Serial.println(F("\n>>> Laser BLOCKED (LDR interrupted)"));
//       Serial.print(F("LDR Value: "));
//       Serial.println(ldrValue);
//       onLaserBlocked();
//     }
//     lastLaserState = laserDetected;
//   }
// }

// // Event handlers - customize these based on your needs
// void onLaserDetected() {
//   // Example: Show notification on LCD
//   lcd.clear();
//   lcd.setCursor(0, 0);
//   lcd.print(F("Laser Detected!"));
//   lcd.setCursor(0, 1);
//   lcd.print(F("Path Clear"));
//   delay(1000);
  
//   if (currentState == IDLE) {
//     displayIdle();
//   }
// }

// void onLaserBlocked() {
//   // Example: Alert when laser is blocked
//   lcd.clear();
//   lcd.setCursor(0, 0);
//   lcd.print(F("Laser Blocked!"));
//   lcd.setCursor(0, 1);
//   lcd.print(F("Path Obstructed"));
//   delay(1000);
  
//   if (currentState == IDLE) {
//     displayIdle();
//   }
// }

// // Optional: Get laser detection status
// bool isLaserClear() {
//   return laserDetected;
// }


// ===== LDR (WATER DETECTION) FUNCTIONS =====
// const int LDR_THRESHOLD = 500;  // Adjust based on testing (0-1023)
   const int LDR_THRESHOLD = 747;                               // If LDR > threshold = NO WATER (clear)
                                 // If LDR < threshold = WATER DETECTED

bool hasWater = false;
unsigned long lastLDRCheck = 0;
const unsigned long LDR_CHECK_INTERVAL = 500;  // Check every 500ms

void initLDR() {
  pinMode(LDR_PIN, INPUT);
  pinMode(LASER_PIN, OUTPUT);
  
  // Turn on laser module
  digitalWrite(LASER_PIN, HIGH);
  
  Serial.println(F("LDR initialized"));
  Serial.println(F("Laser module: ON"));
  Serial.print(F("LDR Threshold: "));
  Serial.println(LDR_THRESHOLD);
  
  delay(500);
  
  // Initial reading
  int initialReading = analogRead(LDR_PIN);
  Serial.print(F("Initial LDR reading: "));
  Serial.println(initialReading);
}

int readLDR() {
  return analogRead(LDR_PIN);
}

void checkLDRStatus() {
  // Only check during specific states to save processing
  if (currentState != BOTTLE_DETECTED && currentState != WEIGHING) {
    return;
  }
  
  // Throttle checks to every 500ms
  if (millis() - lastLDRCheck < LDR_CHECK_INTERVAL) {
    return;
  }
  lastLDRCheck = millis();
  
  int ldrValue = readLDR();
  
  // Higher value = laser getting through (no water/empty)
  // Lower value = laser blocked/scattered (water present)
  if (ldrValue < LDR_THRESHOLD) {
    hasWater = true;
  } else {
    hasWater = false;
  }
  
  // Debug output
  static int lastPrintedValue = -1;
  if (abs(ldrValue - lastPrintedValue) > 50) {  // Only print on significant change
    Serial.print(F("LDR: "));
    Serial.print(ldrValue);
    Serial.print(F(" | Water: "));
    Serial.println(hasWater ? F("YES") : F("NO"));
    lastPrintedValue = ldrValue;
  }
}

bool bottleHasWater() {
  // Take multiple readings for accuracy
  int readings[5];
  int sum = 0;
  
  for (int i = 0; i < 5; i++) {
    readings[i] = analogRead(LDR_PIN);
    sum += readings[i];
    delay(50);
  }
  
  int average = sum / 5;
  
  Serial.print(F("\n>>> Water Detection Check"));
  Serial.print(F("\n    LDR Average: "));
  Serial.print(average);
  Serial.print(F("\n    Threshold: "));
  Serial.println(LDR_THRESHOLD);
  
  if (average < LDR_THRESHOLD) {
    Serial.println(F("    Result: WATER DETECTED ❌"));
    return true;  // Has water
  } else {
    Serial.println(F("    Result: NO WATER (Empty) ✓"));
    return false;  // No water (empty)
  }
}

// Helper function to show water detection result on LCD
void displayWaterDetected() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Bottle Has"));
  lcd.setCursor(0, 1);
  lcd.print(F("WATER! Remove"));
  
  Serial.println(F("\n!!! BOTTLE HAS WATER - REJECTED !!!"));
  
  delay(3000);
}