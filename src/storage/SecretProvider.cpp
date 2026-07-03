// SPDX-License-Identifier: MIT OR Apache-2.0

#include "storage/SecretProvider.h"

namespace cardputer_launcher {

bool isValidSecretRefName(const String& ref) {
  if (ref.isEmpty() || ref.length() > 96) {
    return false;
  }
  for (size_t index = 0; index < ref.length(); ++index) {
    const char c = ref[index];
    const bool valid = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                       (c >= '0' && c <= '9') || c == '_' || c == '-' ||
                       c == '.' || c == ':';
    if (!valid) {
      return false;
    }
  }
  return true;
}

}  // namespace cardputer_launcher
