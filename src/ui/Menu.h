#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <Arduino.h>
#include <vector>

#include "ui/DisplayPort.h"

namespace cardputer_launcher {

class Menu {
 public:
  void clear();
  void addItem(const String& item);
  void next();
  void previous();
  size_t selectedIndex() const;
  size_t size() const;
  void render(DisplayPort& display, const String& title, const String& status) const;

 private:
  std::vector<String> items_;
  size_t selected_ = 0;
};

}  // namespace cardputer_launcher

