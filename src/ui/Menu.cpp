// SPDX-License-Identifier: MIT OR Apache-2.0

#include "ui/Menu.h"

namespace cardputer_launcher {

void Menu::clear() {
  items_.clear();
  selected_ = 0;
}

void Menu::addItem(const String& item) { items_.push_back(item); }

void Menu::next() {
  if (items_.empty()) {
    return;
  }
  selected_ = (selected_ + 1) % items_.size();
}

void Menu::previous() {
  if (items_.empty()) {
    return;
  }
  selected_ = selected_ == 0 ? items_.size() - 1 : selected_ - 1;
}

size_t Menu::selectedIndex() const { return selected_; }

size_t Menu::size() const { return items_.size(); }

void Menu::render(DisplayPort& display, const String& title, const String& statusText) const {
  display.status(title, statusText);
  if (items_.empty()) {
    display.line(0, "No items");
    return;
  }

  const size_t visibleRows = 6;
  size_t start = 0;
  if (selected_ >= visibleRows) {
    start = selected_ - visibleRows + 1;
  }

  for (size_t row = 0; row < visibleRows && start + row < items_.size(); ++row) {
    const size_t index = start + row;
    String prefix = index == selected_ ? "> " : "  ";
    display.line(static_cast<uint8_t>(row), prefix + items_[index]);
  }
}

}  // namespace cardputer_launcher

