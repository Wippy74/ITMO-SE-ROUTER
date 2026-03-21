#pragma once

#include "../include/api_handler.h"
#include <set>
#include <vector>

struct Segment {
  std::string thread_title;
  std::string thread_number;
  std::string transport_type;
  std::string carrier;
  std::string from_title;
  std::string to_title;
  std::string departure;
  std::string arrival;
  std::string stops;
  std::string departure_terminal;
  std::string arrival_terminal;
  std::string departure_platform;
  std::string arrival_platform;
  std::string start_date;
  double duration = 0;
  bool has_transfers = false;
};

class RouteBuilder {
public:
  explicit RouteBuilder(ApiHandler& api);
private:
  static constexpr int kMaxRes = 100;
  ApiHandler& api_;

  std::vector<Segment> FindRoutes(const std::string& from, const std::string& to, const std::string& date, int MaxTransfers);

  void PrintRoutes(const std::vector<Segment>& segs);

  std::vector<Segment> ParseRoutes(const nlohmann::json& data, int max_transfers);
  Segment ParseSegment(const nlohmann::json& j);

  std::string RusTransport(const std::string& t);
};