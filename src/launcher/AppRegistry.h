#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <stddef.h>

#include "launcher/App.h"

namespace cardputer_launcher {

class AppRegistry {
 public:
  static constexpr size_t kMaxApps = 8;

  bool add(App* app);
  App* at(size_t index) const;
  size_t size() const;

 private:
  App* apps_[kMaxApps] = {};
  size_t count_ = 0;
};

}  // namespace cardputer_launcher

