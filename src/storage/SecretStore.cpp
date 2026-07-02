// SPDX-License-Identifier: MIT OR Apache-2.0

#include "storage/SecretStore.h"

#include <ArduinoJson.h>
#include <SD.h>

namespace cardputer_launcher {

namespace {

bool isValidSecretRef(const String& ref) {
  if (ref.isEmpty() || ref.length() > 96) {
    return false;
  }

  for (size_t index = 0; index < ref.length(); ++index) {
    const char c = ref[index];
    const bool valid = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                       (c >= '0' && c <= '9') || c == '_' || c == '-' ||
                       c == '.' || c == ':';
    if (!valid) {
      return false;
    }
  }
  return true;
}

}  // namespace

void SecretStore::begin(bool sdAvailable) { sdAvailable_ = sdAvailable; }

bool SecretStore::resolve(const String& ref, String& value) {
  value = "";
  if (!sdAvailable_) {
    lastError_ = "SD unavailable";
    return false;
  }
  if (!isValidSecretRef(ref)) {
    lastError_ = "secret ref invalid";
    return false;
  }

  File file = SD.open("/secrets.json", FILE_READ);
  if (!file) {
    lastError_ = "secrets.json missing";
    return false;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  if (error) {
    lastError_ = String("secrets parse: ") + error.c_str();
    return false;
  }

  const char* secret = doc[ref.c_str()] | "";
  if (strlen(secret) == 0) {
    lastError_ = "secret ref missing";
    return false;
  }

  value = secret;
  lastError_ = "";
  return true;
}

const String& SecretStore::lastError() const { return lastError_; }

}  // namespace cardputer_launcher
