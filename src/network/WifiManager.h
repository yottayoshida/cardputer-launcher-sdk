#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <Arduino.h>

#include "storage/ConfigLoader.h"

namespace cardputer_launcher {

class WifiManager {
 public:
  bool connect(const WifiSettings& settings, uint32_t timeoutMs = 15000);
  bool isConnected() const;
  const String& lastError() const;

 private:
  String lastError_;
};

}  // namespace cardputer_launcher

