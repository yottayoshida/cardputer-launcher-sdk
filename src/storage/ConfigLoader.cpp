// SPDX-License-Identifier: MIT OR Apache-2.0

#include "storage/ConfigLoader.h"

#include <ArduinoJson.h>
#include <SD.h>

#include "storage/SdLayout.h"
#include "storage/SecretStore.h"

namespace cardputer_launcher {

namespace {

bool ensureSd(bool available, String& error) {
  if (!available) {
    error = "SD unavailable";
    return false;
  }
  return true;
}

bool readRequiredString(JsonVariantConst value, const String& path, String& out, String& error) {
  if (!value.is<const char*>()) {
    error = path + " must be a non-empty string";
    return false;
  }

  String raw = value.as<const char*>();
  String probe = raw;
  probe.trim();
  if (probe.isEmpty()) {
    error = path + " must be a non-empty string";
    return false;
  }
  out = raw;
  return true;
}

bool validateVersion(JsonVariantConst value, const String& path, String& error) {
  if (!value.is<int>() || value.as<int>() != 1) {
    error = path + " must be 1";
    return false;
  }
  return true;
}

bool hasMember(JsonObject object, const char* key) {
  for (JsonPair kv : object) {
    if (String(kv.key().c_str()) == key) {
      return true;
    }
  }
  return false;
}

// Parses an optional boolean field, leaving `out` untouched when absent.
bool parseOptionalBool(JsonObject item, const char* key, const String& path, bool& out,
                        String& error) {
  if (!hasMember(item, key)) {
    return true;
  }
  if (!item[key].is<bool>()) {
    error = path + "." + key + " must be true or false";
    return false;
  }
  out = item[key].as<bool>();
  return true;
}

bool resolveHeaderValue(JsonVariant value, SecretStore* secrets, String& resolved,
                        bool& wasSecret, String& error) {
  resolved = "";
  wasSecret = false;

  const char* literal = value.as<const char*>();
  if (literal) {
    resolved = literal;
    return true;
  }

  if (!value.is<JsonObject>()) {
    error = "webhook header invalid";
    return false;
  }

  JsonObject object = value.as<JsonObject>();
  if (object.size() != 1) {
    error = "secretRef object invalid";
    return false;
  }
  const char* ref = object["secretRef"] | "";
  if (strlen(ref) == 0) {
    error = "secretRef missing";
    return false;
  }
  if (!secrets) {
    error = "secret store unavailable";
    return false;
  }
  if (!secrets->resolve(ref, resolved)) {
    error = secrets->lastError();
    return false;
  }
  wasSecret = true;
  return true;
}

bool parseRiskLevel(JsonVariantConst value, const String& path, RiskLevel& risk, String& error) {
  const char* raw = value.as<const char*>();
  if (!raw) {
    error = path + " must be a string";
    return false;
  }
  String level = raw;
  if (level == "low") {
    risk = RiskLevel::Low;
  } else if (level == "medium") {
    risk = RiskLevel::Medium;
  } else if (level == "high") {
    risk = RiskLevel::High;
  } else {
    error = path + " must be low, medium, or high";
    return false;
  }
  return true;
}

bool isValidInputKey(const String& key) {
  if (key.isEmpty() || !(key[0] >= 'a' && key[0] <= 'z')) {
    return false;
  }
  for (size_t i = 0; i < key.length(); ++i) {
    char c = key[i];
    bool ok = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_';
    if (!ok) {
      return false;
    }
  }
  return true;
}

bool parseInputFieldKind(const String& raw, InputField::Kind& kind) {
  if (raw == "text") {
    kind = InputField::Kind::ShortText;
  } else if (raw == "choice") {
    kind = InputField::Kind::Choice;
  } else if (raw == "boolean") {
    kind = InputField::Kind::Boolean;
  } else if (raw == "confirmation") {
    kind = InputField::Kind::Confirmation;
  } else {
    return false;
  }
  return true;
}

bool parseInputField(JsonObject item, const String& path, InputField& field, String& error) {
  if (!readRequiredString(item["key"], path + ".key", field.key, error)) {
    return false;
  }
  if (!isValidInputKey(field.key)) {
    error = path + ".key must start with a-z and contain only a-z, 0-9, or _";
    return false;
  }

  String kindRaw;
  if (!readRequiredString(item["kind"], path + ".kind", kindRaw, error)) {
    return false;
  }
  if (!parseInputFieldKind(kindRaw, field.kind)) {
    error = path + ".kind must be text, choice, boolean, or confirmation";
    return false;
  }

  if (!readRequiredString(item["label"], path + ".label", field.label, error)) {
    return false;
  }

  if (!parseOptionalBool(item, "required", path, field.required, error)) {
    return false;
  }

  const bool hasChoices = hasMember(item, "choices");
  if (field.kind == InputField::Kind::Choice) {
    if (!hasChoices || !item["choices"].is<JsonArray>()) {
      error = path + ".choices must be an array for kind choice";
      return false;
    }
    JsonArray choicesArray = item["choices"].as<JsonArray>();
    if (choicesArray.size() < 2 || choicesArray.size() > 8) {
      error = path + ".choices must have 2 to 8 entries";
      return false;
    }
    for (JsonVariant choiceVariant : choicesArray) {
      String choice;
      if (!readRequiredString(choiceVariant, path + ".choices[]", choice, error)) {
        return false;
      }
      field.choices.push_back(choice);
    }
  } else if (hasChoices) {
    error = path + ".choices is only valid for kind choice";
    return false;
  }

  const bool hasMaxLength = hasMember(item, "maxLength");
  if (field.kind == InputField::Kind::ShortText) {
    if (hasMaxLength) {
      if (!item["maxLength"].is<int>()) {
        error = path + ".maxLength must be an integer";
        return false;
      }
      int maxLength = item["maxLength"].as<int>();
      if (maxLength < 1 || maxLength > 128) {
        error = path + ".maxLength must be between 1 and 128";
        return false;
      }
      field.maxLength = static_cast<size_t>(maxLength);
    }
  } else if (hasMaxLength) {
    error = path + ".maxLength is only valid for kind text";
    return false;
  }

  // Validated after choices/maxLength are known so a bad default (too long,
  // not a declared choice, not a real boolean) fails loudly at load time
  // instead of being silently clamped or ignored when the field is shown.
  if (hasMember(item, "default")) {
    const char* raw = item["default"].as<const char*>();
    if (!raw) {
      error = path + ".default must be a string";
      return false;
    }
    field.defaultValue = raw;

    switch (field.kind) {
      case InputField::Kind::ShortText:
        if (field.defaultValue.length() > field.maxLength) {
          error = path + ".default exceeds maxLength";
          return false;
        }
        break;
      case InputField::Kind::Choice: {
        bool found = false;
        for (const String& choice : field.choices) {
          if (choice == field.defaultValue) {
            found = true;
            break;
          }
        }
        if (!found) {
          error = path + ".default must be one of choices";
          return false;
        }
        break;
      }
      case InputField::Kind::Boolean:
        if (field.defaultValue != "true" && field.defaultValue != "false") {
          error = path + ".default must be true or false";
          return false;
        }
        break;
      case InputField::Kind::Confirmation:
        error = path + ".default is not valid for kind confirmation";
        return false;
    }
  }

  return true;
}

bool isLoopbackHost(const String& host) {
  return host.equalsIgnoreCase("localhost") || host == "127.0.0.1" || host == "::1";
}

String stripPort(const String& hostPort) {
  if (hostPort.startsWith("[")) {
    int end = hostPort.indexOf(']');
    return end >= 0 ? hostPort.substring(1, end) : hostPort;
  }
  int colon = hostPort.indexOf(':');
  return colon >= 0 ? hostPort.substring(0, colon) : hostPort;
}

// Strips a userinfo prefix ("user:pass@") so a URL like
// "http://127.0.0.1@evil.com" cannot spoof the loopback check while the
// underlying HTTP client actually connects to the host after '@'.
String stripUserinfo(const String& hostPort) {
  int at = hostPort.lastIndexOf('@');
  return at >= 0 ? hostPort.substring(at + 1) : hostPort;
}

// RFC 3986 scheme names are case-insensitive; the host-side Python validator
// already normalizes via urlparse, so firmware matches that here.
bool startsWithScheme(const String& url, const char* scheme) {
  size_t len = strlen(scheme);
  if (url.length() < len) {
    return false;
  }
  return url.substring(0, len).equalsIgnoreCase(scheme);
}

// Only the "input" namespace is implemented in this PR. "secret" and any
// other namespace are reserved so a config that loads today keeps the same
// meaning once secret-backed placeholders are implemented (see SECURITY.md).
bool validatePlaceholderToken(const String& token, const std::vector<InputField>& inputs,
                               const String& path, String& error) {
  int dot = token.indexOf('.');
  if (dot < 0) {
    error = path + " placeholder must be namespace.key";
    return false;
  }
  String ns = token.substring(0, dot);
  String key = token.substring(dot + 1);
  if (ns != "input") {
    error = path + " placeholder namespace '" + ns + "' is reserved or unknown";
    return false;
  }
  for (const InputField& field : inputs) {
    if (field.key == key) {
      return true;
    }
  }
  error = path + " references undefined input '" + key + "'";
  return false;
}

// Used for header values and (indirectly) URLs: any number of "{{input.x}}"
// placeholders embedded anywhere in free text.
bool validateEmbeddedPlaceholders(const String& text, const std::vector<InputField>& inputs,
                                   const String& path, String& error) {
  int searchFrom = 0;
  while (true) {
    int open = text.indexOf("{{", searchFrom);
    if (open < 0) {
      return true;
    }
    int close = text.indexOf("}}", open + 2);
    if (close < 0) {
      error = path + " has an unterminated placeholder";
      return false;
    }
    if (!validatePlaceholderToken(text.substring(open + 2, close), inputs, path, error)) {
      return false;
    }
    searchFrom = close + 2;
  }
}

// Placeholders may appear anywhere at or after `authorityEnd` (the URL's
// path/query/fragment), never in the scheme/userinfo/host/port, so a typed
// value can never redirect a request to a different host.
bool validateUrlPlaceholders(const String& url, size_t authorityEnd,
                              const std::vector<InputField>& inputs, const String& path,
                              String& error) {
  int firstOpen = url.indexOf("{{");
  if (firstOpen >= 0 && static_cast<size_t>(firstOpen) < authorityEnd) {
    error = path + " placeholder not allowed before the host";
    return false;
  }
  return validateEmbeddedPlaceholders(url, inputs, path, error);
}

// Body placeholders must occupy an entire JSON string value ("{{input.x}}",
// quotes included) so substitution is a single token replacement rather than
// string concatenation inside arbitrary JSON text.
bool validateBodyPlaceholders(const String& bodyJson, const std::vector<InputField>& inputs,
                               const String& path, String& error) {
  int searchFrom = 0;
  while (true) {
    int open = bodyJson.indexOf("{{", searchFrom);
    if (open < 0) {
      return true;
    }
    int close = bodyJson.indexOf("}}", open + 2);
    if (close < 0) {
      error = path + " has an unterminated placeholder";
      return false;
    }
    const bool quotedWhole = open > 0 && bodyJson[open - 1] == '"' &&
                              close + 2 < static_cast<int>(bodyJson.length()) &&
                              bodyJson[close + 2] == '"';
    if (!quotedWhole) {
      error = path + " placeholder must be the entire JSON string value";
      return false;
    }
    // ArduinoJson's compact serialization puts ':' immediately after a key's
    // closing quote with no whitespace, so this reliably tells a key from a
    // value. Placeholders may only replace values, never field names.
    const int afterQuote = close + 3;
    if (afterQuote < static_cast<int>(bodyJson.length()) && bodyJson[afterQuote] == ':') {
      error = path + " placeholder must be a value, not an object key";
      return false;
    }
    if (!validatePlaceholderToken(bodyJson.substring(open + 2, close), inputs, path, error)) {
      return false;
    }
    searchFrom = close + 2;
  }
}

bool validateHttpsUrl(const String& url, const String& path, bool allowLocalHttp,
                       const std::vector<InputField>& inputs, String& error) {
  bool isHttps = startsWithScheme(url, "https://");
  if (!isHttps && !(allowLocalHttp && startsWithScheme(url, "http://"))) {
    error = path + " must use https";
    return false;
  }

  const size_t schemeLen = isHttps ? 8 : 7;
  String hostAndPath = url.substring(schemeLen);
  int slash = hostAndPath.indexOf('/');
  int query = hostAndPath.indexOf('?');
  int fragment = hostAndPath.indexOf('#');
  int end = hostAndPath.length();
  if (slash >= 0 && slash < end) {
    end = slash;
  }
  if (query >= 0 && query < end) {
    end = query;
  }
  if (fragment >= 0 && fragment < end) {
    end = fragment;
  }
  String hostPort = hostAndPath.substring(0, end);
  hostPort.trim();
  if (hostPort.isEmpty()) {
    error = path + " must include a host";
    return false;
  }

  if (!isHttps && !isLoopbackHost(stripPort(stripUserinfo(hostPort)))) {
    error = path + " local HTTP must use a loopback host";
    return false;
  }

  if (!validateUrlPlaceholders(url, schemeLen + static_cast<size_t>(end), inputs, path, error)) {
    return false;
  }
  return true;
}

File openWebhookConfig() {
  File appPack = SD.open(kSdWebhookCommandsPath, FILE_READ);
  if (appPack) {
    return appPack;
  }
  return SD.open(kSdLegacyWebhookConfigPath, FILE_READ);
}

}  // namespace

bool revalidateResolvedUrl(const String& url, bool allowLocalHttp, String& error) {
  if (url.indexOf("{{") >= 0) {
    error = "url still contains an unresolved placeholder";
    return false;
  }
  return validateHttpsUrl(url, "url", allowLocalHttp, {}, error);
}

void ConfigLoader::setSdAvailable(bool available) { sdAvailable_ = available; }

bool ConfigLoader::sdAvailable() const { return sdAvailable_; }

bool ConfigLoader::ensureLayout() { return ensureSdLayout(sdAvailable_, lastError_); }

bool ConfigLoader::loadWifi(WifiSettings& settings) {
  if (!ensureSd(sdAvailable_, lastError_)) {
    return false;
  }

  File file = SD.open("/settings.json", FILE_READ);
  if (!file) {
    lastError_ = "settings.json missing";
    return false;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  if (error) {
    lastError_ = String("settings parse: ") + error.c_str();
    return false;
  }

  if (!validateVersion(doc["version"], "settings.version", lastError_)) {
    return false;
  }

  if (!doc["wifi"].is<JsonObject>()) {
    lastError_ = "settings.wifi must be an object";
    return false;
  }

  String ssid;
  if (!readRequiredString(doc["wifi"]["ssid"], "settings.wifi.ssid", ssid, lastError_)) {
    return false;
  }

  String password;
  if (!readRequiredString(doc["wifi"]["password"], "settings.wifi.password", password,
                          lastError_)) {
    return false;
  }

  settings.ssid = ssid;
  settings.password = password;
  lastError_ = "";
  return true;
}

bool ConfigLoader::loadWebhooks(std::vector<WebhookCommand>& commands,
                                SecretStore* secrets) {
  commands.clear();
  if (!ensureSd(sdAvailable_, lastError_)) {
    return false;
  }

  File file = openWebhookConfig();
  if (!file) {
    lastError_ = "webhook config missing";
    return false;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  if (error) {
    lastError_ = String("webhook parse: ") + error.c_str();
    return false;
  }

  if (!validateVersion(doc["version"], "webhook config version", lastError_)) {
    return false;
  }

  if (!doc["commands"].is<JsonArray>()) {
    lastError_ = "webhook config commands must be a non-empty array";
    return false;
  }

  JsonArray items = doc["commands"].as<JsonArray>();
  if (items.size() == 0) {
    lastError_ = "webhook config commands must be a non-empty array";
    return false;
  }

  size_t index = 0;
  for (JsonVariant itemVariant : items) {
    String path = String("commands[") + index + "]";
    if (!itemVariant.is<JsonObject>()) {
      lastError_ = path + " must be an object";
      commands.clear();
      return false;
    }

    JsonObject item = itemVariant.as<JsonObject>();
    WebhookCommand command;
    if (!readRequiredString(item["name"], path + ".name", command.name, lastError_)) {
      commands.clear();
      return false;
    }

    if (!readRequiredString(item["method"], path + ".method", command.method, lastError_)) {
      commands.clear();
      return false;
    }
    command.method.toUpperCase();

    if (command.method != "GET" && command.method != "POST") {
      lastError_ = path + ".method must be GET or POST";
      commands.clear();
      return false;
    }

    if (!readRequiredString(item["url"], path + ".url", command.url, lastError_)) {
      commands.clear();
      return false;
    }

    if (!parseOptionalBool(item, "allowLocalHttp", path, command.allowLocalHttp, lastError_)) {
      commands.clear();
      return false;
    }

    if (!parseOptionalBool(item, "confirm", path, command.confirm, lastError_)) {
      commands.clear();
      return false;
    }

    if (hasMember(item, "category")) {
      if (!readRequiredString(item["category"], path + ".category", command.category,
                               lastError_)) {
        commands.clear();
        return false;
      }
    }

    if (hasMember(item, "description")) {
      if (!readRequiredString(item["description"], path + ".description", command.description,
                               lastError_)) {
        commands.clear();
        return false;
      }
    }

    if (hasMember(item, "risk")) {
      if (!parseRiskLevel(item["risk"], path + ".risk", command.risk, lastError_)) {
        commands.clear();
        return false;
      }
    }

    if (!parseOptionalBool(item, "requirePreview", path, command.requirePreview, lastError_)) {
      commands.clear();
      return false;
    }

    if (command.risk == RiskLevel::High && !(command.confirm && command.requirePreview)) {
      lastError_ = path + " risk:high requires confirm:true and requirePreview:true";
      commands.clear();
      return false;
    }

    if (hasMember(item, "inputs")) {
      if (!item["inputs"].is<JsonArray>()) {
        lastError_ = path + ".inputs must be an array";
        commands.clear();
        return false;
      }
      JsonArray inputsArray = item["inputs"].as<JsonArray>();
      size_t inputIndex = 0;
      for (JsonVariant inputVariant : inputsArray) {
        if (!inputVariant.is<JsonObject>()) {
          lastError_ = path + ".inputs[" + inputIndex + "] must be an object";
          commands.clear();
          return false;
        }
        InputField field;
        String inputPath = path + ".inputs[" + inputIndex + "]";
        if (!parseInputField(inputVariant.as<JsonObject>(), inputPath, field, lastError_)) {
          commands.clear();
          return false;
        }
        command.inputs.push_back(field);
        ++inputIndex;
      }

      for (size_t i = 0; i < command.inputs.size(); ++i) {
        for (size_t j = i + 1; j < command.inputs.size(); ++j) {
          if (command.inputs[i].key == command.inputs[j].key) {
            lastError_ = path + ".inputs has duplicate key '" + command.inputs[i].key + "'";
            commands.clear();
            return false;
          }
        }
      }
    }

    if (!validateHttpsUrl(command.url, path + ".url", command.allowLocalHttp, command.inputs,
                           lastError_)) {
      commands.clear();
      return false;
    }

    if (!item["headers"].isNull()) {
      if (!item["headers"].is<JsonObject>()) {
        lastError_ = path + ".headers must be an object";
        commands.clear();
        return false;
      }
      for (JsonPair kv : item["headers"].as<JsonObject>()) {
        String key = kv.key().c_str();
        String keyProbe = key;
        keyProbe.trim();
        if (keyProbe.isEmpty()) {
          lastError_ = path + ".headers key must be a non-empty string";
          commands.clear();
          return false;
        }

        Header header;
        header.name = key;
        if (!resolveHeaderValue(kv.value(), secrets, header.value, header.sensitive,
                                 lastError_)) {
          commands.clear();
          return false;
        }
        // Secret-backed values are opaque; only literal header text may
        // reference typed inputs.
        if (!header.sensitive &&
            !validateEmbeddedPlaceholders(header.value, command.inputs,
                                           path + ".headers." + key, lastError_)) {
          commands.clear();
          return false;
        }
        command.headers.push_back(header);
      }
    }

    if (!item["body"].isNull()) {
      if (command.method == "GET") {
        lastError_ = path + ".body is only supported for POST";
        commands.clear();
        return false;
      }
      serializeJson(item["body"], command.bodyJson);
      if (!validateBodyPlaceholders(command.bodyJson, command.inputs, path + ".body",
                                     lastError_)) {
        commands.clear();
        return false;
      }
    }

    commands.push_back(command);
    ++index;
  }

  lastError_ = "";
  return true;
}

const String& ConfigLoader::lastError() const { return lastError_; }

}  // namespace cardputer_launcher
