#pragma once

#include <nlohmann/json.hpp>
#include "cache.h"
#include <vector>
#include <map>
#include <stdexcept>
#include <string>

struct Segment {
  std::string from;
  std::string thread_title;
  std::string thread_number;
  std::string carrier;
  std::string transport_type;
  std::string to;
  std::string departure;
  std::string arrival;
  double duration;
};

class ApiHandler {
public:
  ApiHandler(const std::string& ApiKey, Cache& cache);

  std::vector<Segment> search(const std::string& from, const std::string& to, const std::string& date, const std::string& transport = "");
private:
  static constexpr const char* BaseURL = "https://api.rasp.yandex-net.ru";
  std::string ApiKey_;
  Cache& cache_;
  nlohmann::json HttpGet(const std::string& endpoint, const std::map<std::string, std::string>& params);
  std::string MakeCacheKey(const std::string& endpoint, const std::map<std::string, std::string>& params);
  Segment ParseSegment(const nlohmann::json& j);
};