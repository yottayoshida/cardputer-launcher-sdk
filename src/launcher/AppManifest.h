#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <stdint.h>

namespace cardputer_launcher {

constexpr uint32_t kPermissionStorageRead = 1u << 0;
constexpr uint32_t kPermissionStorageWrite = 1u << 1;
constexpr uint32_t kPermissionNetworkHttp = 1u << 2;
constexpr uint32_t kPermissionInputKeyboard = 1u << 3;
constexpr uint32_t kPermissionDisplayDraw = 1u << 4;
constexpr uint32_t kPermissionIrTransmit = 1u << 5;
constexpr uint32_t kPermissionSensorRead = 1u << 6;

constexpr uint32_t kCapabilityCommandList = 1u << 0;
constexpr uint32_t kCapabilityCommandExecute = 1u << 1;
constexpr uint32_t kCapabilityConfigReload = 1u << 2;
constexpr uint32_t kCapabilityLogView = 1u << 3;
constexpr uint32_t kCapabilityNotesEdit = 1u << 4;
constexpr uint32_t kCapabilityIrSend = 1u << 5;
constexpr uint32_t kCapabilitySensorView = 1u << 6;

struct AppManifest {
  constexpr AppManifest(const char* appId = "",
                        const char* appName = "",
                        const char* appVersion = "",
                        const char* appCategory = "",
                        const char* appConfigPath = "",
                        uint32_t appPermissions = 0,
                        uint32_t appCapabilities = 0)
      : id(appId),
        name(appName),
        version(appVersion),
        category(appCategory),
        configPath(appConfigPath),
        permissions(appPermissions),
        capabilities(appCapabilities) {}

  const char* id;
  const char* name;
  const char* version;
  const char* category;
  const char* configPath;
  uint32_t permissions;
  uint32_t capabilities;
};

}  // namespace cardputer_launcher
