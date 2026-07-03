// SPDX-License-Identifier: MIT OR Apache-2.0

#include "storage/RedactionRegistry.h"

#include <algorithm>

#include "util/Encoding.h"

namespace cardputer_launcher {

RedactionRegistry::RegisterResult RedactionRegistry::registerSecret(const String& rawValue) {
  if (rawValue.isEmpty()) {
    return RegisterResult::Ok;
  }

  const String candidates[3] = {rawValue, percentEncode(rawValue),
                                 escapeJsonStringInner(rawValue)};

  std::vector<String> toAdd;
  for (const String& candidate : candidates) {
    const bool known = std::find(variants_.begin(), variants_.end(), candidate) !=
                            variants_.end() ||
                        std::find(toAdd.begin(), toAdd.end(), candidate) != toAdd.end();
    if (!known) {
      toAdd.push_back(candidate);
    }
  }
  if (toAdd.empty()) {
    return RegisterResult::Ok;
  }

  if (variants_.size() + toAdd.size() > kMaxEntries) {
    return RegisterResult::TooManyEntries;
  }

  size_t addedBytes = 0;
  for (const String& candidate : toAdd) {
    addedBytes += candidate.length();
  }
  if (totalBytes_ + addedBytes > kMaxTotalBytes) {
    return RegisterResult::TooManyBytes;
  }

  for (const String& candidate : toAdd) {
    variants_.push_back(candidate);
  }
  totalBytes_ += addedBytes;
  return RegisterResult::Ok;
}

String RedactionRegistry::redact(const String& text) const {
  std::vector<String> byLengthDesc = variants_;
  std::sort(byLengthDesc.begin(), byLengthDesc.end(),
            [](const String& a, const String& b) { return a.length() > b.length(); });

  String result = text;
  for (const String& variant : byLengthDesc) {
    if (!variant.isEmpty()) {
      result.replace(variant, "[REDACTED]");
    }
  }
  return result;
}

}  // namespace cardputer_launcher
