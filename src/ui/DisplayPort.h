#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <Arduino.h>
#include <stdint.h>

namespace cardputer_launcher {

class DisplayPort {
 public:
  void begin();
  void clear();
  void line(uint8_t row, const String& text, uint16_t color = 0xFFFF);
  void status(const String& left, const String& right);
  void message(const String& title, const String& body);
  int width() const;
  int height() const;
};

}  // namespace cardputer_launcher

