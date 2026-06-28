#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include "launcher/App.h"

namespace cardputer_launcher {

class AboutApp : public App {
 public:
  const char* id() const override;
  const char* name() const override;
  void onInput(AppContext& ctx, const InputEvent& event) override;
  void render(AppContext& ctx) override;
};

}  // namespace cardputer_launcher

