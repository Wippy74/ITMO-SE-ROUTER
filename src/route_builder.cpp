#include <../include/route_builder.h>

RouteBuilder::RouteBuilder(ApiHandler& api) : api_(api) {}

Segment RouteBuilder::ParseSegment(const nlohmann::json& j) {
  Segment r;
  auto thr = j.value("thread", nlohmann::json::object());
  r.thread_title = thr.value("title", "");
  r.thread_number = thr.value("number", "");
  r.transport_type = thr.value("transport_type", "");
  auto car = thr.value("carrier", nlohmann::json::object());
  r.carrier = car.value("title", "");
  r.from_title = j.value("from", nlohmann::json::object()).value("title", "");
  r.to_title = j.value("to", nlohmann::json::object()).value("title", "");
  if (j.contains("departure") && j["departure"].is_string()) {
    r.departure = j["departure"].get<std::string>();
  }
  if (j.contains("arrival") && j["arrival"].is_string()) {
    r.arrival = j["arrival"].get<std::string>();
  }
  r.duration = j.value("duration", 0.0);
  r.has_transfers = j.value("has_transfers", false);
  r.stops = j.value("stops", "");
  if (j.contains("departure_terminal") && j["departure_terminal"].is_string()) {
    r.departure_terminal = j["departure_terminal"].get<std::string>();
  }
  if (j.contains("arrival_terminal") && j["arrival_terminal"].is_string()) {
    r.arrival_terminal = j["arrival_terminal"].get<std::string>();
  }
  if (j.contains("departure_platform") && j["departure_platform"].is_string()) {
    r.departure_platform = j["departure_platform"].get<std::string>();
  }
  if (j.contains("arrival_platform") && j["arrival_platform"].is_string()) {
    r.arrival_platform = j["arrival_platform"].get<std::string>();
  }
  r.start_date = j.value("start_date", "");
  return r;
}

std::vector<Segment> RouteBuilder::ParseRoutes(const nlohmann::json& data, int MaxTransfers) {
  std::vector<Segment> segs;
  for (auto& s : data.value("segments", nlohmann::json::array())) {
    try {
      Segment seg = ParseSegment(s);
      if (MaxTransfers == 0 && seg.has_transfers) {
        continue;
      }
      segs.push_back(std::move(seg));
    } catch (const std::exception& ex) {
      std::cerr << "Failure during route parsing: " << ex.what() << '\n';
    }
  }
  return segs;
}
