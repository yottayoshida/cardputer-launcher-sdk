#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <Arduino.h>
#include <vector>

#include "launcher/App.h"

namespace cardputer_launcher {

class LogViewerApp : public App {
 public:
  const char* id() const override;
  const char* name() const override;
  AppManifest manifest() const override;
  void onStart(AppContext& ctx) override;
  void onInput(AppContext& ctx, const InputEvent& event) override;
  void render(AppContext& ctx) override;

 private:
  std::vector<String> lines_;
};

}  // namespace cardputer_launcher
