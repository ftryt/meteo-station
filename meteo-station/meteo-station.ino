#include <LiquidCrystal_I2C.h>

#define AnalogButtonPin 32

// Global
LiquidCrystal_I2C lcd(0x27, 20, 4);
int page = 0;
int lastPage = -1;
unsigned long lastInputTime = 0;

void setup() {
  Serial.begin(115200);

  // LCD initialization
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0,0);
}

void loop() {
  handleInput();
  drawMenu();
}

void handleInput() {
  if (millis() - lastInputTime < 350) return; 

  int val = analogRead(AnalogButtonPin);
  bool changed = false;

  if (val > 3000) {
    page--;
    changed = true;
  } else if (val < 1500) {
    page++;
    changed = true;
  }

  if (!changed) return;

  lastInputTime = millis();

  if (page > 2) page = 0;
  if (page < 0) page = 2;
}

void drawMenu() {
  if (page == lastPage) return;

  lcd.clear();

  switch (page) {
    case 0:
      lcd.setCursor(0, 0);
      lcd.print("Page 1 (Home)");
      break;
    case 1:
      lcd.setCursor(0, 0);
      lcd.print("Page 2 (Settings)");
      break;
    case 2:
      lcd.setCursor(0, 0);
      lcd.print("Page 3 (Info)");
      break;
  }

  lastPage = page;
}