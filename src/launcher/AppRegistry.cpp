// SPDX-License-Identifier: MIT OR Apache-2.0

#include "launcher/AppRegistry.h"

namespace cardputer_launcher {

bool AppRegistry::add(App* app) {
  if (app == nullptr || count_ >= kMaxApps) {
    return false;
  }
  apps_[count_++] = app;
  return true;
}

App* AppRegistry::at(size_t index) const {
  if (index >= count_) {
    return nullptr;
  }
  return apps_[index];
}

size_t AppRegistry::size() const { return count_; }

}  // namespace cardputer_launcher

