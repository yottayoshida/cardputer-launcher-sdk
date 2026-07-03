// SPDX-License-Identifier: MIT OR Apache-2.0

#include "ui/IncrementalFilter.h"

namespace cardputer_launcher {

std::vector<size_t> filterIndices(const std::vector<String>& labels, const String& query) {
  std::vector<size_t> indices;
  if (query.isEmpty()) {
    indices.reserve(labels.size());
    for (size_t i = 0; i < labels.size(); ++i) {
      indices.push_back(i);
    }
    return indices;
  }

  String needle = query;
  needle.toLowerCase();

  for (size_t i = 0; i < labels.size(); ++i) {
    String haystack = labels[i];
    haystack.toLowerCase();
    if (haystack.indexOf(needle) >= 0) {
      indices.push_back(i);
    }
  }
  return indices;
}

}  // namespace cardputer_launcher
