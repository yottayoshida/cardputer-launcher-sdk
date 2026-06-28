// SPDX-License-Identifier: MIT OR Apache-2.0

#include "network/HttpClient.h"

#include <HTTPClient.h>

namespace cardputer_launcher {

namespace {

String previewOf(const String& value) {
  if (value.length() <= 160) {
    return value;
  }
  return value.substring(0, 160) + "...";
}

}  // namespace

HttpResponse HttpClient::send(const HttpRequest& request) {
  HttpResponse response;
  HTTPClient http;

  if (!request.url.startsWith("https://") && !request.url.startsWith("http://")) {
    response.error = "unsupported URL";
    return response;
  }

  if (!http.begin(request.url)) {
    response.error = "HTTP begin failed";
    return response;
  }

  for (const Header& header : request.headers) {
    http.addHeader(header.name, header.value);
  }

  int status = 0;
  if (request.method == "POST") {
    status = http.POST(request.body);
  } else {
    status = http.GET();
  }

  response.statusCode = status;
  if (status <= 0) {
    response.error = http.errorToString(status);
    http.end();
    return response;
  }

  response.preview = previewOf(http.getString());
  response.ok = status >= 200 && status < 300;
  if (!response.ok) {
    response.error = "HTTP " + String(status);
  }
  http.end();
  return response;
}

}  // namespace cardputer_launcher

