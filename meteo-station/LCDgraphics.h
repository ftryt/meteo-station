#ifndef LCDgraphics_H
#define LCDgraphics_H

#include "Globals.h"
#include <esp_wifi.h>
#include <soc/gpio_struct.h>  // GPIO access

const char* wl_status_to_string(wl_status_t status) {
  switch (status) {
    case WL_NO_SHIELD: return "NO SHIELD";
    case WL_IDLE_STATUS: return "IDLE STATUS";
    case WL_NO_SSID_AVAIL: return "NO SSID AVAIL";
    case WL_SCAN_COMPLETED: return "SCAN COMPLETED";
    case WL_CONNECTED: return "CONNECTED";
    case WL_CONNECT_FAILED: return "CONNECT FAILED";
    case WL_CONNECTION_LOST: return "CONNECTION LOST";
    case WL_DISCONNECTED: return "DISCONNECTED";
    default: return "UNKNOWN";
  }
}

const char* getModeString(wifi_mode_t mode) {
  switch (mode) {
    case WIFI_OFF: return "OFF";
    case WIFI_STA: return "STA";
    case WIFI_AP:  return "AP";
    case WIFI_AP_STA: return "AP+STA";
    default: return "UNKNOWN";
  }
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
    if (subPageVal > 3000) {
      subPage++;
      changed = true;
      lastPage = -1;
    } else if (subPageVal < 1500) {
      subPage--;
      changed = true;
      lastPage = -1;
    }

    // digitalRead(OkButtonPin)
    // Read entire 32-bit GPIO input register using GPIO.in and take needed bit
    bool btnState = (GPIO.in >> OkButtonPin) & 1;

    // Button pressed (0)
    if (!btnState && okButtonWasReleased) {
      okButtonWasReleased = false;
      lastPage = -1;
      switch (subPage) {
        case 0:
          settings.APEnabled = !settings.APEnabled;
            // && !wm.getConfigPortalActive()
          if (settings.APEnabled){
            Serial.println("Button Pressed, Starting Config Portal");
            wm.startConfigPortal("Meteo-Station-ESP32");
          } else {
            Serial.println("Stop Config Portal");
            wm.stopConfigPortal();
          }
          break;
        case 1:
          settings.backlight = !settings.backlight;
          if (settings.backlight) lcd.backlight();
          else lcd.noBacklight();
          break;
        case 2:
          preferences.clear();
          wm.resetSettings();
          break;
      }
    }
    // Button released (1)
    if (btnState) okButtonWasReleased = true;
  }

  if (!changed) return;

  lastInputTime = millis();

  if (page > 2) page = 0;
  if (page < 0) page = 2;

  if (subPage > 2) subPage = 0;
  if (subPage < 0) subPage = 2;
}

void drawSettingMenuLine(uint8_t row, const char* label, bool selected, bool enabled, bool oneAction) {
  lcd.setCursor(0, row);
  lcd.print(selected ? "* " : "  ");
  lcd.print(label);
  // No need for +/- for "Clear memory" etc...
  if (!oneAction) lcd.print(enabled ? " [+]" : " [-]");
}

void displaySettingMenu() {
  const char* menuLabels[] = {
    "Accsess point",
    "BackLight",
    "Clear memory"
  };

  const uint8_t MENU_ITEMS = 3;

  for (uint8_t i = 0; i < MENU_ITEMS; i++) {
    bool enabled = false;

    if (i == 0) enabled = settings.APEnabled;
    if (i == 1) enabled = settings.backlight;

    drawSettingMenuLine(i + 1, menuLabels[i], subPage == i, enabled, i == 2);
  }
}

// Only updates if page changes or if needed
void drawMenu() {
  if (page == lastPage) return;

  lcd.clear();

  switch (page) {
    case HomePage:
      // lcd.print(preferences.getString("city", "N/A"));
      break;
    case SettingPage:
      lcd.setCursor(0, 0);
      lcd.print("---   Settings   ---");
      displaySettingMenu();
      break;
    case WifiInfoPage:
      lcd.setCursor(0, 0);
      lcd.print("---   Wifi Info  ---");
      break;
  }

  lastPage = page;
}

// Update every tick
void updateMenu() {
  float temp = bme.readTemperature();
  float hum = bme.readHumidity();
  float pres = bme.readPressure() / 100.0F;

//   Serial.print("Temp: ");
//   Serial.print(temp);
//   Serial.print(" Hum: ");
//   Serial.print(hum);
//   Serial.print(" Pres: ");
//   Serial.println(pres);

  switch (page) {
    case HomePage:
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
      lcd.print(" HOME    ");
      lcd.write(byte(1));
      lcd.print(" ");
      lcd.print(apiData.city);
      
      lcd.setCursor(0, 1);
      lcd.print("T : ");
      lcd.print(temp, 1);
      //
      lcd.setCursor(10, 1);
      lcd.print(apiData.temp);
      lcd.setCursor(18, 1);
      lcd.print((char)223);
      lcd.print("C");

      lcd.setCursor(0, 2);
      lcd.print("H : ");
      lcd.print(hum, 1);
      //
      lcd.setCursor(10, 2);
      lcd.print(apiData.humidity);
      lcd.setCursor(19, 2);
      lcd.print("%");

      lcd.setCursor(0, 3);
      lcd.print("P :");
      lcd.print(pres, 1);
      //
      lcd.setCursor(10, 3);
      lcd.print(apiData.pressure);
      lcd.setCursor(17, 3);
      lcd.print("hPa");
      break;
    case SettingPage:
      break;
    case WifiInfoPage:
      lcd.setCursor(0, 1);
      if (WiFi.getMode() == WIFI_OFF) {
        lcd.print("Bad wifi mode!");
        break;
      }
      
      // Get NVS ssid
      wifi_config_t conf;
      esp_wifi_get_config(WIFI_IF_STA, &conf);
      const char* ssidRaw = reinterpret_cast<const char*>(conf.sta.ssid);
      
      lcd.print("SSID: ");
      
      String ssidStr = String(ssidRaw);
      if (ssidStr.length() > 14) {
        ssidStr = ssidStr.substring(0, 14);
      }
      lcd.print(ssidStr);

      lcd.setCursor(0, 2);
      lcd.print("STATUS: ");
      String statusStr = String(wl_status_to_string(WiFi.status()));
      if (statusStr.length() > 12) {
        ssidStr = ssidStr.substring(0, 12);
      }
      lcd.print(statusStr);

      lcd.setCursor(0, 3);
      lcd.print("Mode: ");
      lcd.print(getModeString(WiFi.getMode()));
      break;
  }
}


#endif