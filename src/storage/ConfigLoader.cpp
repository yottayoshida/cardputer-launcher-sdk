// SPDX-License-Identifier: MIT OR Apache-2.0

#include "storage/ConfigLoader.h"

#include <ArduinoJson.h>
#include <SD.h>

#include "storage/SdLayout.h"
#include "storage/SecretStore.h"

namespace cardputer_launcher {

namespace {

bool ensureSd(bool available, String& error) {
  if (!available) {
    error = "SD unavailable";
    return false;
  }
  return true;
}

bool readRequiredString(JsonVariantConst value, const String& path, String& out, String& error) {
  if (!value.is<const char*>()) {
    error = path + " must be a non-empty string";
    return false;
  }

  String raw = value.as<const char*>();
  String probe = raw;
  probe.trim();
  if (probe.isEmpty()) {
    error = path + " must be a non-empty string";
    return false;
  }
  out = raw;
  return true;
}

bool validateVersion(JsonVariantConst value, const String& path, String& error) {
  if (!value.is<int>() || value.as<int>() != 1) {
    error = path + " must be 1";
    return false;
  }
  return true;
}

bool resolveHeaderValue(JsonVariant value, SecretStore* secrets, String& resolved,
                        String& error) {
  resolved = "";

  const char* literal = value.as<const char*>();
  if (literal) {
    resolved = literal;
    return true;
  }

  if (!value.is<JsonObject>()) {
    error = "webhook header invalid";
    return false;
  }

  JsonObject object = value.as<JsonObject>();
  if (object.size() != 1) {
    error = "secretRef object invalid";
    return false;
  }
  const char* ref = object["secretRef"] | "";
  if (strlen(ref) == 0) {
    error = "secretRef missing";
    return false;
  }
  if (!secrets) {
    error = "secret store unavailable";
    return false;
  }
  if (!secrets->resolve(ref, resolved)) {
    error = secrets->lastError();
    return false;
  }
  return true;
}

bool hasMember(JsonObject object, const char* key) {
  for (JsonPair kv : object) {
    if (String(kv.key().c_str()) == key) {
      return true;
    }
  }
  return false;
}

bool isLoopbackHost(const String& host) {
  return host == "localhost" || host == "127.0.0.1" || host == "::1";
}

String stripPort(const String& hostPort) {
  if (hostPort.startsWith("[")) {
    int end = hostPort.indexOf(']');
    return end >= 0 ? hostPort.substring(1, end) : hostPort;
  }
  int colon = hostPort.indexOf(':');
  return colon >= 0 ? hostPort.substring(0, colon) : hostPort;
}

bool validateHttpsUrl(const String& url, const String& path, bool allowLocalHttp,
                       String& error) {
  bool isHttps = url.startsWith("https://");
  if (!isHttps && !(allowLocalHttp && url.startsWith("http://"))) {
    error = path + " must use https";
    return false;
  }

  String hostAndPath = url.substring(isHttps ? 8 : 7);
  int slash = hostAndPath.indexOf('/');
  int query = hostAndPath.indexOf('?');
  int fragment = hostAndPath.indexOf('#');
  int end = hostAndPath.length();
  if (slash >= 0 && slash < end) {
    end = slash;
  }
  if (query >= 0 && query < end) {
    end = query;
  }
  if (fragment >= 0 && fragment < end) {
    end = fragment;
  }
  String hostPort = hostAndPath.substring(0, end);
  hostPort.trim();
  if (hostPort.isEmpty()) {
    error = path + " must include a host";
    return false;
  }

  if (!isHttps && !isLoopbackHost(stripPort(hostPort))) {
    error = path + " local HTTP must use a loopback host";
    return false;
  }
  return true;
}

File openWebhookConfig() {
  File appPack = SD.open(kSdWebhookCommandsPath, FILE_READ);
  if (appPack) {
    return appPack;
  }
  return SD.open(kSdLegacyWebhookConfigPath, FILE_READ);
}

}  // namespace

void ConfigLoader::setSdAvailable(bool available) { sdAvailable_ = available; }

bool ConfigLoader::sdAvailable() const { return sdAvailable_; }

bool ConfigLoader::ensureLayout() { return ensureSdLayout(sdAvailable_, lastError_); }

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

  if (!validateVersion(doc["version"], "settings.version", lastError_)) {
    return false;
  }

  if (!doc["wifi"].is<JsonObject>()) {
    lastError_ = "settings.wifi must be an object";
    return false;
  }

  String ssid;
  if (!readRequiredString(doc["wifi"]["ssid"], "settings.wifi.ssid", ssid, lastError_)) {
    return false;
  }

  String password;
  if (!readRequiredString(doc["wifi"]["password"], "settings.wifi.password", password,
                          lastError_)) {
    return false;
  }

  settings.ssid = ssid;
  settings.password = password;
  lastError_ = "";
  return true;
}

bool ConfigLoader::loadWebhooks(std::vector<WebhookCommand>& commands,
                                SecretStore* secrets) {
  commands.clear();
  if (!ensureSd(sdAvailable_, lastError_)) {
    return false;
  }

  File file = openWebhookConfig();
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

  if (!validateVersion(doc["version"], "webhook config version", lastError_)) {
    return false;
  }

  if (!doc["commands"].is<JsonArray>()) {
    lastError_ = "webhook config commands must be a non-empty array";
    return false;
  }

  JsonArray items = doc["commands"].as<JsonArray>();
  if (items.size() == 0) {
    lastError_ = "webhook config commands must be a non-empty array";
    return false;
  }

  size_t index = 0;
  for (JsonVariant itemVariant : items) {
    String path = String("commands[") + index + "]";
    if (!itemVariant.is<JsonObject>()) {
      lastError_ = path + " must be an object";
      commands.clear();
      return false;
    }

    JsonObject item = itemVariant.as<JsonObject>();
    WebhookCommand command;
    if (!readRequiredString(item["name"], path + ".name", command.name, lastError_)) {
      commands.clear();
      return false;
    }

    if (!readRequiredString(item["method"], path + ".method", command.method, lastError_)) {
      commands.clear();
      return false;
    }
    command.method.toUpperCase();

    if (command.method != "GET" && command.method != "POST") {
      lastError_ = path + ".method must be GET or POST";
      commands.clear();
      return false;
    }

    if (!readRequiredString(item["url"], path + ".url", command.url, lastError_)) {
      commands.clear();
      return false;
    }

    if (hasMember(item, "allowLocalHttp")) {
      if (!item["allowLocalHttp"].is<bool>()) {
        lastError_ = path + ".allowLocalHttp must be true or false";
        commands.clear();
        return false;
      }
      command.allowLocalHttp = item["allowLocalHttp"].as<bool>();
    }

    if (!validateHttpsUrl(command.url, path + ".url", command.allowLocalHttp, lastError_)) {
      commands.clear();
      return false;
    }

    if (hasMember(item, "confirm")) {
      if (!item["confirm"].is<bool>()) {
        lastError_ = path + ".confirm must be true or false";
        commands.clear();
        return false;
      }
      command.confirm = item["confirm"].as<bool>();
    }

    if (!item["headers"].isNull()) {
      if (!item["headers"].is<JsonObject>()) {
        lastError_ = path + ".headers must be an object";
        commands.clear();
        return false;
      }
      for (JsonPair kv : item["headers"].as<JsonObject>()) {
        String key = kv.key().c_str();
        String keyProbe = key;
        keyProbe.trim();
        if (keyProbe.isEmpty()) {
          lastError_ = path + ".headers key must be a non-empty string";
          commands.clear();
          return false;
        }

        Header header;
        header.name = key;
        if (!resolveHeaderValue(kv.value(), secrets, header.value, lastError_)) {
          commands.clear();
          return false;
        }
        command.headers.push_back(header);
      }
    }

    if (!item["body"].isNull()) {
      if (command.method == "GET") {
        lastError_ = path + ".body is only supported for POST";
        commands.clear();
        return false;
      }
      serializeJson(item["body"], command.bodyJson);
    }

    commands.push_back(command);
    ++index;
  }

  lastError_ = "";
  return true;
}

const String& ConfigLoader::lastError() const { return lastError_; }

}  // namespace cardputer_launcher
