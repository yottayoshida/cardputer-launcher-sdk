// SPDX-License-Identifier: MIT OR Apache-2.0

#include "apps/WebhookLauncherApp.h"

#include "launcher/AppContext.h"
#include "network/HttpClient.h"
#include "storage/LogStore.h"

namespace cardputer_launcher {

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
  menu_.clear();
  awaitingConfirm_ = false;
  preview_ = "";
  configOk_ = ctx.config.loadWebhooks(commands_, &ctx.secrets);
  if (!configOk_) {
    status_ = ctx.config.lastError();
    return;
  }
  rebuildMenu();
  status_ = String(commands_.size()) + " command(s)";
}

void WebhookLauncherApp::onInput(AppContext& ctx, const InputEvent& event) {
  if (!configOk_ || commands_.empty()) {
    if (event.action == InputAction::Select) {
      onStart(ctx);
    }
    return;
  }

  if (awaitingConfirm_) {
    if (event.action == InputAction::Confirm || event.action == InputAction::Select) {
      awaitingConfirm_ = false;
      execute(ctx, commands_[menu_.selectedIndex()]);
    } else if (event.action == InputAction::Cancel || event.action == InputAction::Back) {
      awaitingConfirm_ = false;
      status_ = "Canceled";
    }
    return;
  }

  if (event.action == InputAction::Up) {
    menu_.previous();
  } else if (event.action == InputAction::Down) {
    menu_.next();
  } else if (event.action == InputAction::Select) {
    WebhookCommand& command = commands_[menu_.selectedIndex()];
    if (command.confirm) {
      awaitingConfirm_ = true;
      status_ = "Y confirm / N cancel";
    } else {
      execute(ctx, command);
    }
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

  if (awaitingConfirm_) {
    const WebhookCommand& command = commands_[menu_.selectedIndex()];
    ctx.display.status("Confirm", command.method);
    ctx.display.line(0, command.name);
    ctx.display.line(1, command.url);
    ctx.display.line(3, "Y confirm / N cancel");
    return;
  }

  menu_.render(ctx.display, "Webhooks", status_);
  if (!preview_.isEmpty()) {
    ctx.display.line(5, preview_);
  }
}

void WebhookLauncherApp::execute(AppContext& ctx, const WebhookCommand& command) {
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
    ctx.logs.appendRequest({command.name, command.method, command.url, 0, status_, ""});
    return;
  }

  HttpRequest request;
  request.method = command.method;
  request.url = command.url;
  request.headers = command.headers;
  request.body = command.bodyJson;

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
      {command.name, command.method, command.url, response.statusCode, status_, preview_});
}

void WebhookLauncherApp::rebuildMenu() {
  menu_.clear();
  for (const WebhookCommand& command : commands_) {
    menu_.addItem(command.name);
  }
}

}  // namespace cardputer_launcher
