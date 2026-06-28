// SPDX-License-Identifier: MIT OR Apache-2.0

#include "ui/Toast.h"

namespace cardputer_launcher {

void Toast::show(const String& message, uint32_t ttlMs) {
  message_ = message;
  expiresAt_ = millis() + ttlMs;
  dirty_ = true;
}

void Toast::render(DisplayPort& display) {
  if (message_.isEmpty()) {
    return;
  }
  if (millis() > expiresAt_) {
    message_ = "";
    dirty_ = false;
    return;
  }
  display.line(6, message_);
  dirty_ = false;
}

bool Toast::needsRender() const { return dirty_ || (!message_.isEmpty() && millis() <= expiresAt_); }

}  // namespace cardputer_launcher

