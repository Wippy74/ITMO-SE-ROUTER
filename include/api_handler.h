#pragma once

#include <nlohmann/json.hpp>
#include "cache.h"
#include <vector>
#include <map>
#include <stdexcept>
#include <string>
#include <chrono>

struct Segment {
  std::string from_title;
  std::string thread_title;
  std::string thread_number;
  std::string carrier;
  std::string transport_type;
  std::string to_title;
  std::string departure;
  std::string arrival;
  double duration;
};

class ApiHandler {
public:
  ApiHandler(const std::string& ApiKey, Cache& cache);

  nlohmann::json Search(const std::string& from, const std::string& to, const std::string& date, bool transfers = false);
private:
  static constexpr const char* BaseURL = "https://api.rasp.yandex-net.ru";
  static constexpr std::chrono::hours StationListTTL_ = std::chrono::hours(96);
  std::chrono::system_clock::time_point StationListLoaded_{};
  
  std::string ApiKey_;
  Cache& cache_;
  
  std::unordered_map<std::string, std::string> CityIdx_;

  void EnsureStationList();
  void BuildCityIdx(const nlohmann::json& data);
  nlohmann::json HttpGet(const std::string& endpoint, const std::map<std::string, std::string>& params);
  std::string MakeCacheKey(const std::string& endpoint, const std::map<std::string, std::string>& params);
  Segment ParseSegment(const nlohmann::json& j);
};