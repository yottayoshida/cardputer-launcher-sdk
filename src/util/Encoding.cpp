// SPDX-License-Identifier: MIT OR Apache-2.0

#include "util/Encoding.h"

#include <ArduinoJson.h>

namespace cardputer_launcher {

String percentEncode(const String& value) {
  String result;
  result.reserve(value.length());
  for (size_t i = 0; i < value.length(); ++i) {
    const unsigned char c = static_cast<unsigned char>(value[i]);
    const bool unreserved = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                             (c >= '0' && c <= '9') || c == '-' || c == '.' || c == '_' ||
                             c == '~';
    if (unreserved) {
      result += static_cast<char>(c);
      continue;
    }
    char buf[4];
    snprintf(buf, sizeof(buf), "%%%02X", c);
    result += buf;
  }
  return result;
}

String escapeJsonString(const String& value) {
  JsonDocument doc;
  doc.set(value);
  String out;
  serializeJson(doc, out);
  return out;
}

String escapeJsonStringInner(const String& value) {
  String quoted = escapeJsonString(value);
  if (quoted.length() < 2) {
    return quoted;
  }
  return quoted.substring(1, quoted.length() - 1);
}

}  // namespace cardputer_launcher
