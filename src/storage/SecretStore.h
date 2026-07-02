#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <Arduino.h>

namespace cardputer_launcher {

class SecretStore {
 public:
  void begin(bool sdAvailable);
  bool resolve(const String& ref, String& value);
  const String& lastError() const;

 private:
  bool sdAvailable_ = false;
  String lastError_;
};

}  // namespace cardputer_launcher
