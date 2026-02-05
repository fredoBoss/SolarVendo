// ===== CHARGING FUNCTIONS =====

// ===== RELAY CONFIGURATION =====
const bool RELAY_ACTIVE_LOW = true;  // Change to true if relay works inverted

void relayON() {
  if (RELAY_ACTIVE_LOW) {
    digitalWrite(RELAY_PIN, LOW);
  } else {
    digitalWrite(RELAY_PIN, HIGH);
  }
  Serial.println(F(">>> RELAY: ON"));
}

void relayOFF() {
  if (RELAY_ACTIVE_LOW) {
    digitalWrite(RELAY_PIN, HIGH);
  } else {
    digitalWrite(RELAY_PIN, LOW);
  }
  Serial.println(F(">>> RELAY: OFF"));
}

// int calculateChargingTime(float weight) {
//   if (weight < 100) {
//     return 0;
//   } else if (weight >= 500) {
//     return 192;
//   } else {
//     float extraWeight = weight - 100;
//     int intervals = (int)(extraWeight / 25);
//     return intervals * 12;
//   }
// }

//adjusted charging time per 20 minutes
int calculateChargingTime(float weight) {
  if (weight < 20) {
    return 0;
  } else if (weight >= 500) {
    return 120;
  } else {
    float extraWeight = weight - 20;
    int intervals = (int)(extraWeight / 20);
    return intervals * 5;
  }
}

void startCharging() {
  currentState = CHARGING;
  isCharging = true;
  chargingStartTime = millis();
  
  relayON();
  Serial.println(F(">>> USB charging activated"));
  Serial.print(F(">>> Charging for "));
  Serial.print(chargingMinutes);
  Serial.println(F(" minutes"));
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Charging Started"));
  lcd.setCursor(0, 1);
  lcd.print(chargingMinutes);
  lcd.print(F(" minutes"));
  delay(2000);
}

void stopCharging() {
  relayOFF();
  isCharging = false;
  Serial.println(F(">>> USB charging deactivated"));
}

void handleCharging() {
  unsigned long elapsed = millis() - chargingStartTime;
  unsigned long totalDuration = (unsigned long)chargingMinutes * 60000UL;
  
  if (elapsed < totalDuration) {
    unsigned long remaining = (totalDuration - elapsed) / 1000;
    int minutes = remaining / 60;
    int seconds = remaining % 60;
    
    unsigned long elapsedSeconds = elapsed / 1000;
    int elapsedMinutes = elapsedSeconds / 60;
    int elapsedSecs = elapsedSeconds % 60;
    
    displayChargingCountdown(minutes, seconds, elapsedMinutes, elapsedSecs);
    
    static unsigned long lastSerialUpdate = 0;
    if (millis() - lastSerialUpdate > 10000) {
      Serial.print(F("Charging... Remaining: "));
      Serial.print(minutes);
      Serial.print(F(":"));
      if (seconds < 10) Serial.print(F("0"));
      Serial.print(seconds);
      Serial.print(F(" | Elapsed: "));
      Serial.print(elapsedMinutes);
      Serial.print(F(":"));
      if (elapsedSecs < 10) Serial.print(F("0"));
      Serial.print(elapsedSecs);
      Serial.println(F(" | Relay: ON"));
      lastSerialUpdate = millis();
    }
    
    relayON();  // Keep relay ON
    
    static unsigned long lastWeightCheck = 0;
    if (millis() - lastWeightCheck > 5000) {
      float weight = getWeight();
      if (weight < WEIGHT_THRESHOLD) {
        Serial.println(F("\n!!! Bottle removed!"));
        stopCharging();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("Bottle Removed!"));
        lcd.setCursor(0, 1);
        lcd.print(F("Charge Stopped"));
        delay(3000);
        resetSystem();
        return;
      }
      lastWeightCheck = millis();
    }
  } else {
    Serial.println(F("\n>>> Charging complete!"));
    stopCharging();
    currentState = COMPLETE;
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Charging Done!"));
    lcd.setCursor(0, 1);
    lcd.print(F("Remove Bottle"));
  }
}

void testRelay() {
  Serial.println(F("\n=== RELAY TEST ==="));
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Relay Test"));
  
  Serial.println(F("Turning relay ON..."));
  lcd.setCursor(0, 1);
  lcd.print(F("Relay: ON"));
  relayON();
  delay(3000);
  
  Serial.println(F("Turning relay OFF..."));
  lcd.setCursor(0, 1);
  lcd.print(F("Relay: OFF "));
  relayOFF();
  delay(2000);
  
  Serial.println(F("Test complete!"));
  Serial.println(F("==================\n"));
  
  if (currentState == IDLE) displayIdle();
}


// // ===== CHARGING FUNCTIONS =====

// int calculateChargingTime(float weight) {
//   if (weight < 100) {
//     return 0;
//   } else if (weight >= 500) {
//     return 192;  // Max: 500g = 192 minutes
//   } else {
//     float extraWeight = weight - 100;
//     int intervals = (int)(extraWeight / 25);
//     return intervals * 12;
//   }
// }

// void startCharging() {
//   currentState = CHARGING;
//   isCharging = true;
//   chargingStartTime = millis();
  
//   digitalWrite(RELAY_PIN, HIGH);
//   Serial.println(F(">>> RELAY: ON"));
//   Serial.println(F(">>> USB charging activated"));
//   Serial.print(F(">>> Charging for "));
//   Serial.print(chargingMinutes);
//   Serial.println(F(" minutes"));
  
//   lcd.clear();
//   lcd.setCursor(0, 0);
//   lcd.print(F("Charging Started"));
//   lcd.setCursor(0, 1);
//   lcd.print(chargingMinutes);
//   lcd.print(F(" minutes"));
//   delay(2000);
// }

// void stopCharging() {
//   digitalWrite(RELAY_PIN, LOW);
//   isCharging = false;
//   Serial.println(F(">>> RELAY: OFF"));
//   Serial.println(F(">>> USB charging deactivated"));
// }

// void handleCharging() {
//   unsigned long elapsed = millis() - chargingStartTime;
//   unsigned long totalDuration = (unsigned long)chargingMinutes * 60000UL;
  
//   if (elapsed < totalDuration) {
//     unsigned long remaining = (totalDuration - elapsed) / 1000;
//     int minutes = remaining / 60;
//     int seconds = remaining % 60;
    
//     unsigned long elapsedSeconds = elapsed / 1000;
//     int elapsedMinutes = elapsedSeconds / 60;
//     int elapsedSecs = elapsedSeconds % 60;
    
//     displayChargingCountdown(minutes, seconds, elapsedMinutes, elapsedSecs);
    
//     static unsigned long lastSerialUpdate = 0;
//     if (millis() - lastSerialUpdate > 10000) {
//       Serial.print(F("Charging... Remaining: "));
//       Serial.print(minutes);
//       Serial.print(F(":"));
//       if (seconds < 10) Serial.print(F("0"));
//       Serial.print(seconds);
//       Serial.print(F(" | Elapsed: "));
//       Serial.print(elapsedMinutes);
//       Serial.print(F(":"));
//       if (elapsedSecs < 10) Serial.print(F("0"));
//       Serial.print(elapsedSecs);
//       Serial.println(F(" | Relay: ON"));
//       lastSerialUpdate = millis();
//     }
    
//     digitalWrite(RELAY_PIN, HIGH);
    
//     static unsigned long lastWeightCheck = 0;
//     if (millis() - lastWeightCheck > 5000) {
//       float weight = getWeight();
//       if (weight < WEIGHT_THRESHOLD) {
//         Serial.println(F("\n!!! Bottle removed during charging!"));
//         Serial.print(F("Weight dropped to: "));
//         Serial.print(weight, 1);
//         Serial.println(F(" g"));
//         stopCharging();
//         lcd.clear();
//         lcd.setCursor(0, 0);
//         lcd.print(F("Bottle Removed!"));
//         lcd.setCursor(0, 1);
//         lcd.print(F("Charge Stopped"));
//         delay(3000);
//         resetSystem();
//         return;
//       }
//       lastWeightCheck = millis();
//     }
//   } else {
//     Serial.println(F("\n>>> Charging complete!"));
//     Serial.print(F("Total time: "));
//     Serial.print(chargingMinutes);
//     Serial.println(F(" minutes"));
    
//     stopCharging();
//     currentState = COMPLETE;
    
//     lcd.clear();
//     lcd.setCursor(0, 0);
//     lcd.print(F("Charging Done!"));
//     lcd.setCursor(0, 1);
//     lcd.print(F("Remove Bottle"));
//   }
// }