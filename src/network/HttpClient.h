#pragma once

// SPDX-License-Identifier: MIT OR Apache-2.0

#include <Arduino.h>
#include <vector>

#include "storage/ConfigLoader.h"

namespace cardputer_launcher {

struct HttpRequest {
  String method;
  String url;
  std::vector<Header> headers;
  String body;
};

struct HttpResponse {
  bool ok = false;
  int statusCode = 0;
  String error;
  String preview;
};

class HttpClient {
 public:
  HttpResponse send(const HttpRequest& request);
};

}  // namespace cardputer_launcher

