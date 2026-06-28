// SPDX-License-Identifier: MIT OR Apache-2.0

#include "ui/DisplayPort.h"

#include <M5Cardputer.h>

namespace cardputer_launcher {

void DisplayPort::begin() {
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setTextColor(WHITE, BLACK);
  clear();
}

void DisplayPort::clear() { M5Cardputer.Display.fillScreen(BLACK); }

void DisplayPort::line(uint8_t row, const String& text, uint16_t color) {
  const int y = 18 + static_cast<int>(row) * 16;
  M5Cardputer.Display.fillRect(0, y, width(), 16, BLACK);
  M5Cardputer.Display.setTextColor(color, BLACK);
  M5Cardputer.Display.drawString(text, 4, y);
}

void DisplayPort::status(const String& left, const String& right) {
  M5Cardputer.Display.fillRect(0, 0, width(), 16, DARKGREY);
  M5Cardputer.Display.setTextColor(WHITE, DARKGREY);
  M5Cardputer.Display.drawString(left, 4, 2);
  int rightX = width() - (right.length() * 6) - 4;
  if (rightX < 100) {
    rightX = 100;
  }
  M5Cardputer.Display.drawString(right, rightX, 2);
}

void DisplayPort::message(const String& title, const String& body) {
  clear();
  status(title, "");
  line(1, body);
}

int DisplayPort::width() const { return M5Cardputer.Display.width(); }

int DisplayPort::height() const { return M5Cardputer.Display.height(); }

}  // namespace cardputer_launcher

