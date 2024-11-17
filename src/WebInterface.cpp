// WebServer.cpp
#include "WebInterface.h"

void WebInterface::begin()
{

    // More detailed SPIFFS check
    Serial.println("\n=== SPIFFS Detailed Check ===");
    if (!SPIFFS.begin(true))
    { // Added format on fail
        Serial.println("SPIFFS Mount Failed - Attempting format");
        if (!SPIFFS.begin(true))
        {
            Serial.println("SPIFFS Mount Still Failed");
            return;
        }
    }

    // Print total and used space
    Serial.printf("Total space: %d bytes\n", SPIFFS.totalBytes());
    Serial.printf("Used space: %d bytes\n", SPIFFS.usedBytes());

    // List all files with detailed info
    Serial.println("\nListing all files:");
    File root = SPIFFS.open("/");
    if (!root)
    {
        Serial.println("- Failed to open directory");
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println("- Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        String fileName = file.name();
        size_t fileSize = file.size();

        Serial.printf("File: %s\n", fileName.c_str());
        Serial.printf("  Size: %d bytes\n", fileSize);

        // Try to read first few bytes of each file
        if (fileSize > 0)
        {
            String info = fileName + " size: " + fileSize + " bytes";
            Serial.println(info);
        }
        file = root.openNextFile();
    }
    root.close();

    // Serve the main page at root URL
    server.on("/", HTTP_GET, [this]()
              {
        Serial.println("Handling root request");
        Serial.print("Client IP: ");
        Serial.println(server.client().remoteIP());
        serveFile("/data/index.html"); });

    // Serve static files
    server.on("/data/index.html", HTTP_GET, [this]()
              {
        Serial.println("Handling /index.html request");
        Serial.print("Client IP: ");
        Serial.println(server.client().remoteIP());
        serveFile("/data/index.html"); });

    // API endpoint for getting data
    server.on("/data", HTTP_GET, [this]()
              {
        Serial.println("Handling /data request");
        server.sendHeader("Content-Type", "application/json");
        server.sendHeader("Access-Control-Allow-Origin", "*");
        
        StaticJsonDocument<1024> doc;
        doc["import_power"] = 1240;
        doc["export_power"] = 0;
        doc["temperature"] = 21.5;
        doc["humidity"] = 45;
        doc["light"] = 320;

        JsonArray switches = doc.createNestedArray("switches");
        for(int i = 0; i < 3; i++) {
            JsonObject sw = switches.createNestedObject();
            sw["state"] = true;
            sw["duration"] = "1h 23m";
        }

        String response;
        serializeJson(doc, response);
        Serial.print("Sending JSON response (");
        Serial.print(response.length());
        Serial.println(" bytes)");
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
        Serial.print("Request not found: ");
        Serial.println(server.uri());
        Serial.print("Method: ");
        Serial.println(server.method() == HTTP_GET ? "GET" : 
                      server.method() == HTTP_POST ? "POST" : "OTHER");
        Serial.print("Client IP: ");
        Serial.println(server.client().remoteIP());
        
        if (!serveFile(server.uri())) {
            Serial.println("File not found, sending 404");
            server.send(404, "text/plain", "Not found");
        } });

    server.begin();
    Serial.print("Web server started on IP: ");
    Serial.println(WiFi.localIP());
}

void WebInterface::update()
{
    server.handleClient();
}

bool WebInterface::serveFile(const String &path)
{
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
    else if (path.endsWith(".png"))
        contentType = "image/png";
    else if (path.endsWith(".jpg"))
        contentType = "image/jpeg";
    else
        contentType = "text/plain";

    Serial.print("Attempting to serve: ");
    Serial.println(path);

    File file = SPIFFS.open(path, "r");
    if (!file)
    {
        Serial.printf("File not found: %s\n", path.c_str());
        return false;
    }

    Serial.printf("Heap before: %d bytes\n", ESP.getFreeHeap());
    size_t fileSize = file.size();
    Serial.printf("Heap after: %d bytes\n", ESP.getFreeHeap());

    Serial.print("File size: ");
    Serial.print(fileSize);
    Serial.println(" bytes");

    server.sendHeader("Connection", "keep-alive");
    server.sendHeader("Keep-Alive", "timeout=10, max=100");
    server.sendHeader("Content-Type", contentType);
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Access-Control-Allow-Origin", "*");

    bool success = server.streamFile(file, contentType) == fileSize;
    file.close();

    Serial.print("File served successfully: ");
    Serial.println(success ? "yes" : "no");

    return success;
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