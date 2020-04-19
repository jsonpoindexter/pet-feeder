#include <Arduino.h>
#include ".\conf.h" // WIFI SSID/Password

// CONFIG
#define NAME "Maxi" // unique feeder Name
#define FEED_HR 6   // time to feed in HR 0-23. Set to 5 for 6am
#define SERVO_PIN 13
#define OPEN_POS 0        // Open pos of feeder motor
#define CLOSE_POS 90      // Close pos of feeder motor
#define OPEN_MS 1000      // How long to keep feeder open
#define DETACH_DELAY 1000 // How long wait before detaching servo

// SERVO SETUP
#include <Servo.h>
Servo myservo;

// WIFI SETUP
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
const char *ssid = STASSID;
const char *password = STAPSK;

// WEBSERVER SETUP
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"
AsyncWebServer server(80);
void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

// TIME SETUP
#include <NTPClient.h>
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// SPIDFF SETUP
#include <FS.h> // Include the SPIFFS library
#include "Config.h"
const char *filename = "/config.json";
Config config;

// FEED SCHEDULE SETUP
unsigned long previousMillis = 0;
const long interval = 5000;           // interval at which to check feed times (ms)

void setup()
{
  delay(1000);
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Mounting FS...");
  if (SPIFFS.begin()) {
    // Should load default config if run for the first time
    loadConfiguration(filename, config);
    // Create configuration file
    saveConfiguration(filename, config);
  }

  // WIFI INIT
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Connecting to WIFI...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  // WEBSERVER INIT
  // POST name
  // Set name
  server.on("/name", HTTP_POST, [](AsyncWebServerRequest *request) {
    logger(request);
    const char *PARAM_NAME = "name";

    if (request->hasParam(PARAM_NAME)) {
      String name = request->getParam(PARAM_NAME)->value();
      config.name = name;
      saveConfiguration(filename, config);
      request->send(200);
    } else {
      request->send(400, "text/plain", "name: String parameter required");
    }
    
  });
  // GET current scheduled
  // Return current schedule and name
  server.on("/schedule", HTTP_GET, [](AsyncWebServerRequest *request) {
    logger(request);
    // Build json config for saving to file system
    StaticJsonDocument<256> doc;
    doc["name"] = config.name;
    JsonArray array = doc.createNestedArray("schedule");
    for (size_t i = 0; i < sizeof(config.schedule) / sizeof(uint32); i++ ) {
      if(config.schedule[i]) array.add(config.schedule[i]);
    }

    // Response
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    serializeJson(doc, *response);
    request->send(response);
  });

  // POST schedule
  // Set schedule for feederr
  AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler("/schedule", [](AsyncWebServerRequest *request, JsonVariant &jsonReq) {
    logger(request);
    JsonArray array = jsonReq.as<JsonArray>();

    // Build json config for saving to file system
    StaticJsonDocument<256> doc;
    doc["schedule"] = array;
    // Empty out current schedule
    for (size_t i = 0; i < sizeof(config.schedule) / sizeof(uint32); i++ ) {
       config.schedule[i] = 0;
    }
    int index = 0;
    for(JsonVariant v : array) {
      config.schedule[index] = v.as<uint32>();
      index++;
    }
    saveConfiguration(filename, config);

    // Rebuild schedule json array from config for response
    StaticJsonDocument<256> doc1;
    JsonArray array1 = doc1.createNestedArray("schedule");
    for (size_t i = 0; i < sizeof(config.schedule) / sizeof(uint32); i++ ) {
       if(config.schedule[i]) array1.add(config.schedule[i]);
    }

    // Response
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    serializeJson(doc1, *response);
    request->send(response);
  });
  // POST feed
  // Activates feeding funcionality
  server.on("/feed", HTTP_POST, [](AsyncWebServerRequest *request) {
    logger(request);
    feed();
    request->send(200);
  });
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Authorization, Content-Type");
  server.addHandler(handler);
  server.onNotFound([](AsyncWebServerRequest *request) {
    if (request->method() == HTTP_OPTIONS) {
      request->send(200);
    } else {
      request->send(404);
    }
  });
  server.begin();

  // TIME INIT
  timeClient.begin();
  timeClient.update();
  Serial.println("Fetched NTP Time");

  // SERVO INIT
  myservo.attach(SERVO_PIN); // Attach/Detach motor each time bc otherwise its noisy while idle
  myservo.write(CLOSE_POS);
  delay(DETACH_DELAY); // Let motor reach pos before detaching
  myservo.detach();
  delay(1000);
}

void loop() {
  checkFeedSchedule();
}

void checkFeedSchedule() {
 unsigned long currentMillis = millis(); 
 if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    // Check all current feed times and compare to current
    for (size_t i = 0; i < sizeof(config.schedule) / sizeof(uint32); i++ ) {
      if( config.schedule[i] && timeClient.getEpochTime() >= config.schedule[i]) {
        feed();
        // Increase fzeed time by 24hr
        config.schedule[i] = (config.schedule[i] + (24 * 60 * 60));
        // saveConfiguration(filename, config);
      }
    }
  }
}

void feed() {
  myservo.attach(SERVO_PIN); // Attach/Detach motor each time bc otherwise its noisy while idle
  myservo.write(OPEN_POS);
  // delay(OPEN_MS);
  myservo.write(CLOSE_POS);
  // delay(DETACH_DELAY); // Let motor reach pos before detaching
  myservo.detach();
}

void logger(AsyncWebServerRequest *request) {
  Serial.print(request -> methodToString());Serial.print(" ");
  Serial.print(request -> url());Serial.print(" - ");
  Serial.print(request -> host());Serial.print(" - ");
  Serial.println(request -> requestedConnTypeToString());
}