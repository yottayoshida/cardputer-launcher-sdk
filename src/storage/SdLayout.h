#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <Arduino.h>

namespace cardputer_launcher {

extern const char* const kSdAppsDir;
extern const char* const kSdWebhookAppDir;
extern const char* const kSdLogDir;
extern const char* const kSdCacheDir;
extern const char* const kSdBackupsDir;
extern const char* const kSdWebhookCommandsPath;
extern const char* const kSdLegacyWebhookConfigPath;
extern const char* const kSdLauncherLogPath;

bool ensureSdLayout(bool sdAvailable, String& error);

}  // namespace cardputer_launcher
