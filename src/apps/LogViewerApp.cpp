// SPDX-License-Identifier: MIT OR Apache-2.0

#include "apps/LogViewerApp.h"

#include "launcher/AppContext.h"

namespace cardputer_launcher {

const char* LogViewerApp::id() const { return "logs"; }

const char* LogViewerApp::name() const { return "Log Viewer"; }

void LogViewerApp::onStart(AppContext& ctx) { lines_ = ctx.logs.readRecent(5); }

void LogViewerApp::onInput(AppContext& ctx, const InputEvent& event) {
  if (event.action == InputAction::Select) {
    lines_ = ctx.logs.readRecent(5);
    ctx.toast.show("Logs refreshed");
  }
}

void LogViewerApp::render(AppContext& ctx) {
  ctx.display.clear();
  ctx.display.status("Logs", ctx.logs.available() ? "SD:ok" : "SD:missing");
  for (size_t index = 0; index < lines_.size() && index < 5; ++index) {
    ctx.display.line(static_cast<uint8_t>(index), lines_[index]);
  }
  if (lines_.empty()) {
    ctx.display.line(0, "No logs yet");
  }
}

}  // namespace cardputer_launcher

