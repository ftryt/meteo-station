ESP32-based weather station project that collects environmental data (temperature, humidity, pressure) using a BME280 sensor and displays it on an LCD screen. The device connects to WiFi, fetches weather data from an external API, and allows user configuration (e.g. city selection) via a built-in WiFiManager portal.

The project includes handling of network connectivity, non-volatile storage (NVS), GPIO configuration, and a simple UI/menu system. It demonstrates practical embedded programming concepts such as sensor integration, asynchronous operations, and debugging on hardware level.

Technologies: C/C++, ESP32, Arduino framework, WiFiManager, REST API, I2C (LCD, BME280).
