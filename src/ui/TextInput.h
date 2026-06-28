#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <Arduino.h>

#include "input/Keyboard.h"

namespace cardputer_launcher {

class TextInput {
 public:
  void clear();
  void handle(const InputEvent& event);
  const String& value() const;

 private:
  String value_;
};

}  // namespace cardputer_launcher

