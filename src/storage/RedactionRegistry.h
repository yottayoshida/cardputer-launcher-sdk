#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <Arduino.h>
#include <vector>

namespace cardputer_launcher {

// Tracks the resolved secret values used while rendering and executing a
// single webhook command, so preview screens and logs can redact them by
// exact match instead of relying only on keyword heuristics. Scoped to one
// command's lifecycle: the caller must reassign a fresh instance when a
// command starts and reset it as soon as that command finishes or is
// aborted, so a resolved secret's exposure window is as short as possible.
class RedactionRegistry {
 public:
  enum class RegisterResult { Ok, TooManyEntries, TooManyBytes };

  static constexpr size_t kMaxEntries = 48;
  static constexpr size_t kMaxTotalBytes = 4096;

  // Registers a secret's raw value plus the forms it takes once substituted
  // into a URL (percent-encoded) or a JSON body (escaped), so redact() finds
  // it regardless of which surface it was rendered into. All-or-nothing: if
  // capacity does not allow registering every new variant, none are added
  // and the caller must treat this as a hard failure (see kMaxEntries /
  // kMaxTotalBytes) rather than proceed with a partially-protected secret.
  RegisterResult registerSecret(const String& rawValue);

  // Replaces every registered variant found in `text` with "[REDACTED]",
  // longest variant first so a short secret that is a substring of a longer
  // one never leaves a corrupted fragment behind.
  String redact(const String& text) const;

 private:
  std::vector<String> variants_;
  size_t totalBytes_ = 0;
};

}  // namespace cardputer_launcher
