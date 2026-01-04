#include <LiquidCrystal_I2C.h>
#include <Adafruit_BME280.h>

#define PageSwapPin 32
#define SubPageSwapPin 35

#define HomePage 0
#define SettingPage 1

// Global
LiquidCrystal_I2C lcd(0x27, 20, 4);
int page = 0;
int subPage = 0;
int lastPage = -1;
unsigned long lastInputTime = 0;
Adafruit_BME280 bme;

// Custom symbols
byte house[8] = {
  0b00100,
  0b01110,
  0b01110,
  0b11111,
  0b11111,
  0b11111,
  0b10101,
  0b11111
};

byte thermometer[8] = {
  0b00100,
  0b01010,
  0b01010,
  0b01010,
  0b01010,
  0b10001,
  0b10001,
  0b01110
};

void setup() {
  Serial.begin(115200);

  // LCD initialization
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.createChar(0, house);
  lcd.createChar(1, thermometer);
  
  // BME280 Initialization
  if (!bme.begin(0x76)) {
    Serial.println("Sensor Error!");
    lcd.clear();
    lcd.print("Sensor Error!");
    while (1);
  }
}

void loop() {
  handleInput();
  drawMenu();
  updateMenu();

  int val = analogRead(SubPageSwapPin);
  Serial.println(val);
}

void handleInput() {
  if (millis() - lastInputTime < 400) return; 

  int val = analogRead(PageSwapPin);
  int subPageVal = analogRead(SubPageSwapPin);
  bool changed = false;

  if (val > 3000) {
    page--;
    changed = true;
  } else if (val < 1500) {
    page++;
    changed = true;
  }

  // Setting page
  if (page == SettingPage) {
    if (subPageVal > 3000){
      subPage++;
      changed = true;
      lastPage = -1;
    } else if (subPageVal < 1500) {
      subPage--;
      changed = true;
      lastPage = -1;
    }
  }

  if (!changed) return;

  lastInputTime = millis();

  if (page > 2) page = 0;
  if (page < 0) page = 2;

  if (subPage > 2) subPage = 0;
  if (subPage < 0) subPage = 2;
}

void drawMenu() {
  if (page == lastPage) return;

  lcd.clear();

  switch (page) {
    case HomePage:
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
      lcd.print(" HOME    ");
      lcd.write(byte(1));
      lcd.print(" LVIV");
      break;
    case SettingPage:
      lcd.setCursor(0, 0);
      lcd.print("---   Settings   ---");
      displaySettingMenu();
      break;
    case 2:
      lcd.setCursor(0, 0);
      lcd.print("Page 3 (Info)");
      break;
  }

  lastPage = page;
}

void displaySettingMenu(){
  switch (subPage){
    case 0:
      lcd.setCursor(0, 1);
      lcd.print("* Option 1 [+]");
      lcd.setCursor(0, 2);
      lcd.print("Option 2 [-]");
      lcd.setCursor(0, 3);
      lcd.print("Option 3 [-]");
      break;
    case 1:
      lcd.setCursor(0, 1);
      lcd.print("Option 1 [+]");
      lcd.setCursor(0, 2);
      lcd.print("* Option 2 [-]");
      lcd.setCursor(0, 3);
      lcd.print("Option 3 [-]");
      break;
    case 2:
      lcd.setCursor(0, 1);
      lcd.print("Option 1 [+]");
      lcd.setCursor(0, 2);
      lcd.print("Option 2 [-]");
      lcd.setCursor(0, 3);
      lcd.print("* Option 3 [-]");
      break;
  }
}

void updateMenu(){
  float temp = bme.readTemperature();
  float hum = bme.readHumidity();
  float pres = bme.readPressure() / 100.0F;
  
  Serial.print("Temp: "); Serial.print(temp);
  Serial.print(" Hum: "); Serial.print(hum);
  Serial.print(" Pres: "); Serial.println(pres);

  switch (page) {
    case HomePage:
      lcd.setCursor(0, 1);
      lcd.print("T : ");
      lcd.print(temp, 1);
      //
      lcd.print("  N/A     ");
      lcd.print((char)223);
      lcd.print("C");

      lcd.setCursor(0, 2);
      lcd.print("H : ");
      lcd.print(hum, 1);
      //
      lcd.print("  N/A      %");

      lcd.setCursor(0, 3);
      lcd.print("P :");
      lcd.print(pres, 1);
      //
      lcd.print("  N/A    hPa");
      break;
    case SettingPage:
      lcd.setCursor(0, 0);
      lcd.print("---   Settings   ---");
      break;
    case 2:
      lcd.setCursor(0, 0);
      lcd.print("Page 3 (Info)");
      break;
  }
}