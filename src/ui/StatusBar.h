#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <Arduino.h>

#include "ui/DisplayPort.h"

namespace cardputer_launcher {

class StatusBar {
 public:
  void render(DisplayPort& display, const String& left, const String& right) {
    display.status(left, right);
  }
};

}  // namespace cardputer_launcher

