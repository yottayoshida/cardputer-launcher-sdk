// SPDX-License-Identifier: MIT OR Apache-2.0

#include "launcher/Launcher.h"

#include "launcher/AppContext.h"
#include "ui/IncrementalFilter.h"

namespace cardputer_launcher {

Launcher::Launcher(AppRegistry& registry) : registry_(registry) {}

void Launcher::begin(AppContext& ctx) {
  searching_ = false;
  searchInput_.clear();
  rebuildMenu();
  ctx.toast.show("Ready");
  dirty_ = true;
}

void Launcher::rebuildMenu() {
  std::vector<String> labels;
  for (size_t index = 0; index < registry_.size(); ++index) {
    App* app = registry_.at(index);
    if (app != nullptr) {
      labels.push_back(app->name());
    }
  }

  visibleAppIndices_ = filterIndices(labels, searchInput_.value());

  menu_.clear();
  for (size_t index : visibleAppIndices_) {
    menu_.addItem(labels[index]);
  }
}

void Launcher::startSearch() {
  searching_ = true;
  dirty_ = true;
}

void Launcher::stopSearch() {
  // Tab exits text entry but keeps the current filter applied.
  searching_ = false;
  dirty_ = true;
}

void Launcher::cancelSearch() {
  // Cancel (or Back on an empty query) clears the filter entirely.
  searching_ = false;
  searchInput_.clear();
  rebuildMenu();
  dirty_ = true;
}

InputMode Launcher::currentInputMode(const AppContext& ctx) const {
  if (searching_) {
    return InputMode::TextEntry;
  }
  if (activeApp_ != nullptr) {
    return activeApp_->inputMode(ctx);
  }
  return InputMode::Navigation;
}

void Launcher::handleInput(AppContext& ctx, const InputEvent& event) {
  if (event.action == InputAction::None) {
    return;
  }

  if (activeApp_ != nullptr) {
    const bool appOwnsBack =
        activeApp_->ownsBack(ctx) || activeApp_->inputMode(ctx) == InputMode::TextEntry;
    if (event.action == InputAction::Back && !appOwnsBack) {
      activeApp_->onBlur(ctx);
      activeApp_->onStop(ctx);
      activeApp_ = nullptr;
      dirty_ = true;
      return;
    }
    activeApp_->onInput(ctx, event);
    dirty_ = true;
    return;
  }

  if (searching_) {
    switch (event.action) {
      case InputAction::ToggleSearch:
        stopSearch();
        return;
      case InputAction::Cancel:
        cancelSearch();
        return;
      case InputAction::Select:
        openSelected(ctx);
        return;
      case InputAction::Up:
        menu_.previous();
        dirty_ = true;
        return;
      case InputAction::Down:
        menu_.next();
        dirty_ = true;
        return;
      case InputAction::Back:
        if (searchInput_.value().isEmpty()) {
          cancelSearch();
          return;
        }
        break;
      default:
        break;
    }
    searchInput_.handle(event);
    rebuildMenu();
    dirty_ = true;
    return;
  }

  if (event.action == InputAction::ToggleSearch) {
    startSearch();
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

void Launcher::tick(AppContext& ctx) {
  if (activeApp_ != nullptr) {
    activeApp_->onTick(ctx);
    dirty_ = true;
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
    String title = searching_ ? ("Search: " + searchInput_.renderWindow(20)) : "Launcher";
    menu_.render(ctx.display, title, ctx.sdAvailable ? "SD:ok" : "SD:missing");
  }
  ctx.toast.render(ctx.display);
  dirty_ = false;
}

void Launcher::openSelected(AppContext& ctx) {
  if (visibleAppIndices_.empty() || menu_.selectedIndex() >= visibleAppIndices_.size()) {
    ctx.toast.show("No app");
    return;
  }

  App* app = registry_.at(visibleAppIndices_[menu_.selectedIndex()]);
  if (app == nullptr) {
    ctx.toast.show("No app");
    return;
  }
  activeApp_ = app;
  activeApp_->onStart(ctx);
  activeApp_->onFocus(ctx);
  if (searching_) {
    cancelSearch();
  }
  dirty_ = true;
}

}  // namespace cardputer_launcher
