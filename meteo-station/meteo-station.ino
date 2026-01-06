#include <LiquidCrystal_I2C.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <esp_wifi.h>
#include "Globals.h"
#include "LCDgraphics.h"
#include "FetchApi.h"

char inputCity[25] = "";
WiFiManagerParameter custom_city_key("City", "Enter your city (to specify use ',' eg. London,CA)", inputCity, 25);

// Dump data from NVS
void printSavedCredentials() {
  // ESP can crash or return garbage
  if (WiFi.getMode() == WIFI_OFF) {
    WiFi.mode(WIFI_STA); 
  }

  wifi_config_t conf;
  
  // Config from the ESP-IDF driver
  // WIFI_IF_STA means "Station Interface" (the client mode)
  esp_wifi_get_config(WIFI_IF_STA, &conf);

  // Cast the raw bytes to char*
  const char* ssid = reinterpret_cast<const char*>(conf.sta.ssid);
  const char* password = reinterpret_cast<const char*>(conf.sta.password);

  Serial.println("--- DUMPING NVS CREDENTIALS ---");
  
  if (String(ssid).length() > 0) {
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("Password: ");
    Serial.println(password);
  } else {
    Serial.println("No credentials saved in NVS.");
  }
  
  Serial.println("-------------------------------");
}

// Callback, when pressed "Save" in WiFiManager
void saveConfigCallback() {
  Serial.println("Credentials saved");

  // Reload menu
  settings.APEnabled = false;
  lastPage = -1;

  String newSSID = wm.getWiFiSSID();
  String newPass = wm.getWiFiPass();

  Serial.print("New SSID: ");
  Serial.println(newSSID);

  strcpy(inputCity, custom_city_key.getValue());

  shouldConnect = true;
}

void setup() {
  // Preferences
  preferences.begin("weather-app", false);
  apiData.init();
  
  // Wifi setup
  WiFi.mode(WIFI_STA);
  wm.setSaveConfigCallback(saveConfigCallback);
  wm.setConnectTimeout(1);
  wm.setBreakAfterConfig(true);
  wm.setConfigPortalBlocking(false);
  wm.addParameter(&custom_city_key);

  // Serial setup
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("Meteo station started ...");

  printSavedCredentials();

  pinMode(OkButtonPin, INPUT_PULLUP);

  // Settings setup
  settings.APEnabled = false;
  settings.backlight = true;
  shouldConnect = true; // Always try to connect to previous network

  // LCD initialization
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.createChar(0, house);
  lcd.createChar(1, thermometer);

  // BME280 Initialization
  if (!bme.begin(0x76)) {
    Serial.println("Sensor Error!");
    lcd.clear();
    lcd.print("Sensor Error!");
    while (1)
      ;
  }
}

void loop() {
  handleInput();
  drawMenu();
  updateMenu();

  // int val = digitalRead(OkButtonPin);
  // Serial.println(val);

  wm.process();

  if (shouldConnect) {
    shouldConnect = false;

    WiFi.mode(WIFI_STA);

    Serial.println("Initiating Background Connection...");
    WiFi.begin();
  }

  // Update weater logic
  if (millis() - weatherTimer > 5000) {
    Serial.print("[Loop] City: ");
    Serial.println(inputCity);


    if (strlen(inputCity) > 0) {
      GeoLocation returnLoc = getGeoFromCity(String(inputCity));

      if (returnLoc.found) {
        inputCity[0] = '\0';
        getWeather();
      } else {
        Serial.println("[Loop] Failed to get city ...");
      }
    } else getWeather();
    
    // getGeoFromCity("Lviv");

    weatherTimer = millis();
  }
}