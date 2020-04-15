#include <FS.h>
#include <Arduino.h>

#define MAX_SCHEDULES 5

struct Config {
    String name;
    int schedule[MAX_SCHEDULES];
};

// Loads the configuration from a file
void loadConfiguration(const char *filename, Config &config) {
  // Open file for reading
  File file = SPIFFS.open(filename, "r");

  StaticJsonDocument<256> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.print("Failed to read ");Serial.print(filename);Serial.print(": ");Serial.println(error.c_str());
    Serial.println("Using default configuration");
  }

  // Copy values from the JsonDocument to the Config
  config.name = doc["name"] | "unassigned";
  // config.schedule = doc["schedule"] | [];
  JsonArray array = doc["schedule"];
  int index = 0;
  for(JsonVariant v : array) {
    config.schedule[index] = v.as<int>();
    index++;
  }

  // Close the file (Curiously, File's destructor doesn't close the file)
  file.close();
}

// Saves the configuration to a file
void saveConfiguration(const char *filename, const Config &config) {
  // Delete existing file, otherwise the configuration is appended to the file
  SPIFFS.remove(filename);

  // Open file for writing
  File file = SPIFFS.open(filename, "w");
  if (!file) {
    Serial.println(F("Failed to create file"));
    return;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<256> doc;

  // Set the values in the document
  doc["name"] = config.name;

  JsonArray array = doc.createNestedArray("schedule");
  for (int i = 0; i < MAX_SCHEDULES; i++ ) {
    array.add(config.schedule[i]);
  }

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file
  file.close();
}
