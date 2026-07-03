#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <Arduino.h>
#include <vector>

namespace cardputer_launcher {

// Returns indices into `labels` whose text contains `query` (case-insensitive
// substring match), preserving original order. An empty query returns every
// index unfiltered.
std::vector<size_t> filterIndices(const std::vector<String>& labels, const String& query);

}  // namespace cardputer_launcher
