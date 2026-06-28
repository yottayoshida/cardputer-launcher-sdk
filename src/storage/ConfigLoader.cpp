// SPDX-License-Identifier: MIT OR Apache-2.0

#include "storage/ConfigLoader.h"

#include <ArduinoJson.h>
#include <SD.h>

namespace cardputer_launcher {

namespace {

bool ensureSd(bool available, String& error) {
  if (!available) {
    error = "SD unavailable";
    return false;
  }
  return true;
}

}  // namespace

void ConfigLoader::setSdAvailable(bool available) { sdAvailable_ = available; }

bool ConfigLoader::sdAvailable() const { return sdAvailable_; }

bool ConfigLoader::loadWifi(WifiSettings& settings) {
  if (!ensureSd(sdAvailable_, lastError_)) {
    return false;
  }

  File file = SD.open("/settings.json", FILE_READ);
  if (!file) {
    lastError_ = "settings.json missing";
    return false;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  if (error) {
    lastError_ = String("settings parse: ") + error.c_str();
    return false;
  }

  const char* ssid = doc["wifi"]["ssid"] | "";
  const char* password = doc["wifi"]["password"] | "";
  if (strlen(ssid) == 0 || strlen(password) == 0) {
    lastError_ = "Wi-Fi config missing";
    return false;
  }

  settings.ssid = ssid;
  settings.password = password;
  lastError_ = "";
  return true;
}

bool ConfigLoader::loadWebhooks(std::vector<WebhookCommand>& commands) {
  commands.clear();
  if (!ensureSd(sdAvailable_, lastError_)) {
    return false;
  }

  File file = SD.open("/apps/webhook_launcher.json", FILE_READ);
  if (!file) {
    lastError_ = "webhook config missing";
    return false;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  if (error) {
    lastError_ = String("webhook parse: ") + error.c_str();
    return false;
  }

  if ((doc["version"] | 0) != 1 || !doc["commands"].is<JsonArray>()) {
    lastError_ = "webhook config invalid";
    return false;
  }

  for (JsonObject item : doc["commands"].as<JsonArray>()) {
    WebhookCommand command;
    command.name = item["name"] | "";
    command.method = item["method"] | "";
    command.method.toUpperCase();
    command.url = item["url"] | "";
    command.confirm = item["confirm"] | false;

    if (command.name.isEmpty() || command.url.isEmpty() ||
        (command.method != "GET" && command.method != "POST")) {
      lastError_ = "webhook command invalid";
      commands.clear();
      return false;
    }

    if (item["headers"].is<JsonObject>()) {
      for (JsonPair kv : item["headers"].as<JsonObject>()) {
        Header header;
        header.name = kv.key().c_str();
        header.value = kv.value().as<const char*>() ? kv.value().as<const char*>() : "";
        command.headers.push_back(header);
      }
    }

    if (!item["body"].isNull()) {
      serializeJson(item["body"], command.bodyJson);
    }

    commands.push_back(command);
  }

  if (commands.empty()) {
    lastError_ = "no webhook commands";
    return false;
  }

  lastError_ = "";
  return true;
}

const String& ConfigLoader::lastError() const { return lastError_; }

}  // namespace cardputer_launcher

