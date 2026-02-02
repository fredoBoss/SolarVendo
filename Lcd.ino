// ===== LCD DISPLAY FUNCTIONS =====

void initLCD() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(F("Solar Charging"));
  lcd.setCursor(0, 1);
  lcd.print(F("System Ready"));
  Serial.println(F("LCD initialized"));
  delay(2000);
}

void displayIdle() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Place Bottle"));
  lcd.setCursor(0, 1);
  lcd.print(F("on Scale"));
}

void displayWeightResult(float weight, int minutes) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Wt: "));
  lcd.print(weight, 1);
  lcd.print(F("g"));
  
  lcd.setCursor(0, 1);
  lcd.print(F("Time: "));
  lcd.print(minutes);
  lcd.print(F(" min"));
  delay(2000);
}

void displayChargingCountdown(int minutes, int seconds, int elapsedMin, int elapsedSec) {
  lcd.setCursor(0, 0);
  lcd.print(F("Charging: "));
  if (minutes < 10) lcd.print(F("0"));
  lcd.print(minutes);
  lcd.print(F(":"));
  if (seconds < 10) lcd.print(F("0"));
  lcd.print(seconds);
  
  lcd.setCursor(0, 1);
  lcd.print(F("Done: "));
  if (elapsedMin < 10) lcd.print(F("0"));
  lcd.print(elapsedMin);
  lcd.print(F(":"));
  if (elapsedSec < 10) lcd.print(F("0"));
  lcd.print(elapsedSec);
  lcd.print(F("   "));
}