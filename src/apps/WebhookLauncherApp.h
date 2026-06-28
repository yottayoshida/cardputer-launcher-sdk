#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <Arduino.h>
#include <vector>

#include "launcher/App.h"
#include "storage/ConfigLoader.h"
#include "ui/Menu.h"

namespace cardputer_launcher {

class WebhookLauncherApp : public App {
 public:
  const char* id() const override;
  const char* name() const override;
  void onStart(AppContext& ctx) override;
  void onInput(AppContext& ctx, const InputEvent& event) override;
  void render(AppContext& ctx) override;

 private:
  void execute(AppContext& ctx, const WebhookCommand& command);
  void rebuildMenu();

  std::vector<WebhookCommand> commands_;
  Menu menu_;
  bool configOk_ = false;
  bool awaitingConfirm_ = false;
  String status_;
  String preview_;
};

}  // namespace cardputer_launcher

