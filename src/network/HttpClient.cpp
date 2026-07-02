// SPDX-License-Identifier: MIT OR Apache-2.0

#include "network/HttpClient.h"

#include <HTTPClient.h>

namespace cardputer_launcher {

namespace {

constexpr uint16_t kConnectTimeoutMs = 5000;
constexpr uint16_t kReadTimeoutMs = 5000;
constexpr size_t kMaxRequestBodyBytes = 2048;
constexpr size_t kMaxResponsePreviewBytes = 160;
constexpr uint8_t kGetRetryCount = 1;

String hostOf(const String& url) {
  const int schemeEnd = url.indexOf("://");
  if (schemeEnd < 0) {
    return "";
  }
  int start = schemeEnd + 3;
  if (start >= static_cast<int>(url.length())) {
    return "";
  }
  if (url[start] == '[') {
    const int end = url.indexOf(']', start + 1);
    if (end < 0) {
      return "";
    }
    return url.substring(start + 1, end);
  }

  int end = url.length();
  for (int index = start; index < static_cast<int>(url.length()); ++index) {
    const char c = url[index];
    if (c == ':' || c == '/' || c == '?' || c == '#') {
      end = index;
      break;
    }
  }
  return url.substring(start, end);
}

bool isLoopbackHost(const String& host) {
  return host == "localhost" || host == "127.0.0.1" || host == "::1";
}

bool isAllowedUrl(const String& url, bool allowLocalHttp) {
  if (url.startsWith("https://")) {
    return true;
  }
  if (!url.startsWith("http://")) {
    return false;
  }
  return allowLocalHttp && isLoopbackHost(hostOf(url));
}

String previewOf(const String& value) {
  if (value.length() <= kMaxResponsePreviewBytes) {
    return value;
  }
  return value.substring(0, kMaxResponsePreviewBytes) + "...";
}

HttpErrorKind classifyTransportError(const String& error) {
  String lowered = error;
  lowered.toLowerCase();
  if (lowered.indexOf("timeout") >= 0 || lowered.indexOf("timed out") >= 0) {
    return HttpErrorKind::Timeout;
  }
  return HttpErrorKind::Network;
}

int sendOnce(HTTPClient& http, const HttpRequest& request) {
  if (request.method == "POST") {
    return http.POST(request.body);
  }
  return http.GET();
}

}  // namespace

HttpResponse HttpClient::send(const HttpRequest& request) {
  HttpResponse response;

  if (!isAllowedUrl(request.url, request.allowLocalHttp)) {
    response.errorKind = HttpErrorKind::Policy;
    response.error = "URL policy blocked";
    return response;
  }

  if (request.method == "POST" && request.body.length() > kMaxRequestBodyBytes) {
    response.errorKind = HttpErrorKind::Limit;
    response.error = "request body too large";
    return response;
  }

  const uint8_t retries = request.method == "GET" ? kGetRetryCount : 0;
  for (uint8_t attempt = 0; attempt <= retries; ++attempt) {
    HTTPClient http;
    http.setConnectTimeout(kConnectTimeoutMs);
    http.setTimeout(kReadTimeoutMs);

    if (!http.begin(request.url)) {
      response.errorKind = HttpErrorKind::Network;
      response.error = "HTTP begin failed";
      return response;
    }

    for (const Header& header : request.headers) {
      http.addHeader(header.name, header.value);
    }

    const int status = sendOnce(http, request);
    response.statusCode = status;
    if (status <= 0) {
      const String errorText = http.errorToString(status);
      http.end();
      if (attempt < retries) {
        continue;
      }
      response.errorKind = classifyTransportError(errorText);
      response.error = errorText;
      return response;
    }

    response.preview = previewOf(http.getString());
    response.ok = status >= 200 && status < 300;
    if (!response.ok) {
      response.errorKind = HttpErrorKind::HttpStatus;
      response.error = "HTTP " + String(status);
    }
    http.end();
    return response;
  }

  response.errorKind = HttpErrorKind::Network;
  response.error = "HTTP request failed";
  return response;
}

}  // namespace cardputer_launcher
