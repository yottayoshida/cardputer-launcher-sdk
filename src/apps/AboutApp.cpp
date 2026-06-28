// SPDX-License-Identifier: MIT OR Apache-2.0

#include "apps/AboutApp.h"

#include "launcher/AppContext.h"

namespace cardputer_launcher {

const char* AboutApp::id() const { return "about"; }

const char* AboutApp::name() const { return "Settings/About"; }

void AboutApp::onInput(AppContext& ctx, const InputEvent& event) {
  (void)ctx;
  (void)event;
}

void AboutApp::render(AppContext& ctx) {
  ctx.display.clear();
  ctx.display.status("About", CARDPUTER_LAUNCHER_VERSION);
  ctx.display.line(0, "Cardputer Launcher SDK");
  ctx.display.line(1, "MIT OR Apache-2.0");
  ctx.display.line(2, CARDPUTER_LAUNCHER_REPO);
  ctx.display.line(4, "Back: delete");
}

}  // namespace cardputer_launcher

