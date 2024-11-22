// WebServer.cpp
#include "WebInterface.h"

void WebInterface::updateCache()
{
    if (p1Meter)
    {
        cached.import_power = p1Meter->getCurrentImport();
        cached.export_power = p1Meter->getCurrentExport();
    }
    cached.temperature = sensors.getTemperature();
    cached.humidity = sensors.getHumidity();
    cached.light = sensors.getLightLevel();

    cached.socket_states[0] = socket1 ? socket1->getCurrentState() : false;
    cached.socket_states[1] = socket2 ? socket2->getCurrentState() : false;
    cached.socket_states[2] = socket3 ? socket3->getCurrentState() : false;

    for (int i = 0; i < 3; i++)
    {
        cached.socket_durations[i] = millis() - lastStateChangeTime[i];
    }
}

void WebInterface::begin()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    server.client().setNoDelay(true);

    Serial.printf("Total space: %d bytes\n", SPIFFS.totalBytes());
    Serial.printf("Used space: %d bytes\n", SPIFFS.usedBytes());

    // Serve the main page at root URL
    server.on("/", HTTP_GET, [this]()
              { serveFile("/data/index.html"); });

    // API endpoint for getting data
    server.on("/data", HTTP_GET, [this]()
              {
        server.sendHeader("Content-Type", "application/json");
        server.sendHeader("Access-Control-Allow-Origin", "*");

        StaticJsonDocument<2048> doc;
        doc["import_power"] = cached.import_power;
        doc["export_power"] = cached.export_power;
        doc["temperature"] = cached.temperature;
        doc["humidity"] = cached.humidity;
        doc["light"] = cached.light;

        JsonArray switches = doc.createNestedArray("switches");
        for(int i = 0; i < 3; i++) {
            JsonObject sw = switches.createNestedObject();
            sw["state"] = cached.socket_states[i];
            sw["duration"] = String(cached.socket_durations[i]/1000) + "s";
        }

        String response;
        serializeJson(doc, response);
        server.send(200, "application/json", response); });

    // API endpoints for controlling switches
    server.on("/switch/1", HTTP_POST, [this]()
              { handleSwitch(1); });
    server.on("/switch/2", HTTP_POST, [this]()
              { handleSwitch(2); });
    server.on("/switch/3", HTTP_POST, [this]()
              { handleSwitch(3); });

    // Handle any other static files
    server.onNotFound([this]()
                      {
        if (!serveFile(server.uri())) {
            server.send(404, "text/plain", "Not found");
        } });

    server.begin();
    Serial.println("Web server started on IP: " + WiFi.localIP().toString());
}

void WebInterface::update()
{
    static unsigned long lastWebUpdate = 0;
    unsigned long now = millis();

    // Reset web server if it hasn't handled requests for too long
    if (now - lastWebUpdate > 30000)
    { // 30 seconds
        Serial.println("Web > Watchdog: Resetting web server");
        server.close();
        server.begin();
        lastWebUpdate = now;
    }
    server.handleClient();
    lastWebUpdate = now;
    // Update cache periodically
    static unsigned long lastCacheUpdate = 0;
    if (millis() - lastCacheUpdate >= 1000)
    { // Update every second
        updateCache();
        lastCacheUpdate = millis();
    }

    yield();

    unsigned long now = millis();
    if (now - lastCheck >= CHECK_INTERVAL)
    {
        lastCheck = now;
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("WiFi connection lost - attempting reconnect");
            WiFi.reconnect();
        }
    }
}

bool WebInterface::serveFile(const String &path)
{
    if (!buffer)
    {
        Serial.println("Buffer not allocated!");
        return false;
    }

    String contentType;
    if (path.endsWith(".html"))
        contentType = "text/html";
    else if (path.endsWith(".css"))
        contentType = "text/css";
    else if (path.endsWith(".js"))
        contentType = "application/javascript";
    else if (path.endsWith(".json"))
        contentType = "application/json";
    else if (path.endsWith(".ico"))
        contentType = "image/x-icon";
    else
        contentType = "text/plain";

    File file = SPIFFS.open(path, "r");
    if (!file)
    {
        return false;
    }

    size_t fileSize = file.size();

    server.sendHeader("Content-Type", contentType);
    server.sendHeader("Content-Length", String(fileSize));
    server.sendHeader("Connection", "close");
    server.sendHeader("Cache-Control", "no-cache");
    server.setContentLength(fileSize);
    server.send(200, contentType, "");

    size_t totalBytesSent = 0;
    while (totalBytesSent < fileSize)
    {
        if (!server.client().connected())
        {
            file.close();
            return false;
        }

        size_t bytesRead = file.read(buffer, min(BUFFER_SIZE, fileSize - totalBytesSent));
        if (bytesRead == 0)
            break;

        size_t bytesWritten = server.client().write(buffer, bytesRead);
        if (bytesWritten != bytesRead)
        {
            delay(50);
            continue;
        }

        totalBytesSent += bytesWritten;
        delay(1);
        yield();
    }

    file.close();
    return totalBytesSent == fileSize;
}

void WebInterface::handleSwitch(int switchNumber)
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
    server.sendHeader("Content-Type", "application/json");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", "{\"success\":true}");
}