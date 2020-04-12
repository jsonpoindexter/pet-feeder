#include <Arduino.h>

#include ".\conf.h" 

// CONFIG
#define FEED_HR 6 // time to feed in HR 0-23. Set to 5 for 6am
#define SERVO_PIN 13
#define OPEN_POS 0 // Open pos of feeder motor
#define CLOSE_POS 90 // Close pos of feeder motor
#define OPEN_MS 1000 // How long to keep feeder open
#define DETACH_DELAY 1000 // How long wait before detaching servo

// SERVO SETUP
#include <Servo.h>
Servo myservo; 

// WIFI SETUP
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
const char* ssid     = STASSID;
const char* password = STAPSK;

// TIME SETUP
#include <NTPClient.h>
const long utcOffsetInSeconds = -8 * 3600; // -8 for PST
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
int previousFeedDay = -1; // Last day the the beast was fed. Range is 0-6 (Sun, Mon, Tue... )


void setup() {
  delay(1000);
  Serial.begin(115200);
  

  // WIFI INIT
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Connecting to WIFI...");
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  Serial.println("Connected to WIFI");
  timeClient.begin();
  timeClient.update();
  Serial.println("Fetched NTP Time");
  WiFi.disconnect();
  Serial.println("Disconnect from WIFI");

  // SERVO INIT
  myservo.attach(SERVO_PIN); // Attach/Detach motor each time bc otherwise its noisy while idle
  myservo.write(CLOSE_POS); 
  delay(DETACH_DELAY); // Let motor reach pos before detaching
  myservo.detach(); 
  delay(1000);
}

void loop() {
  Serial.print(previousFeedDay);
  Serial.print(" - ");
  Serial.print(timeClient.getDay());
  Serial.print(":");
  Serial.print(timeClient.getHours());
  Serial.print(":");
  Serial.println(timeClient.getMinutes());
  if(isFeedingTime(timeClient.getHours() + 1, timeClient.getDay())) { // +1 for daylight savings?
    feed();
    previousFeedDay = timeClient.getDay();
  }
  delay(60 * 1000);
}


bool isFeedingTime(int currentHour, int currentDay) {
  return currentHour == FEED_HR && previousFeedDay != currentDay;
}

void feed() {
  Serial.println("FEED");
  myservo.attach(SERVO_PIN); // Attach/Detach motor each time bc otherwise its noisy while idle
  myservo.write(OPEN_POS); 
  delay(OPEN_MS);
  myservo.write(CLOSE_POS); 
  delay(DETACH_DELAY); // Let motor reach pos before detaching
  myservo.detach(); 
}