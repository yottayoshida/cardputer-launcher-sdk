// SPDX-License-Identifier: MIT OR Apache-2.0

#include "apps/WebhookLauncherApp.h"

#include "launcher/AppContext.h"
#include "network/HttpClient.h"
#include "storage/LogStore.h"
#include "ui/IncrementalFilter.h"

namespace cardputer_launcher {

namespace {

// The display fits roughly 38 characters per line at the default text size.
// A resolved URL is most often verified by its tail (the substituted typed
// value), so a long URL is truncated from the front rather than the back.
constexpr size_t kPreviewLineChars = 38;

String previewUrlLine(const String& url) {
  if (url.length() <= kPreviewLineChars) {
    return url;
  }
  return "..." + url.substring(url.length() - (kPreviewLineChars - 3));
}

}  // namespace

const char* WebhookLauncherApp::id() const { return "webhook_launcher"; }

const char* WebhookLauncherApp::name() const { return "Webhook Launcher"; }

AppManifest WebhookLauncherApp::manifest() const {
  return {
      id(),
      name(),
      CARDPUTER_LAUNCHER_VERSION,
      "automation",
      "/apps/webhook_launcher.json",
      kPermissionStorageRead | kPermissionStorageWrite | kPermissionNetworkHttp |
          kPermissionInputKeyboard | kPermissionDisplayDraw,
      kCapabilityCommandList | kCapabilityCommandExecute | kCapabilityConfigReload,
  };
}

void WebhookLauncherApp::onStart(AppContext& ctx) {
  commands_.clear();
  stage_ = Stage::List;
  filtering_ = false;
  filterInput_.clear();
  preview_ = "";
  configOk_ = ctx.config.loadWebhooks(commands_, &ctx.secrets);
  if (!configOk_) {
    status_ = ctx.config.lastError();
    return;
  }
  rebuildMenu();
  status_ = String(commands_.size()) + " command(s)";
}

InputMode WebhookLauncherApp::inputMode(const AppContext&) const {
  if (filtering_) {
    return InputMode::TextEntry;
  }
  if (stage_ == Stage::CollectingInput && activeCommandIndex_ < commands_.size()) {
    const WebhookCommand& command = commands_[activeCommandIndex_];
    if (inputIndex_ < command.inputs.size() &&
        command.inputs[inputIndex_].kind == InputField::Kind::ShortText) {
      return InputMode::TextEntry;
    }
  }
  return InputMode::Navigation;
}

bool WebhookLauncherApp::ownsBack(const AppContext&) const { return stage_ != Stage::List; }

void WebhookLauncherApp::rebuildMenu() {
  std::vector<String> labels;
  for (const WebhookCommand& command : commands_) {
    labels.push_back(command.name);
  }
  visibleCommandIndices_ = filterIndices(labels, filterInput_.value());

  menu_.clear();
  for (size_t index : visibleCommandIndices_) {
    menu_.addItem(labels[index]);
  }
}

void WebhookLauncherApp::onInput(AppContext& ctx, const InputEvent& event) {
  if (!configOk_ || commands_.empty()) {
    if (event.action == InputAction::Select) {
      onStart(ctx);
    }
    return;
  }

  switch (stage_) {
    case Stage::CollectingInput:
      handleFieldInput(ctx, event);
      return;
    case Stage::Preview:
      handlePreviewInput(ctx, event);
      return;
    case Stage::AwaitingConfirm:
      handleConfirmInput(ctx, event);
      return;
    case Stage::List:
      break;
  }

  if (filtering_) {
    handleFilterInput(ctx, event);
    return;
  }

  handleListInput(ctx, event);
}

void WebhookLauncherApp::handleListInput(AppContext& ctx, const InputEvent& event) {
  if (event.action == InputAction::ToggleSearch) {
    filtering_ = true;
    return;
  }
  if (event.action == InputAction::Up) {
    menu_.previous();
  } else if (event.action == InputAction::Down) {
    menu_.next();
  } else if (event.action == InputAction::Select) {
    startCommand(ctx);
  }
}

void WebhookLauncherApp::handleFilterInput(AppContext& ctx, const InputEvent& event) {
  switch (event.action) {
    case InputAction::ToggleSearch:
      filtering_ = false;  // Tab keeps the current filter applied.
      return;
    case InputAction::Cancel:
      filtering_ = false;
      filterInput_.clear();
      rebuildMenu();
      return;
    case InputAction::Select:
      startCommand(ctx);
      return;
    case InputAction::Up:
      menu_.previous();
      return;
    case InputAction::Down:
      menu_.next();
      return;
    case InputAction::Back:
      if (filterInput_.value().isEmpty()) {
        filtering_ = false;
        rebuildMenu();
        return;
      }
      break;
    default:
      break;
  }
  filterInput_.handle(event);
  rebuildMenu();
}

void WebhookLauncherApp::startCommand(AppContext& ctx) {
  if (visibleCommandIndices_.empty() || menu_.selectedIndex() >= visibleCommandIndices_.size()) {
    ctx.toast.show("No command");
    return;
  }
  activeCommandIndex_ = visibleCommandIndices_[menu_.selectedIndex()];
  filtering_ = false;
  bindings_.clear();
  inputIndex_ = 0;

  const WebhookCommand& command = commands_[activeCommandIndex_];
  if (command.inputs.empty()) {
    finishInputCollection(ctx);
    return;
  }
  stage_ = Stage::CollectingInput;
  prepareCurrentField();
}

void WebhookLauncherApp::prepareCurrentField() {
  const InputField& field = commands_[activeCommandIndex_].inputs[inputIndex_];
  switch (field.kind) {
    case InputField::Kind::ShortText:
      fieldInput_.clear();
      fieldInput_.setMaxLength(field.maxLength);
      if (!field.defaultValue.isEmpty()) {
        fieldInput_.seed(field.defaultValue);
      }
      break;
    case InputField::Kind::Choice:
      choiceIndex_ = 0;
      for (size_t i = 0; i < field.choices.size(); ++i) {
        if (field.choices[i] == field.defaultValue) {
          choiceIndex_ = i;
          break;
        }
      }
      break;
    case InputField::Kind::Boolean:
      boolValue_ = field.defaultValue == "true";
      break;
    case InputField::Kind::Confirmation:
      break;
  }
}

void WebhookLauncherApp::abortCommand(const char* reason) {
  stage_ = Stage::List;
  status_ = reason;
}

void WebhookLauncherApp::handleFieldInput(AppContext& ctx, const InputEvent& event) {
  const InputField& field = commands_[activeCommandIndex_].inputs[inputIndex_];

  // Cancel ('n' in Navigation mode) always means "abort this command" so its
  // meaning never flips between field kinds. Boolean values are set only via
  // the arrow toggle below, not via Confirm/Cancel.
  if (event.action == InputAction::Cancel) {
    abortCommand("Canceled");
    return;
  }

  if (event.action == InputAction::Back) {
    const bool textFieldHasContent =
        field.kind == InputField::Kind::ShortText && !fieldInput_.value().isEmpty();
    if (textFieldHasContent) {
      fieldInput_.handle(event);
      return;
    }
    if (inputIndex_ == 0) {
      abortCommand("Canceled");
      return;
    }
    --inputIndex_;
    if (!bindings_.empty()) {
      bindings_.pop_back();
    }
    prepareCurrentField();
    return;
  }

  switch (field.kind) {
    case InputField::Kind::ShortText:
      if (event.action == InputAction::Select) {
        if (commitCurrentField(ctx)) {
          advanceAfterField(ctx);
        }
        return;
      }
      fieldInput_.handle(event);
      return;
    case InputField::Kind::Choice:
      if (event.action == InputAction::Select) {
        if (commitCurrentField(ctx)) {
          advanceAfterField(ctx);
        }
        return;
      }
      if (!field.choices.empty()) {
        if (event.action == InputAction::Left || event.action == InputAction::Up) {
          choiceIndex_ = choiceIndex_ == 0 ? field.choices.size() - 1 : choiceIndex_ - 1;
        } else if (event.action == InputAction::Right || event.action == InputAction::Down) {
          choiceIndex_ = (choiceIndex_ + 1) % field.choices.size();
        }
      }
      return;
    case InputField::Kind::Boolean:
      if (event.action == InputAction::Select) {
        if (commitCurrentField(ctx)) {
          advanceAfterField(ctx);
        }
        return;
      }
      if (event.action == InputAction::Left || event.action == InputAction::Right ||
          event.action == InputAction::Up || event.action == InputAction::Down) {
        boolValue_ = !boolValue_;
      }
      return;
    case InputField::Kind::Confirmation:
      if (event.action == InputAction::Select || event.action == InputAction::Confirm) {
        if (commitCurrentField(ctx)) {
          advanceAfterField(ctx);
        }
      }
      return;
  }
}

bool WebhookLauncherApp::commitCurrentField(AppContext& ctx) {
  const InputField& field = commands_[activeCommandIndex_].inputs[inputIndex_];
  TemplateBinding binding;
  binding.key = field.key;

  switch (field.kind) {
    case InputField::Kind::ShortText: {
      String value = fieldInput_.value();
      if (value.isEmpty() && field.required) {
        ctx.toast.show("Value required");
        return false;
      }
      binding.value = value;
      break;
    }
    case InputField::Kind::Choice:
      if (field.choices.empty() || choiceIndex_ >= field.choices.size()) {
        return false;
      }
      binding.value = field.choices[choiceIndex_];
      break;
    case InputField::Kind::Boolean:
      binding.value = boolValue_ ? "true" : "false";
      break;
    case InputField::Kind::Confirmation:
      binding.value = "true";
      break;
  }

  bindings_.push_back(binding);
  return true;
}

void WebhookLauncherApp::advanceAfterField(AppContext& ctx) {
  const WebhookCommand& command = commands_[activeCommandIndex_];
  ++inputIndex_;
  if (inputIndex_ < command.inputs.size()) {
    prepareCurrentField();
    return;
  }
  finishInputCollection(ctx);
}

void WebhookLauncherApp::finishInputCollection(AppContext& ctx) {
  const WebhookCommand& command = commands_[activeCommandIndex_];
  pendingRequest_ = renderCommandTemplate(command, bindings_);
  if (!pendingRequest_.ok) {
    status_ = pendingRequest_.error;
    ctx.toast.show(status_);
    stage_ = Stage::List;
    return;
  }

  if (command.requirePreview) {
    stage_ = Stage::Preview;
  } else if (command.confirm) {
    stage_ = Stage::AwaitingConfirm;
  } else {
    stage_ = Stage::List;
    execute(ctx, command, pendingRequest_);
  }
}

void WebhookLauncherApp::handlePreviewInput(AppContext& ctx, const InputEvent& event) {
  const WebhookCommand& command = commands_[activeCommandIndex_];
  if (event.action == InputAction::Select || event.action == InputAction::Confirm) {
    if (command.confirm) {
      stage_ = Stage::AwaitingConfirm;
    } else {
      stage_ = Stage::List;
      execute(ctx, command, pendingRequest_);
    }
  } else if (event.action == InputAction::Cancel || event.action == InputAction::Back) {
    abortCommand("Canceled");
  }
}

void WebhookLauncherApp::handleConfirmInput(AppContext& ctx, const InputEvent& event) {
  const WebhookCommand& command = commands_[activeCommandIndex_];
  if (event.action == InputAction::Confirm || event.action == InputAction::Select) {
    stage_ = Stage::List;
    execute(ctx, command, pendingRequest_);
  } else if (event.action == InputAction::Cancel || event.action == InputAction::Back) {
    abortCommand("Canceled");
  }
}

void WebhookLauncherApp::render(AppContext& ctx) {
  ctx.display.clear();
  if (!configOk_) {
    ctx.display.status("Webhooks", "Config error");
    ctx.display.line(0, status_);
    ctx.display.line(2, "Fix SD config");
    ctx.display.line(4, "Enter: reload");
    return;
  }

  switch (stage_) {
    case Stage::CollectingInput:
      renderFieldEditor(ctx);
      return;
    case Stage::Preview:
      renderPreview(ctx);
      return;
    case Stage::AwaitingConfirm: {
      const WebhookCommand& command = commands_[activeCommandIndex_];
      ctx.display.status("Confirm", command.method);
      ctx.display.line(0, command.name);
      ctx.display.line(1, previewUrlLine(pendingRequest_.url));
      ctx.display.line(3, "Y confirm / N cancel");
      return;
    }
    case Stage::List:
      break;
  }

  String title = filtering_ ? ("Search: " + filterInput_.renderWindow(20)) : "Webhooks";
  menu_.render(ctx.display, title, status_);
  if (!preview_.isEmpty()) {
    ctx.display.line(5, preview_);
  }
}

void WebhookLauncherApp::renderFieldEditor(AppContext& ctx) {
  const WebhookCommand& command = commands_[activeCommandIndex_];
  const InputField& field = command.inputs[inputIndex_];
  ctx.display.status(command.name,
                      String(inputIndex_ + 1) + "/" + String(command.inputs.size()));
  ctx.display.line(0, field.label);

  switch (field.kind) {
    case InputField::Kind::ShortText:
      ctx.display.line(2, fieldInput_.renderWindow(20));
      ctx.display.line(4, "Enter: next  n: cancel");
      break;
    case InputField::Kind::Choice:
      if (!field.choices.empty()) {
        ctx.display.line(2, field.choices[choiceIndex_]);
      }
      ctx.display.line(4, "</>: choice  Enter: next");
      break;
    case InputField::Kind::Boolean:
      ctx.display.line(2, boolValue_ ? "true" : "false");
      ctx.display.line(4, "</>: toggle  Enter: next");
      break;
    case InputField::Kind::Confirmation:
      ctx.display.line(4, "y: continue  n: cancel");
      break;
  }
}

void WebhookLauncherApp::renderPreview(AppContext& ctx) {
  const WebhookCommand& command = commands_[activeCommandIndex_];
  ctx.display.status("Preview", command.method);
  ctx.display.line(0, command.name);
  ctx.display.line(1, previewUrlLine(pendingRequest_.url));

  size_t row = 2;
  for (const Header& header : pendingRequest_.headers) {
    if (row >= 5) {
      break;
    }
    String value = header.sensitive ? "***" : header.value;
    ctx.display.line(static_cast<uint8_t>(row), header.name + ": " + value);
    ++row;
  }
  ctx.display.line(5, "Enter: continue  N: cancel");
}

void WebhookLauncherApp::execute(AppContext& ctx, const WebhookCommand& command,
                                  const RenderedRequest& rendered) {
  status_ = "Connecting Wi-Fi";
  preview_ = "";

  WifiSettings settings;
  if (!ctx.config.loadWifi(settings)) {
    status_ = ctx.config.lastError();
    ctx.toast.show(status_);
    return;
  }

  if (!ctx.wifi.connect(settings)) {
    status_ = ctx.wifi.lastError();
    ctx.toast.show(status_);
    ctx.logs.appendRequest({command.name, command.method, rendered.url, 0, status_, ""});
    return;
  }

  HttpRequest request;
  request.method = command.method;
  request.url = rendered.url;
  request.allowLocalHttp = command.allowLocalHttp;
  request.headers = rendered.headers;
  request.body = rendered.body;

  HttpResponse response = ctx.http.send(request);
  if (response.ok) {
    status_ = "HTTP " + String(response.statusCode);
  } else if (response.statusCode > 0) {
    status_ = "HTTP " + String(response.statusCode);
  } else {
    status_ = response.error;
  }
  preview_ = response.preview;
  ctx.toast.show(status_);
  ctx.logs.appendRequest(
      {command.name, command.method, rendered.url, response.statusCode, status_, preview_});
}

}  // namespace cardputer_launcher
