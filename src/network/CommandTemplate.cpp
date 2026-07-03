// SPDX-License-Identifier: MIT OR Apache-2.0

#include "network/CommandTemplate.h"

#include <ArduinoJson.h>

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
// namespace since load-time validation already confirmed it.
String placeholderKey(const String& token) {
  int dot = token.indexOf('.');
  return dot < 0 ? token : token.substring(dot + 1);
}

String percentEncode(const String& value) {
  String result;
  result.reserve(value.length());
  for (size_t i = 0; i < value.length(); ++i) {
    const unsigned char c = static_cast<unsigned char>(value[i]);
    const bool unreserved = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                             (c >= '0' && c <= '9') || c == '-' || c == '.' || c == '_' ||
                             c == '~';
    if (unreserved) {
      result += static_cast<char>(c);
      continue;
    }
    char buf[4];
    snprintf(buf, sizeof(buf), "%%%02X", c);
    result += buf;
  }
  return result;
}

// Produces a quoted, escaped JSON string literal (including the quotes).
String escapeJsonString(const String& value) {
  JsonDocument doc;
  doc.set(value);
  String out;
  serializeJson(doc, out);
  return out;
}

bool renderUrl(const WebhookCommand& command, const std::vector<TemplateBinding>& bindings,
               String& url, String& error) {
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
    const String key = placeholderKey(url.substring(open + 2, close));

    const String* boundValue = findBindingValue(bindings, key);
    if (boundValue == nullptr) {
      error = "missing value for '" + key + "'";
      return false;
    }
    result += percentEncode(*boundValue);
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
                 String& body, String& error) {
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
    const String key = placeholderKey(body.substring(open + 2, close));

    const String* boundValue = findBindingValue(bindings, key);
    if (boundValue == nullptr) {
      error = "missing value for '" + key + "'";
      return false;
    }

    const InputField* field = findInputField(command.inputs, key);
    if (field != nullptr && field->kind == InputField::Kind::Boolean) {
      result += (*boundValue == "true") ? "true" : "false";
    } else {
      result += escapeJsonString(*boundValue);
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
                                       const std::vector<TemplateBinding>& bindings) {
  RenderedRequest rendered;

  if (!renderUrl(command, bindings, rendered.url, rendered.error)) {
    return rendered;
  }
  if (!renderHeaders(command, bindings, rendered.headers, rendered.error)) {
    return rendered;
  }
  if (!renderBody(command, bindings, rendered.body, rendered.error)) {
    return rendered;
  }

  rendered.ok = true;
  return rendered;
}

}  // namespace cardputer_launcher
