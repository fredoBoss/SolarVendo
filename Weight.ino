// ===== WEIGHT SENSOR FUNCTIONS =====

void initWeight() {
  Serial.println(F("Initializing HX711..."));
  scale.begin(HX711_DOUT, HX711_SCK);
  
  int retries = 5;
  while (!scale.is_ready() && retries > 0) {
    Serial.println(F("HX711 not found, retrying..."));
    delay(1000);
    retries--;
  }

  if (!scale.is_ready()) {
    Serial.println(F("ERROR: HX711 failed!"));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("HX711 ERROR!"));
    while (true);
  }
  Serial.println(F("HX711 OK"));

  // Run quick diagnostic
  if (!quickDiagnostic()) {
    Serial.println(F("\n*** STARTUP DIAGNOSTIC FAILED! ***"));
    Serial.println(F("Hardware issue detected"));
    Serial.println(F("Send 'DIAG' for full diagnostic report"));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Hardware Error!"));
    lcd.setCursor(0, 1);
    lcd.print(F("Check wiring"));
    delay(5000);
  }
  
  // Load calibration
  loadCalibration();
  
  if (calibration_factor == 0 || isnan(calibration_factor)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("No Calibration!"));
    lcd.setCursor(0, 1);
    lcd.print(F("Send CAL cmd"));
    Serial.println(F("\n*** NO CALIBRATION ***"));
    Serial.println(F("Send 'CAL' to calibrate"));
  } else {
    scale.set_scale(calibration_factor);
    scale.tare();
    Serial.println(F("Calibration loaded"));
    Serial.print(F("Calibration Factor: "));
    Serial.println(calibration_factor);
  }
}

float getWeight() {
  if (calibration_factor == 0 || isnan(calibration_factor)) {
    return 0;
  }
  return scale.get_units(5);
}

void readWeight() {
  if (calibration_factor == 0 || isnan(calibration_factor)) {
    Serial.println(F("\n*** NOT CALIBRATED ***"));
    Serial.println(F("Send 'CAL' to calibrate first."));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Not Calibrated!"));
    lcd.setCursor(0, 1);
    lcd.print(F("Send CAL cmd"));
    delay(2000);
    if (currentState == IDLE) displayIdle();
    return;
  }
  
  Serial.println(F("\n=== READING WEIGHT ==="));
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Reading Weight"));
  
  delay(500);
  float weight = scale.get_units(10);
  
  Serial.print(F("Current Weight: "));
  Serial.print(weight, 2);
  Serial.println(F(" g"));
  Serial.println(F("======================\n"));
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Weight: "));
  lcd.print(weight, 1);
  lcd.print(F("g"));
  delay(3000);
  
  if (currentState == IDLE) {
    displayIdle();
  }
}

// ===== CALIBRATION =====
void calibrateLoadCell() {
  Serial.println(F("\n========================================"));
  Serial.println(F("LOAD CELL CALIBRATION"));
  Serial.println(F("========================================"));
  
  if (!scale.is_ready()) {
    Serial.println(F("ERROR: HX711 not ready!"));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("HX711 Error!"));
    lcd.setCursor(0, 1);
    lcd.print(F("Check wiring"));
    delay(3000);
    if (currentState == IDLE) displayIdle();
    return;
  }
  
  Serial.println(F("\nSTEP 1: Taring (Zero Point)"));
  Serial.println(F("Remove all weight from scale..."));
  Serial.println(F("Press 'y' when ready..."));
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("STEP 1: Tare"));
  lcd.setCursor(0, 1);
  lcd.print(F("Remove weight"));
  
  waitForUserConfirmation();
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Taring..."));
  lcd.setCursor(0, 1);
  lcd.print(F("Please wait"));
  
  scale.set_scale();
  scale.tare(20);
  
  Serial.println(F("Tare completed."));
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Tare Complete!"));
  delay(1500);
  
  Serial.println(F("\nSTEP 2: Measuring with known weight"));
  Serial.print(F("Place "));
  Serial.print(KNOWN_WEIGHT);
  Serial.println(F("g on scale..."));
  Serial.println(F("Press 'y' when ready..."));
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("STEP 2: Weight"));
  lcd.setCursor(0, 1);
  lcd.print(F("Place 978g"));
  
  waitForUserConfirmation();
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Measuring..."));
  lcd.setCursor(0, 1);
  lcd.print(F("Please wait"));
  
  float reading = scale.get_value(20);
  calibration_factor = reading / KNOWN_WEIGHT;
  
  scale.set_scale(calibration_factor);
  
  Serial.print(F("Tared reading: "));
  Serial.println(reading, 2);
  Serial.print(F("Calibration Factor: "));
  Serial.println(calibration_factor, 6);
  
  if (isnan(calibration_factor) || calibration_factor == 0 || 
      abs(calibration_factor) > 10000 || abs(calibration_factor) < 1) {
    Serial.println(F("\nERROR: Invalid calibration factor!"));
    Serial.print(F("Value: "));
    Serial.println(calibration_factor);
    Serial.println(F("Calibration failed. Please retry."));
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Cal Failed!"));
    lcd.setCursor(0, 1);
    lcd.print(F("Try again"));
    delay(3000);
    
    if (currentState == IDLE) displayIdle();
    return;
  }
  
  saveCalibration();
  
  Serial.println(F("\n*** Calibration saved to EEPROM! ***"));
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Calibration"));
  lcd.setCursor(0, 1);
  lcd.print(F("Saved!"));
  delay(2000);
  
  Serial.println(F("\nSTEP 3: Testing calibration..."));
  delay(2000);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Testing..."));
  
  float testWeight = scale.get_units(10);
  
  Serial.print(F("Test reading: "));
  Serial.print(testWeight, 2);
  Serial.println(F(" g"));
  
  float error = abs(testWeight - KNOWN_WEIGHT);
  float errorPercent = (error / KNOWN_WEIGHT) * 100;
  
  Serial.print(F("Error: "));
  Serial.print(error, 2);
  Serial.print(F(" g ("));
  Serial.print(errorPercent, 1);
  Serial.println(F("%)"));
  
  if (error < 15) {
    Serial.println(F("Test PASSED!"));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Test: "));
    lcd.print(testWeight, 1);
    lcd.print(F("g"));
    lcd.setCursor(0, 1);
    lcd.print(F("Cal SUCCESS!"));
  } else {
    Serial.println(F("Warning: Large error!"));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Test: "));
    lcd.print(testWeight, 1);
    lcd.print(F("g"));
    lcd.setCursor(0, 1);
    lcd.print(F("Error: "));
    lcd.print(errorPercent, 0);
    lcd.print(F("%"));
  }
  
  delay(3000);
  
  Serial.println(F("\nRemove weight..."));
  Serial.println(F("Press 'y' to continue..."));
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Remove weight"));
  lcd.setCursor(0, 1);
  lcd.print(F("Press 'y'"));

  waitForUserConfirmation();
  
  Serial.println(F("========================================\n"));
  
  resetSystem();
}

void printCalibration() {
  Serial.println(F("\n=== CALIBRATION INFO ==="));
  Serial.print(F("Calibration Factor: "));
  Serial.println(calibration_factor, 6);
  Serial.print(F("Known Weight: "));
  Serial.print(KNOWN_WEIGHT);
  Serial.println(F(" g"));
  Serial.print(F("Weight Threshold: "));
  Serial.print(WEIGHT_THRESHOLD);
  Serial.println(F(" g"));
  Serial.println(F("========================\n"));
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Cal: "));
  lcd.print(calibration_factor, 4);
  lcd.setCursor(0, 1);
  lcd.print(F("Threshold: "));
  lcd.print(WEIGHT_THRESHOLD, 0);
  lcd.print(F("g"));
  delay(3000);
  
  if (currentState == IDLE) displayIdle();
}

void saveCalibration() {
  EEPROM.put(EEPROM_ADDR, calibration_factor);
  Serial.println(F("Calibration saved to EEPROM"));
}

void loadCalibration() {
  EEPROM.get(EEPROM_ADDR, calibration_factor);
  
  if (isnan(calibration_factor) || calibration_factor == 0) {
    Serial.println(F("No valid calibration in EEPROM"));
    calibration_factor = 0;
  } else {
    Serial.println(F("Calibration loaded from EEPROM"));
  }
}

void clearEEPROM() {
  Serial.println(F("\n>>> Clearing EEPROM..."));
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Clearing EEPROM"));
  
  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, 0);
  }
  
  calibration_factor = 0;
  Serial.println(F("EEPROM cleared"));
  Serial.println(F("Send 'CAL' to recalibrate\n"));
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("EEPROM Cleared"));
  lcd.setCursor(0, 1);
  lcd.print(F("Recalibrate!"));
  delay(3000);
  displayIdle();
}

// ===== DIAGNOSTICS =====
bool quickDiagnostic() {
  Serial.println(F("\n=== QUICK STARTUP DIAGNOSTIC ==="));
  lcd.clear();
  lcd.print(F("Quick Check..."));
  
  if (!scale.is_ready()) {
    Serial.println(F("❌ HX711 communication failed"));
    lcd.setCursor(0, 1);
    lcd.print(F("HX711 Error!"));
    return false;
  }
  Serial.println(F("✓ HX711 OK"));
  
  long r1 = scale.read();
  delay(100);
  long r2 = scale.read();
  delay(100);
  long r3 = scale.read();
  
  if (r1 == 0 && r2 == 0 && r3 == 0) {
    Serial.println(F("❌ No signal - E+ or E- wire disconnected"));
    lcd.setCursor(0, 1);
    lcd.print(F("Wire Error!"));
    return false;
  }
  
  if (r1 == r2 && r2 == r3) {
    Serial.println(F("❌ Stuck reading - A+ or A- wire issue"));
    lcd.setCursor(0, 1);
    lcd.print(F("Sensor Error!"));
    return false;
  }
  
  Serial.println(F("✓ Basic function OK"));
  lcd.setCursor(0, 1);
  lcd.print(F("All OK!"));
  delay(1500);
  
  Serial.println(F("=== QUICK CHECK PASSED ===\n"));
  return true;
}

bool test_WireIntegrity() {
  Serial.println(F("  Testing 4-wire connection..."));
  
  const int numTests = 10;
  long rawValues[numTests];
  bool hasValidReading = false;
  
  for (int i = 0; i < numTests; i++) {
    if (!scale.is_ready()) {
      Serial.println(F("  ❌ HX711 lost during test"));
      return false;
    }
    rawValues[i] = scale.read();
    delay(100);
  }
  
  for (int i = 0; i < numTests; i++) {
    if (rawValues[i] != 0) {
      hasValidReading = true;
      break;
    }
  }
  
  if (!hasValidReading) {
    Serial.println(F("  ❌ FAILED: All readings ZERO"));
    Serial.println(F("  → E+ or E- wire disconnected"));
    return false;
  }
  
  long minVal = rawValues[0];
  long maxVal = rawValues[0];
  
  for (int i = 1; i < numTests; i++) {
    if (rawValues[i] < minVal) minVal = rawValues[i];
    if (rawValues[i] > maxVal) maxVal = rawValues[i];
  }
  
  long variance = maxVal - minVal;
  
  if (variance == 0) {
    Serial.println(F("  ❌ FAILED: Zero variance"));
    Serial.println(F("  → A+ or A- signal wires disconnected"));
    return false;
  }
  
  if (variance > 50000) {
    Serial.println(F("  ⚠️ WARNING: Very high variance"));
    Serial.print(F("  Variance: "));
    Serial.println(variance);
  }
  
  Serial.println(F("  ✓ PASSED: Wire connections intact"));
  Serial.print(F("  Variance: "));
  Serial.println(variance);
  return true;
}

bool test_SensorDamage() {
  Serial.println(F("  Checking sensor responsiveness..."));
  Serial.println(F("  → Remove all weight"));
  delay(2000);
  
  long baseline[5];
  for (int i = 0; i < 5; i++) {
    baseline[i] = scale.read();
    delay(100);
  }
  
  long baselineAvg = 0;
  for (int i = 0; i < 5; i++) {
    baselineAvg += baseline[i];
  }
  baselineAvg /= 5;
  
  Serial.println(F("  → Press on scale (~100g)"));
  Serial.println(F("  → Checking in 5 seconds..."));
  delay(5000);
  
  long loaded[5];
  for (int i = 0; i < 5; i++) {
    loaded[i] = scale.read();
    delay(100);
  }
  
  long loadedAvg = 0;
  for (int i = 0; i < 5; i++) {
    loadedAvg += loaded[i];
  }
  loadedAvg /= 5;
  
  long difference = abs(loadedAvg - baselineAvg);
  
  Serial.print(F("  Baseline: "));
  Serial.println(baselineAvg);
  Serial.print(F("  Loaded: "));
  Serial.println(loadedAvg);
  Serial.print(F("  Difference: "));
  Serial.println(difference);
  
  if (difference < 100) {
    Serial.println(F("  ❌ FAILED: No response"));
    Serial.println(F("  → Strain gauge damaged"));
    return false;
  }
  
  Serial.println(F("  ✓ PASSED: Sensor responding"));
  return true;
}

bool test_NoiseLevel() {
  Serial.println(F("  Measuring noise..."));
  Serial.println(F("  → Keep scale still"));
  delay(2000);
  
  const int samples = 20;
  long readings[samples];
  
  for (int i = 0; i < samples; i++) {
    readings[i] = scale.read();
    delay(50);
  }
  
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += readings[i];
  }
  long mean = sum / samples;
  
  long varianceSum = 0;
  for (int i = 0; i < samples; i++) {
    long diff = readings[i] - mean;
    varianceSum += (diff * diff);
  }
  float stdDev = sqrt(varianceSum / samples);
  
  Serial.print(F("  Mean: "));
  Serial.println(mean);
  Serial.print(F("  Std Dev: "));
  Serial.println(stdDev, 2);
  
  if (stdDev > 200) {
    Serial.println(F("  ❌ FAILED: Excessive noise"));
    Serial.println(F("  → Check for interference"));
    return false;
  } else if (stdDev > 100) {
    Serial.println(F("  ⚠️ WARNING: Moderate noise"));
  }
  
  Serial.println(F("  ✓ PASSED: Noise acceptable"));
  return true;
}

bool comprehensiveLoadCellCheck() {
  Serial.println(F("\n╔════════════════════════════════════════╗"));
  Serial.println(F("║  LOAD CELL COMPREHENSIVE DIAGNOSTIC    ║"));
  Serial.println(F("╚════════════════════════════════════════╝"));
  
  lcd.clear();
  lcd.print(F("Full Diagnostic"));
  
  lastDiagnostic = {false, false, false, false, ""};
  bool allPassed = true;
  
  Serial.println(F("\n[1/4] HX711 Communication Test..."));
  lcd.setCursor(0, 1);
  lcd.print(F("Test 1/4..."));
  if (!scale.is_ready()) {
    Serial.println(F("  ❌ FAILED"));
    lastDiagnostic.errorMessage = "HX711 Comm Fail";
    allPassed = false;
  } else {
    Serial.println(F("  ✓ PASSED"));
    lastDiagnostic.hx711Communication = true;
  }
  delay(500);
  
  Serial.println(F("\n[2/4] Wire Integrity Test..."));
  lcd.setCursor(0, 1);
  lcd.print(F("Test 2/4..."));
  if (!test_WireIntegrity()) {
    lastDiagnostic.errorMessage = "Wire Damage";
    allPassed = false;
  } else {
    lastDiagnostic.wireIntegrity = true;
  }
  delay(500);
  
  Serial.println(F("\n[3/4] Sensor Damage Test..."));
  lcd.setCursor(0, 1);
  lcd.print(F("Test 3/4..."));
  if (!test_SensorDamage()) {
    lastDiagnostic.errorMessage = "Sensor Damaged";
    allPassed = false;
  } else {
    lastDiagnostic.sensorResponsive = true;
}
delay(500);
Serial.println(F("\n[4/4] Noise Level Test..."));
lcd.setCursor(0, 1);
lcd.print(F("Test 4/4..."));
if (!test_NoiseLevel()) {
lastDiagnostic.errorMessage = "High Noise";
allPassed = false;
} else {
lastDiagnostic.noiseLevel = true;
}
Serial.println(F("\n╔════════════════════════════════════════╗"));
if (allPassed) {
Serial.println(F("║  ✓✓✓ ALL TESTS PASSED ✓✓✓             ║"));
lcd.clear();
lcd.print(F("Diagnostic OK!"));
lcd.setCursor(0, 1);
lcd.print(F("All tests pass"));
} else {
Serial.println(F("║  ⚠️ DIAGNOSTIC FAILED ⚠️                ║"));
Serial.print(F("║  Issue: "));
Serial.println(lastDiagnostic.errorMessage);
lcd.clear();
lcd.print(F("Diag Failed!"));
lcd.setCursor(0, 1);
lcd.print(lastDiagnostic.errorMessage);
}
Serial.println(F("╚════════════════════════════════════════╝\n"));
delay(3000);
return allPassed;
}
