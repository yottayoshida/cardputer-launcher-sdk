#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include "network/HttpClient.h"
#include "network/WifiManager.h"
#include "storage/ConfigLoader.h"
#include "storage/LogStore.h"
#include "storage/SecretStore.h"
#include "ui/DisplayPort.h"
#include "ui/Toast.h"

namespace cardputer_launcher {

struct AppContext {
  DisplayPort& display;
  ConfigLoader& config;
  LogStore& logs;
  SecretStore& secrets;
  WifiManager& wifi;
  HttpClient& http;
  Toast& toast;
  bool sdAvailable;
};

}  // namespace cardputer_launcher
