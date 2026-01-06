#ifndef Globals_H

#define Globals_H

#include <WiFiManager.h>
#include <Preferences.h>

#define PageSwapPin 32
#define SubPageSwapPin 35
#define OkButtonPin 26

#define HomePage 0
#define SettingPage 1
#define WifiInfoPage 2


WiFiManager wm;
bool shouldConnect = false;

Preferences preferences;

LiquidCrystal_I2C lcd(0x27, 20, 4);
Adafruit_BME280 bme;

int page = 0;
int subPage = 0;
int lastPage = -1;
bool okButtonWasReleased = true;
unsigned long lastInputTime = 0;
unsigned long weatherTimer = 0;

struct SettingsData {
  bool APEnabled;
  bool backlight;
} settings;

struct ApiData {
  String city;
  String temp;
  String clouds;
  String humidity;
  String pressure;
  float lat;
  float lon;

  ApiData(){
    city = "N/A";
    temp = "N/A";
    clouds = "N/A";
    humidity = "N/A";
    pressure = "N/A";
  }

  void init(){
    city = preferences.getString("city", "N/A");
    lat = preferences.getFloat("lat", 0);
    lon = preferences.getFloat("lon", 0);
  }
} apiData;

struct GeoLocation {
  float lat;
  float lon;
  String name;
  String country;
  bool found;
};

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


#endif