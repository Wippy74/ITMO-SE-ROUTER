#pragma once

#include "../include/api_handler.h"
#include <set>
#include <vector>

struct MultiRoute {
  std::vector<Segment> segments;
  double total_seconds = 0;
};

class RouteBuilder {
public:
  explicit RouteBuilder(ApiHandler& api);
private:
  ApiHandler& api_;

  std::vector<Segment> FindRoute(const std::string& from, const std::string& to, const std::string& date, int MaxTransfers);

  struct SearchStorage {
    std::string dest;
    std::vector<Segment> path;
    std::set<std::string> visited;
    std::vector<MultiRoute> results;
  };

  void DFS(const std::string& CurrentCode, const std::string& DestCode, const std::string& date, int RemEdges, long long PrevTime, SearchStorage& st);
  void TryDirect(const std::string& CurrentCode, const std::string& SearchDate, long long PrevTime, SearchStorage& st);
  void TryTransfer(const std::string& CurrentCode, const std::string& SearchDate, long long PrevTime, SearchStorage& st);

  static constexpr int kMaxRes = 1000;
};