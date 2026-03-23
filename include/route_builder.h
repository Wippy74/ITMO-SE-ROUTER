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
  std::vector<Segment> transfer_segments;
};

class RouteBuilder {
public:
  explicit RouteBuilder(ApiHandler& api);
  void MakeAndPrint(const std::string& from_city, const std::string& to_city, const std::string& date, int max_transfers, int limit);
private:
  static constexpr int kMaxRes = 100;
  ApiHandler& api_;

  std::vector<Segment> FindRoutes(const std::string& from_code, const std::string& to_code, const std::string& date, int max_transfers);

  void PrintRoutes(const std::vector<Segment>& segs, int limit);

  std::vector<Segment> ParseRoutes(const nlohmann::json& data, int max_transfers);
  Segment ParseSegment(const nlohmann::json& j);
  void PrintStationInfo(const std::string& station, const std::string& terminal, const std::string& platform, const std::string& indent);
  void PrintSingleSegment(const Segment& seg, const std::string& indent);
  void PrintDirectRoute(const Segment& seg);
  void PrintTransferRoute(const Segment& seg);

  void ParseThreadInfo(const nlohmann::json& j, Segment& seg);
  void ParseStations(const nlohmann::json& j, Segment& seg);
  void ParseTimes(const nlohmann::json& j, Segment& seg);
  void ParseTerminalsAndPlatforms(const nlohmann::json& j, Segment& seg);
  void ParseTransferDetails(const nlohmann::json& j, Segment& seg);

  std::string GetJsonString(const nlohmann::json& j, const std::string& key);
  std::string GetNestedString(const nlohmann::json& j, const std::string& key, const std::string& nested_key);

  std::string RusTransport(const std::string& t);
  std::string DurToTime(double dur);
};