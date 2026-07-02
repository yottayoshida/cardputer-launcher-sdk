// SPDX-License-Identifier: MIT OR Apache-2.0

#include "storage/SdLayout.h"

#include <SD.h>

namespace cardputer_launcher {

const char* const kSdAppsDir = "/apps";
const char* const kSdWebhookAppDir = "/apps/webhook_launcher";
const char* const kSdLogDir = "/logs";
const char* const kSdCacheDir = "/cache";
const char* const kSdBackupsDir = "/backups";
const char* const kSdWebhookCommandsPath = "/apps/webhook_launcher/commands.json";
const char* const kSdLegacyWebhookConfigPath = "/apps/webhook_launcher.json";
const char* const kSdLauncherLogPath = "/logs/launcher.jsonl";

namespace {

bool directoryExists(const char* path) {
  File entry = SD.open(path, FILE_READ);
  if (!entry) {
    return false;
  }
  const bool exists = entry.isDirectory();
  entry.close();
  return exists;
}

bool ensureDirectory(const char* path, String& error) {
  if (directoryExists(path)) {
    return true;
  }
  if (SD.exists(path)) {
    error = String("SD path is not a directory: ") + path;
    return false;
  }
  if (!SD.mkdir(path)) {
    error = String("SD mkdir failed: ") + path;
    return false;
  }
  return true;
}

}  // namespace

bool ensureSdLayout(bool sdAvailable, String& error) {
  if (!sdAvailable) {
    error = "SD unavailable";
    return false;
  }

  const char* const directories[] = {
      kSdAppsDir,
      kSdWebhookAppDir,
      kSdLogDir,
      kSdCacheDir,
      kSdBackupsDir,
  };

  for (const char* directory : directories) {
    if (!ensureDirectory(directory, error)) {
      return false;
    }
  }

  error = "";
  return true;
}

}  // namespace cardputer_launcher
