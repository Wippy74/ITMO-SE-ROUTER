#pragma once

#include "../include/api_handler.h"
#include <set>
#include <vector>

struct Route {
  std::string departure;
  std::string arrival;
  std::string from_title;
  std::string to_title;
  std::vector<Segment> segs;
  std::vector<double> wait_seconds;
  double total_duration = 0;
  bool has_transfers = false;
};

class RouteBuilder {
public:
  explicit RouteBuilder(ApiHandler& api);
private:
  ApiHandler& api_;

  std::vector<Segment> FindRoutes(const std::string& from, const std::string& to, const std::string& date, int MaxTransfers);

  Segment ParseSegment(const nlohmann::json& seg);
  Route ParseDirect(const nlohmann::json& seg);
  Route ParseTransfers(const nlohmann::json& seg);
  static constexpr int kMaxRes = 100;
};