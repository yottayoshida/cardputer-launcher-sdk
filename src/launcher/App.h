#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include "input/Keyboard.h"
#include "launcher/AppManifest.h"

#ifndef CARDPUTER_LAUNCHER_VERSION
#define CARDPUTER_LAUNCHER_VERSION "0.0.0-dev"
#endif

namespace cardputer_launcher {

struct AppContext;

class App {
 public:
  virtual ~App() = default;

  virtual const char* id() const = 0;
  virtual const char* name() const = 0;
  virtual AppManifest manifest() const {
    return {id(), name(), CARDPUTER_LAUNCHER_VERSION, "system", "", 0, 0};
  }
  virtual void onStart(AppContext& ctx) { (void)ctx; }
  virtual void onFocus(AppContext& ctx) { (void)ctx; }
  virtual void onBlur(AppContext& ctx) { (void)ctx; }
  virtual void onTick(AppContext& ctx) { (void)ctx; }
  virtual void onSuspend(AppContext& ctx) { (void)ctx; }
  virtual void onResume(AppContext& ctx) { (void)ctx; }
  virtual void onStop(AppContext& ctx) { (void)ctx; }
  virtual void onInput(AppContext& ctx, const InputEvent& event) = 0;
  virtual void render(AppContext& ctx) = 0;

  // Navigation is the default; apps override to TextEntry only while a text
  // field currently owns keyboard focus (see Keyboard::poll(InputMode)).
  virtual InputMode inputMode(const AppContext& ctx) const {
    (void)ctx;
    return InputMode::Navigation;
  }

  // Whether the app wants Back routed to onInput() instead of Launcher
  // closing it. Distinct from inputMode(): a multi-step app (e.g. a field
  // wizard or a confirmation screen) can own Back to mean "go to the
  // previous step" without needing TextEntry's raw-character passthrough.
  virtual bool ownsBack(const AppContext& ctx) const {
    (void)ctx;
    return false;
  }
};

}  // namespace cardputer_launcher
