#ifndef FetchApi_H

#define Globals_H

#include <HTTPClient.h>
#include <ArduinoJson.h>

String apiKey = "4b6b9e2c8de7b737b43fac41796e57b3";

void getWeather() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[Weather] WiFi not connected!");
    return;
  };

  if (String(apiKey) == "") {
    Serial.println("No api key!");
    return;
  }

  if (apiData.lat == 0 || apiData.lon == 0){
    Serial.println("[Weather] Lat or lon is not set!");
    return;
  }

  HTTPClient http;

  String url = "http://api.openweathermap.org/data/2.5/weather?lat=" + String(apiData.lat) + "&lon=" + String(apiData.lon) + "&appid=" + apiKey + "&units=metric";

  // Serial.println(url);

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();
    JsonDocument doc;
    deserializeJson(doc, payload);

    // apiData.city = String(doc["sys"]["country"]) + "," + String(doc["name"]);
    apiData.temp = String(doc["main"]["temp"]);
    apiData.clouds = String(doc["weather"][0]["main"]);
    apiData.humidity = String(doc["main"]["humidity"]);
    apiData.pressure = String(doc["main"]["pressure"]);

    Serial.println(apiData.city);
    Serial.println(apiData.temp);
    Serial.println(apiData.clouds);
    Serial.println(apiData.humidity);
    Serial.println(apiData.pressure);
  } else {
    Serial.println("HTTP Error");
  }
  http.end();
}

GeoLocation getGeoFromCity(String cityName) {
  GeoLocation loc;
  loc.found = false;

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[Geo] WiFi not connected!");
    return loc;
  }

  HTTPClient http;

  cityName.replace(" ", "%20");

  // Endpoint: http://api.openweathermap.org/geo/1.0/direct?q={city name}&limit=1&appid={API key}
  String url = "http://api.openweathermap.org/geo/1.0/direct?q=" + cityName + "&limit=1&appid=" + apiKey;

  Serial.print("[Geo] Looking up: "); 
  Serial.println(cityName);
  
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    
    JsonDocument doc; 
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      // Check if the array is empty (City not found)
      if (doc.size() > 0) {
        loc.name = doc[0]["name"].as<String>();
        loc.country = doc[0]["country"].as<String>();
        loc.lat = doc[0]["lat"];
        loc.lon = doc[0]["lon"];
        loc.found = true;

        Serial.println("[Geo] Success!");
        Serial.print("      Found: "); Serial.print(loc.name); Serial.print(", "); Serial.println(loc.country);
        Serial.print("      Lat: "); Serial.println(loc.lat, 4);
        Serial.print("      Lon: "); Serial.println(loc.lon, 4);

        apiData.city = loc.country + "," + loc.name;
        preferences.putString("city", apiData.city);
        apiData.lat = loc.lat;
        preferences.putFloat("lat", loc.lat);
        apiData.lon = loc.lon;
        preferences.putFloat("lon", loc.lon);
      } else {
        Serial.println("[Geo] City not found.");
      }
    } else {
      Serial.print("[Geo] JSON Error: "); 
      Serial.println(error.c_str());
    }
  } else {
    Serial.print("[Geo] HTTP Error: "); 
    Serial.println(httpCode);
  }

  http.end();
  return loc;
}

#endif