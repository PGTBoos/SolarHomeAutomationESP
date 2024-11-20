<H1> Solar Home Automation ESP32 </H1>  
My cheap home automation project, (for solar cells and power regulation)     <br>
Based upon an ESP32 Wroom dev board      <br>
The idea here is to Monitor power production in winter time.      <br>
  
If enough power turn on a cheap heater. <br>
If power drops, turn it off for a minimal 5-minute rest (a cool-down period, for the Elchaepo device).   <br>
And don't keep it on longer then a certain period, if so cool down and repeat.   <br>
Also turns of if not enough power.   <br>

I wrote a set of libraries for it to control the various wifi modules, and i2C modules  

# hardware list
> averaged prices, homewizard is a bit pricy perhaps, but the ease of their wifi i licked it.<br>
> They also had a nice android app to see live data so not a bad deal.
> 
| Item | Price (€) |
|:--|--:|
| Sensor board BH1750 | 2.25 |
| Sensor board BME280 | 6.50 |
| OLED display 128*64 | 15.00 |
| ESP32 Wroom | 8.00 |
| Home Wizard P1 | 27.00 |
| HomeWizard switch | 27.95 |
| HomeWizard switch | 27.95 |
| HomeWizard switch | 27.95 |
| **Total** | **142.60** |
-----
# Wiring

and bread board setup todo..

-----

# Describe Main.cpp todo / to code 

basic wifi comm done, testing each module, 

-----
# Quick reference of the libraries I wrote.
**CURRENTLY THERE ARE LOT OF CHANGES HAPPENING HERE THIS MAY NOT BE UP TO DATE**

## DisplayManager Quick Reference

### Constructor & Init
```cpp
DisplayManager display;                // Create display instance
bool success = display.begin();        // Initialize (true if OK)
```

### Main Update Function
```cpp
display.updateDisplay(
    float importPower,    // Power in watts
    float exportPower,    // Power in watts
    float temp,          // Temperature in °C
    float humidity,      // Humidity in %
    float light,         // Light in lux
    bool sw1,           // Switch 1 state
    bool sw2,           // Switch 2 state
    bool sw3,           // Switch 3 state
    String sw1Time,     // Switch 1 time text
    String sw2Time,     // Switch 2 time text
    String sw3Time      // Switch 3 time text
);
```

### Individual Page Controls
```cpp
// Power display only
display.showPowerPage(
    float importPower,    // Power in watts
    float exportPower     // Power in watts
);

// Environment display only
display.showEnvironmentPage(
    float temp,          // Temperature in °C
    float humidity,      // Humidity in %
    float light          // Light in lux
);

// Switches display only
display.showSwitchesPage(
    bool switch1,        // Switch 1 state
    bool switch2,        // Switch 2 state
    bool switch3,        // Switch 3 state
    String sw1Time,      // Switch 1 time text
    String sw2Time,      // Switch 2 time text
    String sw3Time       // Switch 3 time text
);
```

### Hardware Setup
```cpp
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1        // Reset pin not used
#define SCREEN_ADDRESS 0x3C
```
- Display: SSD1306 OLED 128x64
- Protocol: I2C
- Auto-rotation: 3 seconds/page

### Dependencies
```cpp
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
```

----

## EnvironmentSensors Quick Reference

### Constructor & Init
```cpp
EnvironmentSensors sensors;            // Create sensors instance
bool success = sensors.begin();        // Initialize (true if any sensor OK)
```

### Main Update Function
```cpp
sensors.update();  // Updates all sensor readings
```

### Getter Methods
```cpp
float temp = sensors.getTemperature();  // Returns °C
float humidity = sensors.getHumidity(); // Returns %
float pressure = sensors.getPressure(); // Returns hPa
float light = sensors.getLightLevel();  // Returns lux
```

### Status Methods
```cpp
bool bmeStatus = sensors.hasBME280();   // BME280 sensor status
bool lightStatus = sensors.hasBH1750(); // Light sensor status
```

### Hardware Setup
```cpp
// BME280 Addresses (tries both)
#define BME280_ADDRESS_1 0x76
#define BME280_ADDRESS_2 0x77

// Sensors Used:
// - BME280: Temperature, Humidity, Pressure
// - BH1750: Light Level (Continuous High Res Mode)
```

### Default Values
```cpp
temperature = 0;  // °C
humidity = 0;     // %
pressure = 0;     // hPa
lightLevel = 0;   // lux
```

### Dependencies
```cpp
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>
```

### Notes
- Initializes I2C automatically
- Returns true if at least one sensor works
- BME280 tries both addresses (0x76, 0x77)
- BH1750 uses CONTINUOUS_HIGH_RES_MODE
- Pressure converted to hPa automatically

----

## HomeP1Device Quick Reference

### Constructor & Init
```cpp
HomeP1Device device("192.168.1.x");    // Create with IP address
```

### Main Update Function
```cpp
device.update();  // Updates power readings every 1 second
```

### Getter Methods
```cpp
float import = device.getCurrentImport();  // Get import power (W)
float export = device.getCurrentExport();  // Get export power (W)
float net = device.getNetPower();         // Get net power (import - export)
bool status = device.isConnected();       // Get connection status
```

### Direct Power Reading
```cpp
float importPower, exportPower;
bool success = device.getPowerData(importPower, exportPower);
```

### Constants
```cpp
const unsigned long READ_INTERVAL = 1000;  // Update interval (ms)
```

### API Endpoint
```cpp
baseUrl = "http://[IP_ADDRESS]"
endpoint = "/api/v1/data"
```

### Dependencies
```cpp
#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>
```

### JSON Response Format
```json
{
    "active_power_w": float  // Negative for export, Positive for import
}
```

### Notes
- Updates automatically every second
- Power values in Watts
- Negative power values = export
- Positive power values = import
- Uses 1KB JSON buffer
- HTTP GET request used

----

## HomeSocketDevice Quick Reference

### Constructor & Init
```cpp
HomeSocketDevice socket("192.168.1.x");    // Create with IP address
```

### Main Update Function
```cpp
socket.update();  // Updates state every 1 second
```

### Control Methods
```cpp
// Set socket state
bool success = socket.setState(true);   // Turn on
bool success = socket.setState(false);  // Turn off

// Get socket state
bool state = socket.getState();        // Get current state from device
```

### Getter Methods
```cpp
bool state = socket.getCurrentState();  // Get last known state
bool status = socket.isConnected();     // Get connection status
```

### Constants
```cpp
const unsigned long READ_INTERVAL = 1000;  // Update interval (ms)
```

### API Endpoints
```cpp
baseUrl = "http://[IP_ADDRESS]"
endpoints = {
    get: "/api/v1/state",    // GET request
    set: "/api/v1/state"     // PUT request
}
```

### JSON Formats
```json
// GET Response:
{
    "power_on": bool
}

// PUT Request:
{
    "power_on": true/false
}
```

### Dependencies
```cpp
#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>
```

### Notes
- Updates automatically every second
- Uses 256B JSON buffer
- Maintains last known state on failed reads
- HTTP GET for status
- HTTP PUT for control
- Content-Type: application/json

-----

## TimeSync Quick Reference

### Constructor & Init
```cpp
TimeSync timeSync;              // Create instance
timeSync.begin();              // Initialize and sync with NTP
```

### Time Methods
```cpp
// Get current time as string (HH:MM:SS)
String time = timeSync.getCurrentTime();

// Get current hour and minute
int hour, minute;
timeSync.getCurrentHourMinute(hour, minute);

// Get minutes since midnight
int minutes = timeSync.getCurrentMinutes();

// Check if current time is between two times
bool active = timeSync.isTimeBetween("14:00", "16:30");
```

### NTP Configuration
```cpp
const char* ntpServer = "pool.ntp.org"
const long gmtOffset_sec = 3600      // UTC+1 (Netherlands)
const int daylightOffset_sec = 3600  // +1 hour DST
```

### Time Format Patterns
```cpp
time_string = "HH:MM:SS"      // Time output format
time_range = "HH:MM"          // Format for isTimeBetween()
```

### Dependencies
```cpp
#include <Arduino.h>
#include <time.h>
```

### Notes
- Tries 10 times to sync with NTP server
- 1 second delay between sync attempts
- Validates time is past year 2024
- Handles overnight time periods in isTimeBetween()
- Returns "Failed to obtain time" if sync fails
- All times are in 24-hour format
- Uses local time (not UTC)

### Example Usage
```cpp
// Get current time
String now = timeSync.getCurrentTime();  // "14:30:45"

// Check if within time range
if (timeSync.isTimeBetween("23:00", "06:00")) {
    // Handle overnight period
}

// Get components
int hour, minute;
timeSync.getCurrentHourMinute(hour, minute);  // hour=14, minute=30
```

-----

## WebInterface Quick Reference

### Constructor & Init
```cpp
WebInterface webInterface;      // Create server on port 80
webInterface.begin();          // Initialize server and SPIFFS
webInterface.update();         // Handle client requests
```

### API Endpoints

#### GET Endpoints
```cpp
GET /                 // Serves index.html from SPIFFS
GET /data            // Returns JSON with all sensor data
```

#### POST Endpoints
```cpp
POST /switch/1       // Control switch 1
POST /switch/2       // Control switch 2
POST /switch/3       // Control switch 3
```

### JSON Formats

#### GET /data Response
```json
{
    "import_power": float,
    "export_power": float,
    "temperature": float,
    "humidity": float,
    "light": float,
    "switches": [
        {
            "state": boolean,
            "duration": "Xh Ym"
        }
    ]
}
```

#### POST /switch/{1,2,3} Request
```json
{
    "state": boolean
}
```

#### POST /switch/{1,2,3} Response
```json
{
    "success": boolean
}
```

### Dependencies
```cpp
#include <Arduino.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
```

### Notes
- Server runs on port 80
- Requires SPIFFS for web files
- Uses 1KB JSON buffer for data
- Uses 200B JSON buffer for switch control
- Serves static files from SPIFFS
- All responses are JSON or HTML
- Error codes: 404 (Not Found), 400 (Bad Request)

### Required SPIFFS Files
```
/index.html          // Main web interface
```
