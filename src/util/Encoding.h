#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <Arduino.h>

namespace cardputer_launcher {

// RFC3986 percent-encoding: unreserved characters pass through raw, every
// other byte becomes %XX. Shared by CommandTemplate (URL placeholder
// substitution) and RedactionRegistry (registering the encoded form a secret
// takes once substituted into a URL, so redaction still matches it).
String percentEncode(const String& value);

// Produces a quoted, escaped JSON string literal (including the quotes).
String escapeJsonString(const String& value);

// Same escaping as escapeJsonString, without the surrounding quotes. Used
// where the escaped form needs to be matched or inserted as a substring
// rather than a standalone JSON value.
String escapeJsonStringInner(const String& value);

}  // namespace cardputer_launcher
