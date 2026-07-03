#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <Arduino.h>
#include <vector>

#include "storage/ConfigLoader.h"
#include "storage/SecretProvider.h"

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

// Resolves {{input.<key>}} placeholders (from `bindings`) and {{secret.<ref>}}
// placeholders (resolved through `secrets`, url/body only) in a command's
// url/headers/body. Commands with neither render unchanged, so this is the
// single code path used for both dry-run preview and execute.
//
// Every secret value successfully resolved while rendering is appended to
// `resolvedSecrets` (when non-null) so the caller can register it with a
// RedactionRegistry before showing a preview or writing a log line. Header
// secretRefs are NOT included here -- they are resolved once at config load
// time (see ConfigLoader), so the caller registers those separately from
// WebhookCommand::headers.
RenderedRequest renderCommandTemplate(const WebhookCommand& command,
                                       const std::vector<TemplateBinding>& bindings,
                                       SecretProvider* secrets = nullptr,
                                       std::vector<String>* resolvedSecrets = nullptr);

}  // namespace cardputer_launcher
