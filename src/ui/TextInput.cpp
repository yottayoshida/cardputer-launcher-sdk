// SPDX-License-Identifier: MIT OR Apache-2.0

#include "ui/TextInput.h"

namespace cardputer_launcher {

void TextInput::clear() { value_ = ""; }

void TextInput::handle(const InputEvent& event) {
  if (event.action == InputAction::Character && event.character >= 32) {
    value_ += event.character;
  } else if (event.action == InputAction::Back && !value_.isEmpty()) {
    value_.remove(value_.length() - 1);
  }
}

const String& TextInput::value() const { return value_; }

}  // namespace cardputer_launcher

