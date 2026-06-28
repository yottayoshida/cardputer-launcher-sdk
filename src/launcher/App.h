#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include "input/Keyboard.h"

namespace cardputer_launcher {

struct AppContext;

class App {
 public:
  virtual ~App() = default;

  virtual const char* id() const = 0;
  virtual const char* name() const = 0;
  virtual void onStart(AppContext& ctx) { (void)ctx; }
  virtual void onStop(AppContext& ctx) { (void)ctx; }
  virtual void onInput(AppContext& ctx, const InputEvent& event) = 0;
  virtual void render(AppContext& ctx) = 0;
};

}  // namespace cardputer_launcher

