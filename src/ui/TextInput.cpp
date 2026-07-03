// SPDX-License-Identifier: MIT OR Apache-2.0

#include "ui/TextInput.h"

namespace cardputer_launcher {

void TextInput::clear() {
  value_ = "";
  cursor_ = 0;
}

void TextInput::seed(const String& initial) {
  value_ = initial;
  if (value_.length() > maxLength_) {
    value_ = value_.substring(0, maxLength_);
  }
  cursor_ = value_.length();
}

void TextInput::setMaxLength(size_t maxLength) {
  maxLength_ = maxLength > 0 ? maxLength : 1;
  if (value_.length() > maxLength_) {
    value_ = value_.substring(0, maxLength_);
  }
  if (cursor_ > value_.length()) {
    cursor_ = value_.length();
  }
}

void TextInput::handle(const InputEvent& event) {
  switch (event.action) {
    case InputAction::Character:
      if (event.character < 32) {
        break;
      }
      if (value_.length() >= maxLength_) {
        break;
      }
      value_ = value_.substring(0, cursor_) + String(event.character) + value_.substring(cursor_);
      cursor_++;
      break;
    case InputAction::Back:
      if (cursor_ > 0) {
        value_ = value_.substring(0, cursor_ - 1) + value_.substring(cursor_);
        cursor_--;
      }
      break;
    case InputAction::Clear:
      value_ = "";
      cursor_ = 0;
      break;
    case InputAction::Left:
      if (cursor_ > 0) {
        cursor_--;
      }
      break;
    case InputAction::Right:
      if (cursor_ < value_.length()) {
        cursor_++;
      }
      break;
    default:
      break;
  }
}

const String& TextInput::value() const { return value_; }

size_t TextInput::cursor() const { return cursor_; }

String TextInput::renderWindow(size_t viewportChars) const {
  if (viewportChars == 0 || value_.length() <= viewportChars) {
    return value_;
  }

  size_t start = 0;
  if (cursor_ >= viewportChars) {
    start = cursor_ - viewportChars + 1;
  }
  const size_t maxStart = value_.length() - viewportChars;
  if (start > maxStart) {
    start = maxStart;
  }
  return value_.substring(start, start + viewportChars);
}

}  // namespace cardputer_launcher
