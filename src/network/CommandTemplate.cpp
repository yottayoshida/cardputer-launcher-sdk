// SPDX-License-Identifier: MIT OR Apache-2.0

#include "network/CommandTemplate.h"

#include <ArduinoJson.h>

#include "util/Encoding.h"

namespace cardputer_launcher {

namespace {

constexpr size_t kMaxUrlLength = 2048;
constexpr size_t kMaxHeaderValueLength = 512;
// Matches HttpClient's kMaxRequestBodyBytes; checked here too so an
// oversized body is rejected at render time with a clear error instead of
// only failing later inside HttpClient::send().
constexpr size_t kMaxBodyBytes = 2048;

const String* findBindingValue(const std::vector<TemplateBinding>& bindings, const String& key) {
  for (const TemplateBinding& binding : bindings) {
    if (binding.key == key) {
      return &binding.value;
    }
  }
  return nullptr;
}

const InputField* findInputField(const std::vector<InputField>& inputs, const String& key) {
  for (const InputField& field : inputs) {
    if (field.key == key) {
      return &field;
    }
  }
  return nullptr;
}

// A validated placeholder token is always "input.<key>"; strip the
// namespace since load-time validation already confirmed it. Only used for
// the input-only paths (headers, and the InputField lookup in renderBody);
// url/body placeholder resolution goes through resolvePlaceholderValue below
// so it can also reach the secret namespace.
String placeholderKey(const String& token) {
  int dot = token.indexOf('.');
  return dot < 0 ? token : token.substring(dot + 1);
}

// Resolves a validated "input.<key>" or "secret.<ref>" token to its
// underlying value, also returning the bare key/ref (namespace stripped) so
// callers that need it for a further lookup (renderBody's InputField check)
// don't have to re-split the same token. A resolved secret is appended to
// `resolvedSecrets` (when non-null) so the caller can register it with a
// RedactionRegistry before showing a preview or writing a log line.
bool resolvePlaceholderValue(const String& token, const std::vector<TemplateBinding>& bindings,
                              SecretProvider* secrets, std::vector<String>* resolvedSecrets,
                              String& value, String& key, bool& isSecret, String& error) {
  int dot = token.indexOf('.');
  const String ns = dot < 0 ? token : token.substring(0, dot);
  key = dot < 0 ? token : token.substring(dot + 1);
  isSecret = false;

  if (ns == "secret") {
    if (!secrets) {
      error = "secret store unavailable";
      return false;
    }
    if (!secrets->resolve(key, value)) {
      error = secrets->lastError();
      return false;
    }
    isSecret = true;
    if (resolvedSecrets) {
      resolvedSecrets->push_back(value);
    }
    return true;
  }

  const String* boundValue = findBindingValue(bindings, key);
  if (boundValue == nullptr) {
    error = "missing value for '" + key + "'";
    return false;
  }
  value = *boundValue;
  return true;
}

bool renderUrl(const WebhookCommand& command, const std::vector<TemplateBinding>& bindings,
               SecretProvider* secrets, std::vector<String>* resolvedSecrets, String& url,
               String& error) {
  url = command.url;
  if (url.indexOf("{{") < 0) {
    return revalidateResolvedUrl(url, command.allowLocalHttp, error);
  }

  String result;
  result.reserve(url.length());
  int cursor = 0;
  while (true) {
    int open = url.indexOf("{{", cursor);
    if (open < 0) {
      result += url.substring(cursor);
      break;
    }
    result += url.substring(cursor, open);
    int close = url.indexOf("}}", open + 2);
    const String token = url.substring(open + 2, close);

    String value;
    String key;
    bool isSecret = false;
    if (!resolvePlaceholderValue(token, bindings, secrets, resolvedSecrets, value, key, isSecret,
                                  error)) {
      return false;
    }
    result += percentEncode(value);
    cursor = close + 2;
  }

  url = result;
  if (url.length() > kMaxUrlLength) {
    error = "url exceeds " + String(kMaxUrlLength) + " bytes after substitution";
    return false;
  }
  return revalidateResolvedUrl(url, command.allowLocalHttp, error);
}

bool renderHeaders(const WebhookCommand& command, const std::vector<TemplateBinding>& bindings,
                    std::vector<Header>& headers, String& error) {
  headers.clear();
  for (const Header& header : command.headers) {
    Header rendered;
    rendered.name = header.name;
    rendered.sensitive = header.sensitive;

    if (header.sensitive || header.value.indexOf("{{") < 0) {
      rendered.value = header.value;
    } else {
      String result;
      result.reserve(header.value.length());
      int cursor = 0;
      while (true) {
        int open = header.value.indexOf("{{", cursor);
        if (open < 0) {
          result += header.value.substring(cursor);
          break;
        }
        result += header.value.substring(cursor, open);
        int close = header.value.indexOf("}}", open + 2);
        const String key = placeholderKey(header.value.substring(open + 2, close));

        const String* boundValue = findBindingValue(bindings, key);
        if (boundValue == nullptr) {
          error = "missing value for '" + key + "'";
          return false;
        }
        if (boundValue->indexOf('\r') >= 0 || boundValue->indexOf('\n') >= 0) {
          error = "value for '" + key + "' cannot contain a line break";
          return false;
        }
        result += *boundValue;
        cursor = close + 2;
      }
      rendered.value = result;
    }

    if (rendered.value.length() > kMaxHeaderValueLength) {
      error = "header '" + rendered.name + "' exceeds " + String(kMaxHeaderValueLength) +
              " bytes after substitution";
      return false;
    }
    headers.push_back(rendered);
  }
  return true;
}

bool renderBody(const WebhookCommand& command, const std::vector<TemplateBinding>& bindings,
                 SecretProvider* secrets, std::vector<String>* resolvedSecrets, String& body,
                 String& error) {
  body = command.bodyJson;
  if (body.indexOf("{{") < 0) {
    return true;
  }

  String result;
  result.reserve(body.length());
  int cursor = 0;
  while (true) {
    int open = body.indexOf("{{", cursor);
    if (open < 0) {
      result += body.substring(cursor);
      break;
    }
    int close = body.indexOf("}}", open + 2);
    // Load-time validation guarantees this placeholder is the entire quoted
    // JSON string value, so the surrounding quotes sit just outside {{ }}.
    const int quoteStart = open - 1;
    const int quoteEnd = close + 3;

    result += body.substring(cursor, quoteStart);
    const String token = body.substring(open + 2, close);

    String value;
    String key;
    bool isSecret = false;
    if (!resolvePlaceholderValue(token, bindings, secrets, resolvedSecrets, value, key, isSecret,
                                  error)) {
      return false;
    }

    // Boolean-typed inputs render as an unquoted JSON literal; secrets never
    // do (a secret ref has no declared InputField kind), so they always take
    // the escaped-string path below.
    const InputField* field = isSecret ? nullptr : findInputField(command.inputs, key);
    if (field != nullptr && field->kind == InputField::Kind::Boolean) {
      result += (value == "true") ? "true" : "false";
    } else {
      result += escapeJsonString(value);
    }

    cursor = quoteEnd;
  }

  body = result;
  if (body.length() > kMaxBodyBytes) {
    error = "body exceeds " + String(kMaxBodyBytes) + " bytes after substitution";
    return false;
  }
  return true;
}

}  // namespace

RenderedRequest renderCommandTemplate(const WebhookCommand& command,
                                       const std::vector<TemplateBinding>& bindings,
                                       SecretProvider* secrets,
                                       std::vector<String>* resolvedSecrets) {
  RenderedRequest rendered;

  if (!renderUrl(command, bindings, secrets, resolvedSecrets, rendered.url, rendered.error)) {
    return rendered;
  }
  if (!renderHeaders(command, bindings, rendered.headers, rendered.error)) {
    return rendered;
  }
  if (!renderBody(command, bindings, secrets, resolvedSecrets, rendered.body, rendered.error)) {
    return rendered;
  }

  rendered.ok = true;
  return rendered;
}

}  // namespace cardputer_launcher
