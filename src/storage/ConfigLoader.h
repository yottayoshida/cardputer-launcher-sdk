#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <Arduino.h>
#include <vector>

#include "storage/SecretProvider.h"

namespace cardputer_launcher {

struct WifiSettings {
  String ssid;
  String password;
};

struct Header {
  String name;
  String value;
  // True when value was resolved from a secretRef. Consumers (preview, logs)
  // must not display the raw value when this is set.
  bool sensitive = false;
};

enum class RiskLevel { Low, Medium, High };

struct InputField {
  enum class Kind { ShortText, Choice, Boolean, Confirmation };

  String key;
  Kind kind = Kind::ShortText;
  String label;
  String defaultValue;
  std::vector<String> choices;  // Kind::Choice only, 2-8 entries
  size_t maxLength = 96;        // Kind::ShortText only, 1-128
  bool required = true;
};

struct WebhookCommand {
  String name;
  String method;
  String url;
  bool allowLocalHttp = false;
  bool confirm = false;
  std::vector<Header> headers;
  String bodyJson;
  String category;  // empty means "Uncategorized" for display purposes
  String description;
  RiskLevel risk = RiskLevel::Low;
  bool requirePreview = false;
  std::vector<InputField> inputs;
};

// Re-validates a URL after template placeholders have been resolved. Defense
// in depth for the execute-time renderer: even a substitution bug can never
// send a request to a host the config did not declare.
bool revalidateResolvedUrl(const String& url, bool allowLocalHttp, String& error);

class ConfigLoader {
 public:
  void setSdAvailable(bool available);
  bool sdAvailable() const;
  bool ensureLayout();
  bool loadWifi(WifiSettings& settings, SecretProvider* secrets = nullptr);
  bool loadWebhooks(std::vector<WebhookCommand>& commands, SecretProvider* secrets = nullptr);
  const String& lastError() const;

 private:
  bool sdAvailable_ = false;
  String lastError_;
};

}  // namespace cardputer_launcher
