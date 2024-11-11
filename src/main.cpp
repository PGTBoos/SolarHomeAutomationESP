#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <SPIFFS.h>

// Config structure
struct Config
{
  char wifi_ssid[32];
  char wifi_password[32];
  char p1_ip[16];
  char socket_ip[16];
  int power_on_threshold;
  int power_off_threshold;
  int min_on_time;
  int min_off_time;
  int max_on_time;
};

// Global variables
Config config;

// Load configuration from SPIFFS
bool loadConfig()
{
  Serial.println("\n=== Loading Configuration ===");
  if (!SPIFFS.begin(true))
  {
    Serial.println("SPIFFS Mount Failed");
    return false;
  }

  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile)
  {
    Serial.println("Failed to open config file");
    return false;
  }

  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, configFile);
  configFile.close();

  if (error)
  {
    Serial.print("Failed to parse config file: ");
    Serial.println(error.c_str());
    return false;
  }

  strlcpy(config.wifi_ssid, doc["wifi_ssid"] | "", sizeof(config.wifi_ssid));
  strlcpy(config.wifi_password, doc["wifi_password"] | "", sizeof(config.wifi_password));

  Serial.println("Configuration loaded successfully");
  Serial.print("SSID: ");
  Serial.println(config.wifi_ssid);
  return true;
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  if (!loadConfig())
  {
    Serial.println("Using default config");
  }

  Serial.print("Connecting to WiFi");
  WiFi.begin(config.wifi_ssid, config.wifi_password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20)
  {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("\nWiFi connection failed!");
  }
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi disconnected. Reconnecting...");
    WiFi.begin(config.wifi_ssid, config.wifi_password);
  }
  delay(1000);
}