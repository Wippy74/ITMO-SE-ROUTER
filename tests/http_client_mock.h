#pragma once

#include <gmock/gmock.h>
#include "../include/http_interface.h"

class MockHttpClient : public HttpClientInterface {
public:
  MOCK_METHOD(Response, Get, (const std::string& URL, (const std::map<std::string, std::string>&) params, int timeout), (override));
};