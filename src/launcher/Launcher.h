#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <stddef.h>
#include <vector>

#include "launcher/AppRegistry.h"
#include "ui/Menu.h"
#include "ui/TextInput.h"

namespace cardputer_launcher {

class Launcher {
 public:
  explicit Launcher(AppRegistry& registry);

  void begin(AppContext& ctx);
  void handleInput(AppContext& ctx, const InputEvent& event);
  void tick(AppContext& ctx);
  void render(AppContext& ctx);

  // What Keyboard::poll() should use on the next frame: TextEntry while the
  // launcher's own app-search field owns focus, otherwise delegates to the
  // active app (apps own TextEntry state such as command search or typed
  // command inputs).
  InputMode currentInputMode(const AppContext& ctx) const;

 private:
  void openSelected(AppContext& ctx);
  void rebuildMenu();
  void startSearch();
  void stopSearch();
  void cancelSearch();

  AppRegistry& registry_;
  Menu menu_;
  App* activeApp_ = nullptr;
  bool dirty_ = true;

  bool searching_ = false;
  TextInput searchInput_;
  std::vector<size_t> visibleAppIndices_;
};

}  // namespace cardputer_launcher
