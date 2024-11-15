// WebServer.cpp
#include <Arduino.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
class WebInterface
{
private:
    WebServer server;

public:
    WebInterface() : server(80) {} // Port 80 for HTTP

    void begin()
    {
        if (!SPIFFS.begin(true))
        {
            Serial.println("An error occurred while mounting SPIFFS");
            return;
        }

        // Serve the main page
        server.on("/", HTTP_GET, [this]()
                  {
            File file = SPIFFS.open("/index.html", "r");
            if(!file) {
                server.send(404, "text/plain", "File not found");
                return;
            }
            server.streamFile(file, "text/html");
            file.close(); });

        // API endpoint for getting data
        server.on("/data", HTTP_GET, [this]()
                  {
            StaticJsonDocument<1024> doc;
            
            // Add power data
            doc["import_power"] = 1240;  // Replace with actual values
            doc["export_power"] = 0;

            // Add environmental data
            doc["temperature"] = 21.5;
            doc["humidity"] = 45;
            doc["light"] = 320;

            // Add switch states
            JsonArray switches = doc.createNestedArray("switches");
            
            for(int i = 0; i < 3; i++) {
                JsonObject sw = switches.createNestedObject();
                sw["state"] = true;  // Replace with actual state
                sw["duration"] = "1h 23m";  // Replace with actual duration
            }

            String response;
            serializeJson(doc, response);
            server.send(200, "application/json", response); });

        // API endpoint for controlling switches
        server.on("/switch/1", HTTP_POST, [this]()
                  { handleSwitch(1); });
        server.on("/switch/2", HTTP_POST, [this]()
                  { handleSwitch(2); });
        server.on("/switch/3", HTTP_POST, [this]()
                  { handleSwitch(3); });

        server.begin();
        Serial.println("Web server started");
    }

    void update()
    {
        server.handleClient(); // Handle web server requests
    }

private:
    void handleSwitch(int switchNumber)
    {
        if (!server.hasArg("plain"))
        {
            server.send(400, "text/plain", "Body not received");
            return;
        }

        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, server.arg("plain"));

        if (error)
        {
            server.send(400, "text/plain", "Invalid JSON");
            return;
        }

        bool state = doc["state"];
        // Handle the switch state change here
        // You'll implement this based on your HomeSocketDevice class

        // Send response
        server.send(200, "application/json", "{\"success\":true}");
    }
};