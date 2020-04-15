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
const long utcOffsetInSeconds = -8 * 3600; // -8 for PST
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
int previousFeedDay = -1; // Last day the the beast was fed. Range is 0-6 (Sun, Mon, Tue... )

// SPIDFF SETUP
#include <FS.h> // Include the SPIFFS library
#include "Config.h"
const char *filename = "/config.json";

Config config;

void setup()
{
  delay(1000);
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Mounting FS...");
  if (SPIFFS.begin()) {
    // Should load default config if run for the first time
    Serial.println(F("Loading configuration..."));
    loadConfiguration(filename, config);
    Serial.print("Config: ");
    Serial.println(config.name);
    for(int i = 0; i < MAX_SCHEDULES; i++) Serial.print(config.schedule[i]);Serial.print(" ");
    Serial.println("");
    // Create configuration file
    Serial.println(F("Saving configuration..."));
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
  server.on("/name", HTTP_POST, [](AsyncWebServerRequest *request) {
    const char *PARAM_NAME = "name";
    Serial.print("POST /name?name=");

    if (request->hasParam(PARAM_NAME)) {
      String name = request->getParam(PARAM_NAME)->value();
      Serial.println(name);
      config.name = name;
      Serial.println(F("Saving configuration..."));
      saveConfiguration(filename, config);
      request->send(200);
    } else {
      Serial.println("");
      request->send(400, "text/plain", "name: String parameter required");
    }
    
  });
  // GET current scheduled
  server.on("/schedule", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("GET /schedule");
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    StaticJsonDocument<256> doc;
    doc["name"] = config.name;
     JsonArray array = doc.createNestedArray("schedule");
    for (int i = 0; i < MAX_SCHEDULES; i++ ) {
      array.add(config.schedule[i]);
    }
    serializeJson(doc, *response);
    request->send(response);
  });
  // POST schedule
  AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler("/schedule", [](AsyncWebServerRequest *request, JsonVariant &json) {
    JsonObject jsonObj = json.as<JsonObject>();
    // LOG
    String body;
    serializeJson(jsonObj, body);
    Serial.print("POST /schedule");
    Serial.print(" body: ");
    Serial.println(body);
    StaticJsonDocument<256> doc;
    doc["schedule"] = jsonObj;
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    serializeJson(doc, *response);

    String currentSchedule;
    serializeJson(doc, currentSchedule);

    request->send(response);
  });

  server.addHandler(handler);
  server.onNotFound(notFound);
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

void loop()
{
  // Serial.print(previousFeedDay);
  // Serial.print(" - ");
  // Serial.print(timeClient.getDay());
  // Serial.print(":");
  // Serial.print(timeClient.getHours());
  // Serial.print(":");
  // Serial.println(timeClient.getMinutes());
  if (isFeedingTime(timeClient.getHours() + 1, timeClient.getDay()))
  { // +1 for daylight savings?
    feed();
    previousFeedDay = timeClient.getDay();
  }
  delay(60 * 1000);
}

bool isFeedingTime(int currentHour, int currentDay)
{
  return currentHour == FEED_HR && previousFeedDay != currentDay;
}

void feed()
{
  Serial.println("FEED");
  myservo.attach(SERVO_PIN); // Attach/Detach motor each time bc otherwise its noisy while idle
  myservo.write(OPEN_POS);
  delay(OPEN_MS);
  myservo.write(CLOSE_POS);
  delay(DETACH_DELAY); // Let motor reach pos before detaching
  myservo.detach();
}