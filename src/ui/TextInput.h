#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <Arduino.h>

#include "input/Keyboard.h"

namespace cardputer_launcher {

class TextInput {
 public:
  void clear();
  void seed(const String& initial);
  void setMaxLength(size_t maxLength);
  void handle(const InputEvent& event);
  const String& value() const;
  size_t cursor() const;

  // Returns at most `viewportChars` characters of value(), scrolled so the
  // cursor stays visible. Used to fit long values on the small display.
  String renderWindow(size_t viewportChars) const;

 private:
  String value_;
  size_t cursor_ = 0;
  size_t maxLength_ = 96;
};

}  // namespace cardputer_launcher
