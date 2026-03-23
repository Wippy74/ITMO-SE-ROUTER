#pragma once

#include <nlohmann/json.hpp>
#include "cache.h"
#include "http_interface.h"
#include <vector>
#include <map>
#include <stdexcept>
#include <string>
#include <chrono>
#include <memory>

class ApiErr : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

class ApiHandler {
public:
  ApiHandler(const std::string& ApiKey, Cache& cache);
  ApiHandler(const std::string& ApiKey, Cache& cache, std::shared_ptr<HttpClientInterface> http_client);

  nlohmann::json Search(const std::string& from, const std::string& to, const std::string& date, bool transfers = false);
  std::string TakeCity(const std::string& city);
  size_t GetCityIndexSize() const {
    return city_idx_.size();
  }
private:
  static constexpr const char* base_url = "https://api.rasp.yandex-net.ru/v3.0";
  static constexpr std::chrono::hours station_list_ttl_ = std::chrono::hours(96);
  std::chrono::system_clock::time_point StationListLoaded_{};
  
  std::string api_key_;
  Cache& cache_;
  std::shared_ptr<HttpClientInterface> http_client_;
  
  std::unordered_map<std::string, std::string> city_idx_;

  void EnsureStationList();
  void BuildCityIdx(const nlohmann::json& data);
  nlohmann::json HttpGet(const std::string& endpoint, const std::map<std::string, std::string>& params);
  std::string MakeCacheKey(const std::string& endpoint, const std::map<std::string, std::string>& params);
};