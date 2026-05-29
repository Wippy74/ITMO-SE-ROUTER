#pragma once

#include <string>
#include <map>

struct Response {
  int status_code = 0;
  std::string text;
  std::string error_message;
};

class HttpClientInterface {
public:
  virtual ~HttpClientInterface() = default;
  virtual Response Get(const std::string& URL, const std::map<std::string, std::string>& params, int timeout = 30000) = 0;
};

class HttpClient : public HttpClientInterface {
public:
  Response Get(const std::string& URL, const std::map<std::string, std::string>& params, int timeout = 30000) override;
};