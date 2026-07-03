#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <Arduino.h>
#include <vector>

#include "storage/ConfigLoader.h"

namespace cardputer_launcher {

// A single collected value for a WebhookCommand's InputField, keyed by
// InputField::key. Boolean fields store "true"/"false" as text; the renderer
// looks up the field's declared Kind to decide how to encode the value.
struct TemplateBinding {
  String key;
  String value;
};

struct RenderedRequest {
  bool ok = false;
  String error;
  String url;
  std::vector<Header> headers;
  String body;
};

// Resolves {{input.<key>}} placeholders in a command's url/headers/body using
// `bindings`. Commands with no inputs render unchanged, so this is the single
// code path used for both dry-run preview and execute.
RenderedRequest renderCommandTemplate(const WebhookCommand& command,
                                       const std::vector<TemplateBinding>& bindings);

}  // namespace cardputer_launcher
