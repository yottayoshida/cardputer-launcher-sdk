#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <Arduino.h>

namespace cardputer_launcher {

// Abstract secret-resolution backend. Every implementation MUST enforce the
// same contract regardless of storage medium, so callers (header resolution,
// URL/body templating, Wi-Fi settings) get uniform safety without repeating
// checks at each call site:
//   - resolve() only succeeds for values at least kMinSecretLength bytes long
//     (a shorter value cannot be redacted reliably, so it is rejected instead
//     of silently accepted and later leaked).
//   - resolve() only succeeds for values containing no control characters
//     (CR, LF, and other bytes below 0x20), so a resolved secret can never be
//     used to inject a header line or corrupt a log line.
//   - lastError() must never include the raw secret value or a fully
//     resolved URL/body, so a caller can safely surface it to the UI or logs.
class SecretProvider {
 public:
  static constexpr size_t kMinSecretLength = 6;

  virtual ~SecretProvider() = default;

  virtual bool resolve(const String& ref, String& value) = 0;
  virtual bool exists(const String& ref) = 0;
  virtual const String& lastError() const = 0;
};

// Ref-name syntax shared by every call site that mentions a secret ref:
// config-load-time placeholder validation (`{{secret.<ref>}}`, the header
// `{"secretRef": "<ref>"}` object) and provider lookups. Letters, digits,
// dot/underscore/hyphen/colon, 1-96 bytes.
bool isValidSecretRefName(const String& ref);

}  // namespace cardputer_launcher
