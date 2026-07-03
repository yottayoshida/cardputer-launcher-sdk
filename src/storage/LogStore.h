#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <Arduino.h>
#include <vector>

#include "storage/RedactionRegistry.h"

namespace cardputer_launcher {

struct RequestLogEntry {
  RequestLogEntry() = default;
  RequestLogEntry(const String& commandName, const String& requestMethod,
                  const String& requestUrl, int responseStatusCode,
                  const String& requestOutcome, const String& responsePreview)
      : command(commandName),
        method(requestMethod),
        url(requestUrl),
        statusCode(responseStatusCode),
        outcome(requestOutcome),
        preview(responsePreview) {}

  String command;
  String method;
  String url;
  int statusCode = 0;
  String outcome;
  String preview;
};

class LogStore {
 public:
  void begin(bool sdAvailable);
  // `registry`, when non-null, is consulted first (exact match on the
  // secret values actually used in this command) before the coarse
  // keyword-based fallback below. Pass nullptr for log lines with no
  // known secret bindings.
  bool appendRequest(const RequestLogEntry& entry, const RedactionRegistry* registry = nullptr);
  std::vector<String> readRecent(size_t maxLines);
  String redact(const String& value, const RedactionRegistry* registry = nullptr) const;
  bool available() const;

 private:
  bool sdAvailable_ = false;
};

}  // namespace cardputer_launcher
