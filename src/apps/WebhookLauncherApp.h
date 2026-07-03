#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <Arduino.h>
#include <vector>

#include "launcher/App.h"
#include "network/CommandTemplate.h"
#include "storage/ConfigLoader.h"
#include "ui/Menu.h"
#include "ui/TextInput.h"

namespace cardputer_launcher {

class WebhookLauncherApp : public App {
 public:
  const char* id() const override;
  const char* name() const override;
  AppManifest manifest() const override;
  void onStart(AppContext& ctx) override;
  void onInput(AppContext& ctx, const InputEvent& event) override;
  void render(AppContext& ctx) override;
  InputMode inputMode(const AppContext& ctx) const override;
  bool ownsBack(const AppContext& ctx) const override;

 private:
  enum class Stage { List, CollectingInput, Preview, AwaitingConfirm };

  void execute(AppContext& ctx, const WebhookCommand& command, const RenderedRequest& rendered);
  void rebuildMenu();
  void startCommand(AppContext& ctx);
  void prepareCurrentField();
  bool commitCurrentField(AppContext& ctx);
  void advanceAfterField(AppContext& ctx);
  void finishInputCollection(AppContext& ctx);
  void abortCommand(const char* reason);
  void handleListInput(AppContext& ctx, const InputEvent& event);
  void handleFilterInput(AppContext& ctx, const InputEvent& event);
  void handleFieldInput(AppContext& ctx, const InputEvent& event);
  void handlePreviewInput(AppContext& ctx, const InputEvent& event);
  void handleConfirmInput(AppContext& ctx, const InputEvent& event);
  void renderFieldEditor(AppContext& ctx);
  void renderPreview(AppContext& ctx);

  std::vector<WebhookCommand> commands_;
  Menu menu_;
  bool configOk_ = false;
  String status_;
  String preview_;

  bool filtering_ = false;
  TextInput filterInput_;
  std::vector<size_t> visibleCommandIndices_;

  Stage stage_ = Stage::List;
  size_t activeCommandIndex_ = 0;
  size_t inputIndex_ = 0;
  std::vector<TemplateBinding> bindings_;
  TextInput fieldInput_;
  size_t choiceIndex_ = 0;
  bool boolValue_ = false;
  RenderedRequest pendingRequest_;
};

}  // namespace cardputer_launcher
