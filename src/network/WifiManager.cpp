// SPDX-License-Identifier: MIT OR Apache-2.0

#include "network/WifiManager.h"

#include <WiFi.h>

namespace cardputer_launcher {

bool WifiManager::connect(const WifiSettings& settings, uint32_t timeoutMs) {
  if (settings.ssid.isEmpty()) {
    lastError_ = "Wi-Fi SSID missing";
    return false;
  }

  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(settings.ssid.c_str(), settings.password.c_str());
  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutMs) {
    delay(250);
  }

  if (WiFi.status() != WL_CONNECTED) {
    lastError_ = "Wi-Fi connect failed";
    return false;
  }

  lastError_ = "";
  return true;
}

bool WifiManager::isConnected() const { return WiFi.status() == WL_CONNECTED; }

const String& WifiManager::lastError() const { return lastError_; }

}  // namespace cardputer_launcher

