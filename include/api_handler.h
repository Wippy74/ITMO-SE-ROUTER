#pragma once

#include <nlohmann/json.hpp>
#include "cache.h"
#include <vector>
#include <map>
#include <stdexcept>
#include <string>

struct Thread {
  std::string uid;
  std::string title;
  std::string number;
  std::string short_title;
  std::string thread_method_link;
  std::string carrier;
  std::string transport_type;
  std::string vehicle;
  std::string transport_subtype;
  std::string express_type;
  std::string from;
  std::string to;
  double duration;
};

class ApiHandler {
public:
  ApiHandler(const std::string& ApiKey, Cache& cache);

  std::vector<Thread> search(const std::string& from, const std::string& to, const std::string& date, const std::string& transport = "");
private:
  std::string ApiKey_;
  Cache& cache_;
  nlohmann::json HttpGet(const std::string& endpoint, const std::map<std::string, std::string>& params);
  std::string MakeCacheKey(const std::string& endpoint, const std::map<std::string, std::string>& params);
  Thread ParseThread(const nlohmann::json& j);
};