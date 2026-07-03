#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <ArduinoJson.h>

#include "storage/SecretProvider.h"

namespace cardputer_launcher {

// SD-card-backed SecretProvider: reads flat key/value pairs from
// /secrets.json. This is the only backend today; an NVS-backed provider is
// deferred until hardware verification of encrypted NVS is available (see
// SECURITY.md).
class SdSecretProvider : public SecretProvider {
 public:
  void begin(bool sdAvailable);
  bool resolve(const String& ref, String& value) override;
  // Equivalent to resolve() into a throwaway buffer: a ref that fails the
  // shared SecretProvider policy checks (too short, contains a control
  // character) is reported as not existing, since it can never be used.
  bool exists(const String& ref) override;
  const String& lastError() const override;

 private:
  // Parses /secrets.json into doc_ on first use and reuses it for every
  // later resolve()/exists() call in this session -- a command referencing
  // several secrets (headers, url, body, wifi.password) would otherwise
  // re-open and re-parse the same file once per reference.
  bool loadDocumentIfNeeded();

  bool sdAvailable_ = false;
  String lastError_;
  JsonDocument doc_;
  bool docLoaded_ = false;
};

}  // namespace cardputer_launcher
