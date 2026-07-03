// SPDX-License-Identifier: MIT OR Apache-2.0

#include "storage/LogStore.h"

#include <ArduinoJson.h>
#include <SD.h>

#include "storage/SdLayout.h"

namespace cardputer_launcher {

namespace {

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
    SD.mkdir(kSdLogDir);
  }
}

bool LogStore::appendRequest(const RequestLogEntry& entry, const RedactionRegistry* registry) {
  if (!sdAvailable_) {
    return false;
  }

  File file = SD.open(kSdLauncherLogPath, FILE_APPEND);
  if (!file) {
    return false;
  }

  JsonDocument doc;
  doc["ts_ms"] = millis();
  doc["command"] = entry.command;
  doc["method"] = entry.method;
  doc["url"] = redact(entry.url, registry);
  doc["status"] = entry.statusCode;
  // No known code path puts a resolved secret in outcome today (it's always
  // a fixed status string or a transport-layer error), but it's cheap
  // defense in depth against a future error source that echoes one.
  doc["outcome"] = redact(entry.outcome, registry);
  doc["preview"] = redact(limitPreview(entry.preview), registry);
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

  File file = SD.open(kSdLauncherLogPath, FILE_READ);
  if (!file) {
    lines.push_back("No logs yet");
    return lines;
  }

  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (!line.isEmpty()) {
      // Historical lines were already redacted at write time; this second
      // pass only re-applies the keyword fallback (no registry, since the
      // secrets used by a past command are no longer in memory).
      lines.push_back(limitPreview(redact(line)));
      if (lines.size() > maxLines) {
        lines.erase(lines.begin());
      }
    }
  }
  file.close();
  return lines;
}

String LogStore::redact(const String& value, const RedactionRegistry* registry) const {
  const String stageOne = registry ? registry->redact(value) : value;
  String lowered = stageOne;
  lowered.toLowerCase();
  if (!hasSecretWord(lowered)) {
    return stageOne;
  }
  return "[REDACTED]";
}

bool LogStore::available() const { return sdAvailable_; }

}  // namespace cardputer_launcher
