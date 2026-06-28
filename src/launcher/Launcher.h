#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <stddef.h>

#include "launcher/AppRegistry.h"
#include "ui/Menu.h"

namespace cardputer_launcher {

class Launcher {
 public:
  explicit Launcher(AppRegistry& registry);

  void begin(AppContext& ctx);
  void handleInput(AppContext& ctx, const InputEvent& event);
  void render(AppContext& ctx);

 private:
  void openSelected(AppContext& ctx);

  AppRegistry& registry_;
  Menu menu_;
  App* activeApp_ = nullptr;
  bool dirty_ = true;
};

}  // namespace cardputer_launcher

