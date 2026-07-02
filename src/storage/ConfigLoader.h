#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <Arduino.h>
#include <vector>

namespace cardputer_launcher {

class SecretStore;

struct WifiSettings {
  String ssid;
  String password;
};

struct Header {
  String name;
  String value;
};

struct WebhookCommand {
  String name;
  String method;
  String url;
  bool allowLocalHttp = false;
  bool confirm = false;
  std::vector<Header> headers;
  String bodyJson;
};

class ConfigLoader {
 public:
  void setSdAvailable(bool available);
  bool sdAvailable() const;
  bool ensureLayout();
  bool loadWifi(WifiSettings& settings);
  bool loadWebhooks(std::vector<WebhookCommand>& commands, SecretStore* secrets = nullptr);
  const String& lastError() const;

 private:
  bool sdAvailable_ = false;
  String lastError_;
};

}  // namespace cardputer_launcher
