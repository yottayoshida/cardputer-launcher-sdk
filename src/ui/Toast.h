#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <Arduino.h>

#include "ui/DisplayPort.h"

namespace cardputer_launcher {

class Toast {
 public:
  void show(const String& message, uint32_t ttlMs = 2500);
  void render(DisplayPort& display);
  bool needsRender() const;

 private:
  String message_;
  uint32_t expiresAt_ = 0;
  bool dirty_ = false;
};

}  // namespace cardputer_launcher

