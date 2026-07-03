// SPDX-License-Identifier: MIT OR Apache-2.0

#include "storage/SdSecretProvider.h"

#include <ArduinoJson.h>
#include <SD.h>

namespace cardputer_launcher {

namespace {

bool hasControlCharacter(const String& value) {
  for (size_t index = 0; index < value.length(); ++index) {
    const unsigned char c = static_cast<unsigned char>(value[index]);
    if (c < 0x20) {
      return true;
    }
  }
  return false;
}

}  // namespace

void SdSecretProvider::begin(bool sdAvailable) {
  sdAvailable_ = sdAvailable;
  docLoaded_ = false;
}

bool SdSecretProvider::loadDocumentIfNeeded() {
  if (docLoaded_) {
    return true;
  }

  File file = SD.open("/secrets.json", FILE_READ);
  if (!file) {
    lastError_ = "secrets.json missing";
    return false;
  }

  DeserializationError error = deserializeJson(doc_, file);
  file.close();
  if (error) {
    lastError_ = String("secrets parse: ") + error.c_str();
    return false;
  }

  docLoaded_ = true;
  return true;
}

bool SdSecretProvider::resolve(const String& ref, String& value) {
  value = "";
  if (!sdAvailable_) {
    lastError_ = "SD unavailable";
    return false;
  }
  if (!isValidSecretRefName(ref)) {
    lastError_ = "secret ref invalid";
    return false;
  }
  if (!loadDocumentIfNeeded()) {
    return false;
  }

  const char* secret = doc_[ref.c_str()] | "";
  const size_t length = strlen(secret);
  if (length == 0) {
    lastError_ = "secret ref missing";
    return false;
  }
  if (length < kMinSecretLength) {
    lastError_ = "secret ref '" + ref + "' is too short (must be at least " +
                 String(static_cast<unsigned>(kMinSecretLength)) + " bytes)";
    return false;
  }

  String candidate = secret;
  if (hasControlCharacter(candidate)) {
    lastError_ = "secret ref '" + ref + "' contains a control character";
    return false;
  }

  value = candidate;
  lastError_ = "";
  return true;
}

bool SdSecretProvider::exists(const String& ref) {
  String discard;
  return resolve(ref, discard);
}

const String& SdSecretProvider::lastError() const { return lastError_; }

}  // namespace cardputer_launcher
