// SPDX-License-Identifier: MIT OR Apache-2.0

#include "storage/LogStore.h"

#include <ArduinoJson.h>
#include <SD.h>

namespace cardputer_launcher {

namespace {

const char* kLogPath = "/logs/launcher.jsonl";

bool hasSecretWord(const String& lowered) {
  return lowered.indexOf("authorization") >= 0 || lowered.indexOf("bearer ") >= 0 ||
         lowered.indexOf("token") >= 0 || lowered.indexOf("secret") >= 0 ||
         lowered.indexOf("password") >= 0 || lowered.indexOf("api_key") >= 0 ||
         lowered.indexOf("apikey") >= 0;
}

String limitPreview(const String& value) {
  if (value.length() <= 120) {
    return value;
  }
  return value.substring(0, 120) + "...";
}

}  // namespace

void LogStore::begin(bool sdAvailable) {
  sdAvailable_ = sdAvailable;
  if (sdAvailable_) {
    SD.mkdir("/logs");
  }
}

bool LogStore::appendRequest(const RequestLogEntry& entry) {
  if (!sdAvailable_) {
    return false;
  }

  File file = SD.open(kLogPath, FILE_APPEND);
  if (!file) {
    return false;
  }

  JsonDocument doc;
  doc["ts_ms"] = millis();
  doc["command"] = entry.command;
  doc["method"] = entry.method;
  doc["url"] = redact(entry.url);
  doc["status"] = entry.statusCode;
  doc["outcome"] = entry.outcome;
  doc["preview"] = redact(limitPreview(entry.preview));
  serializeJson(doc, file);
  file.println();
  file.close();
  return true;
}

std::vector<String> LogStore::readRecent(size_t maxLines) {
  std::vector<String> lines;
  if (!sdAvailable_) {
    lines.push_back("SD unavailable");
    return lines;
  }

  File file = SD.open(kLogPath, FILE_READ);
  if (!file) {
    lines.push_back("No logs yet");
    return lines;
  }

  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (!line.isEmpty()) {
      lines.push_back(limitPreview(redact(line)));
      if (lines.size() > maxLines) {
        lines.erase(lines.begin());
      }
    }
  }
  file.close();
  return lines;
}

String LogStore::redact(const String& value) const {
  String lowered = value;
  lowered.toLowerCase();
  if (!hasSecretWord(lowered)) {
    return value;
  }
  return "[REDACTED]";
}

bool LogStore::available() const { return sdAvailable_; }

}  // namespace cardputer_launcher

