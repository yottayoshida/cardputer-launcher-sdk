// SPDX-License-Identifier: MIT OR Apache-2.0

#include "launcher/Launcher.h"

#include "launcher/AppContext.h"

namespace cardputer_launcher {

Launcher::Launcher(AppRegistry& registry) : registry_(registry) {}

void Launcher::begin(AppContext& ctx) {
  menu_.clear();
  for (size_t index = 0; index < registry_.size(); ++index) {
    App* app = registry_.at(index);
    if (app != nullptr) {
      menu_.addItem(app->name());
    }
  }
  ctx.toast.show("Ready");
  dirty_ = true;
}

void Launcher::handleInput(AppContext& ctx, const InputEvent& event) {
  if (event.action == InputAction::None) {
    return;
  }

  if (activeApp_ != nullptr) {
    if (event.action == InputAction::Back) {
      activeApp_->onStop(ctx);
      activeApp_ = nullptr;
      dirty_ = true;
      return;
    }
    activeApp_->onInput(ctx, event);
    dirty_ = true;
    return;
  }

  if (event.action == InputAction::Up) {
    menu_.previous();
    dirty_ = true;
  } else if (event.action == InputAction::Down) {
    menu_.next();
    dirty_ = true;
  } else if (event.action == InputAction::Select) {
    openSelected(ctx);
  }
}

void Launcher::render(AppContext& ctx) {
  if (!dirty_ && !ctx.toast.needsRender()) {
    return;
  }

  if (activeApp_ != nullptr) {
    activeApp_->render(ctx);
  } else {
    ctx.display.clear();
    menu_.render(ctx.display, "Launcher", ctx.sdAvailable ? "SD:ok" : "SD:missing");
  }
  ctx.toast.render(ctx.display);
  dirty_ = false;
}

void Launcher::openSelected(AppContext& ctx) {
  App* app = registry_.at(menu_.selectedIndex());
  if (app == nullptr) {
    ctx.toast.show("No app");
    return;
  }
  activeApp_ = app;
  activeApp_->onStart(ctx);
  dirty_ = true;
}

}  // namespace cardputer_launcher

