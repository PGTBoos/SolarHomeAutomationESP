#include "HomeSocketDevice.h"

HomeSocketDevice::HomeSocketDevice(const char *ip)
    : baseUrl("http://" + String(ip)), lastKnownState(false), lastReadTime(0),
      lastReadSuccess(false), consecutiveFailures(0), deviceIP(ip),
      lastLogTime(0) {
  Serial.printf("Initializing socket device at IP: %s\n", ip);
}

void HomeSocketDevice::update() {
  unsigned long currentTime = millis();

  // Only try updating if enough time has passed since last attempt
  if (currentTime - lastReadTime < READ_INTERVAL) {
    return;
  }

  // Calculate backoff time based on failures (max 60 seconds)
  unsigned long backoffTime = min(consecutiveFailures * 5000UL, 60000UL);
  if (currentTime - lastReadTime < backoffTime) {
    return;
  }

  // Regular status check
  if (!getState()) {
    consecutiveFailures++;
    if (currentTime - lastLogTime >= 30000) {
      Serial.printf("PowerSocket > %s > Status > Offline (retry in %lu sec)\n",
                    deviceIP.c_str(), backoffTime / 1000);
      lastLogTime = currentTime;
    }
  } else {
    if (consecutiveFailures > 0) {
      Serial.printf("PowerSocket > %s > Status > Back online\n",
                    deviceIP.c_str());
      lastLogTime = currentTime;
    }
    consecutiveFailures = 0;
  }

  lastReadTime = currentTime;
}

bool HomeSocketDevice::makeHttpRequest(const String &endpoint,
                                       const String &method,
                                       const String &payload,
                                       String &response) {
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  WiFiClient newClient;
  HTTPClient http;
  newClient.setTimeout(5000);

  String fullUrl = baseUrl + "/api/v1/state";

  if (!http.begin(newClient, fullUrl)) {
    newClient.stop();
    return false;
  }

  http.setTimeout(5000);
  http.setReuse(false);

  int httpCode;
  if (method == "GET") {
    httpCode = http.GET();
  } else if (method == "PUT") {
    http.addHeader("Content-Type", "application/json");
    httpCode = http.PUT(payload);
  }

  bool success = (httpCode == HTTP_CODE_OK);
  if (success) {
    response = http.getString();
  }

  http.end();
  newClient.stop();
  return success;
}

// for raw output enable below code
// bool HomeSocketDevice::makeHttpRequest(const String &endpoint, const String
// &method, const String &payload, String &response)
// {
//     if (WiFi.status() != WL_CONNECTED)
//     {
//         Serial.printf("Power socket > No WiFi\n");
//         return false;
//     }

//     WiFiClient newClient;
//     HTTPClient http;
//     newClient.setTimeout(5000);

//     String fullUrl = baseUrl + "/api/v1/state";

//     if (!http.begin(newClient, fullUrl))
//     {
//         Serial.printf("Power socket > %s > Connection failed\n",
//         fullUrl.c_str()); return false;
//     }

//     http.setTimeout(5000);
//     http.setReuse(false);

//     int httpCode;
//     if (method == "GET")
//     {
//         httpCode = http.GET();
//     }
//     else if (method == "PUT")
//     {
//         http.addHeader("Content-Type", "application/json");
//         httpCode = http.PUT(payload);
//     }

//     if (httpCode == HTTP_CODE_OK)
//     {
//         response = http.getString();
//         Serial.printf("Power socket > %s > %d > %s\n", fullUrl.c_str(),
//         httpCode, response.c_str());
//     }
//     else if (httpCode == -1)
//     {
//         Serial.printf("Power socket > %s > %d > disconnected\n",
//         fullUrl.c_str(), httpCode);
//     }
//     else
//     {
//         Serial.printf("Power socket > %s > %d > HTTP error\n",
//         fullUrl.c_str(), httpCode);
//     }

//     http.end();
//     return (httpCode == HTTP_CODE_OK);
// }

bool HomeSocketDevice::getState() {
  String response;
  if (!makeHttpRequest("/api/v1/state", "GET", "", response)) {
    Serial.printf("PowerSocket > %s/api/v1/state > Get > HTTP error\n",
                  deviceIP.c_str());
    lastReadSuccess = false;
    return false;
  }

  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, response);

  if (error) {
    Serial.printf("PowerSocket > %s/api/v1/state > Get > JSON error\n",
                  deviceIP.c_str());
    lastReadSuccess = false;
    return false;
  }

  lastKnownState = doc["power_on"] | false;
  Serial.printf("PowerSocket > %s/api/v1/state > Get > is %s\n",
                deviceIP.c_str(), lastKnownState ? "on" : "off");
  lastReadSuccess = true;
  return true;
}

bool HomeSocketDevice::setState(bool state) {
  StaticJsonDocument<200> doc;
  doc["power_on"] = state;
  String jsonString;
  serializeJson(doc, jsonString);

  String response;
  if (!makeHttpRequest("/api/v1/state", "PUT", jsonString, response)) {
    Serial.printf("PowerSocket > %s/api/v1/state > Put > HTTP error\n",
                  deviceIP.c_str());
    return false;
  }

  lastKnownState = state;
  Serial.printf("PowerSocket > %s/api/v1/state > Put > turn %s\n",
                deviceIP.c_str(), state ? "on" : "off");
  return true;
}