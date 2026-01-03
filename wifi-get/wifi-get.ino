#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <Preferences.h> // Для збереження налаштувань у пам'ять

// --- ЗМІННІ ДЛЯ ЗБЕРЕЖЕННЯ ---
char apiKey[50] = "";  // Сюди запишемо ключ
char lat[15] = "";     // Широта
char lon[15] = "";     // Довгота

// Прапор для збереження даних
bool shouldSaveConfig = false;

// --- ОБ'ЄКТИ ---
LiquidCrystal_I2C lcd(0x27, 20, 4);
Preferences preferences; // Це як EEPROM, але краще

// Callback, коли натиснули "Save" у WiFiManager
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setup() {
  Serial.begin(115200);
  
  // 1. Ініціалізація LCD
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Booting...");

  // 2. Читання збережених даних з пам'яті
  preferences.begin("weather-app", false); // Відкриваємо простір імен "weather-app"
  String str_lat = preferences.getString("lat", "50.45"); // Дефолт Київ
  String str_lon = preferences.getString("lon", "30.52");
  String str_key = preferences.getString("apikey", "");   // Пустий за замовчуванням
  
  // Конвертуємо String у char array для WiFiManager
  str_lat.toCharArray(lat, 15);
  str_lon.toCharArray(lon, 15);
  str_key.toCharArray(apiKey, 50);

  // 3. Налаштування WiFiManager
  WiFiManager wm;
  
  // Створюємо власні поля для введення
  WiFiManagerParameter custom_api_key("apikey", "OpenWeather API Key", apiKey, 50);
  WiFiManagerParameter custom_lat("lat", "Latitude (e.g. 50.45)", lat, 15);
  WiFiManagerParameter custom_lon("lon", "Longitude (e.g. 30.52)", lon, 15);

  // Додаємо поля в меню
  wm.addParameter(&custom_api_key);
  wm.addParameter(&custom_lat);
  wm.addParameter(&custom_lon);

  wm.setSaveConfigCallback(saveConfigCallback);

  // КНОПКА СКИДАННЯ (опціонально)
  // Якщо хочеш скинути налаштування, розкоментуй і затисни BOOT при старті
  wm.resetSettings(); preferences.clear();

  lcd.setCursor(0,0);
  lcd.print("Connecting WiFi...");
  
  // 4. Головна магія: Автопідключення або створення точки доступу
  // Якщо не підключився -> створить мережу "ESP32_Weather_Setup" без пароля
  if (!wm.autoConnect("ESP32_Weather_Setup")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    ESP.restart(); // Перезавантажити і спробувати знову
  }

  // 5. Якщо ми тут — ми підключилися!
  lcd.clear();
  lcd.print("WiFi Connected!");
  Serial.println("Connected... yeey :)");

  // 6. Зберігаємо нові параметри, якщо їх ввели
  if (shouldSaveConfig) {
    strcpy(apiKey, custom_api_key.getValue());
    strcpy(lat, custom_lat.getValue());
    strcpy(lon, custom_lon.getValue());

    Serial.println("Saving config...");
    preferences.putString("lat", lat);
    preferences.putString("lon", lon);
    preferences.putString("apikey", apiKey);
    preferences.end();
  }

  // Показуємо IP
  lcd.setCursor(0,1);
  lcd.print(WiFi.localIP());
  delay(2000);
  
  // Перший запит погоди
  getWeather();
}

void loop() {
  // Оновлюємо погоду кожні 60 секунд
  static unsigned long lastTime = 0;
  if (millis() - lastTime > 60000) {
    getWeather();
    lastTime = millis();
  }
}

void getWeather() {
  if (WiFi.status() != WL_CONNECTED) return;
  
  // Перевірка чи введений ключ
  if (String(apiKey) == "") {
    lcd.clear();
    lcd.print("No API Key!");
    lcd.setCursor(0,1);
    lcd.print("Connect to AP");
    lcd.setCursor(0,2);
    lcd.print("ESP32_Weather_Setup");
    return;
  }

  HTTPClient http;
  // Формуємо URL з координатами
  String url = "http://api.openweathermap.org/data/2.5/weather?lat=" + String(lat) + "&lon=" + String(lon) + "&appid=" + String(apiKey) + "&units=metric";
  
  Serial.println(url);
  
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024); // Для версії 6, для 7 - JsonDocument doc;
    deserializeJson(doc, payload);

    const char* cityName = doc["name"]; // Назва міста з сервера!
    float temp = doc["main"]["temp"];
    int hum = doc["main"]["humidity"];

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(cityName); // Виводимо назву міста, яку повернув сервер
    
    lcd.setCursor(0,1);
    lcd.print("Temp: "); lcd.print(temp, 1); lcd.print(" C");
    
    lcd.setCursor(0,2);
    lcd.print("Hum:  "); lcd.print(hum); lcd.print(" %");
    
    lcd.setCursor(0,3);
    lcd.print("Lat/Lon OK");
  } else {
    lcd.clear();
    lcd.print("HTTP Error");
  }
  http.end();
}