#include "../include/route_builder.h"
#include "../include/api_handler.h"
#include <nlohmann/json.hpp>
#include <utility>
#include <vector>
#include <exception> 
#include <iostream>
#include <sstream>

RouteBuilder::RouteBuilder(ApiHandler& api) : api_(api) {}

void RouteBuilder::ParseThreadInfo(const nlohmann::json& j, Segment& seg) {
  auto thr = j.value("thread", nlohmann::json::object());
  seg.thread_title = thr.value("title", "");
  seg.thread_number = thr.value("number", "");
  seg.transport_type = thr.value("transport_type", "");
  auto carrier = thr.value("carrier", nlohmann::json::object());
  seg.carrier = carrier.value("title", "");
}

void RouteBuilder::ParseStations(const nlohmann::json& j, Segment& seg) {
  seg.from_title = GetNestedString(j, "from", "title");
  seg.to_title = GetNestedString(j, "to", "title");
  std::string dep_from = GetNestedString(j, "departure_from", "title");
  std::string arr_to = GetNestedString(j, "arrival_to", "title");
  if (!dep_from.empty()) {
    seg.from_title = dep_from;
  }
  if (!arr_to.empty()) {
    seg.to_title = arr_to;
  }
}

void RouteBuilder::ParseTimes(const nlohmann::json& j, Segment& seg) {
  seg.departure = GetJsonString(j, "departure");
  seg.arrival = GetJsonString(j, "arrival");
  seg.duration = j.value("duration", 0.0);
  seg.start_date = j.value("start_date", "");
}

void RouteBuilder::ParseTerminalsAndPlatforms(const nlohmann::json& j, Segment& seg) {
  seg.departure_terminal = GetJsonString(j, "departure_terminal");
  seg.arrival_terminal = GetJsonString(j, "arrival_terminal");
  seg.departure_platform = GetJsonString(j, "departure_platform");
  seg.arrival_platform = GetJsonString(j, "arrival_platform");
  seg.stops = GetJsonString(j, "stops");
}

void RouteBuilder::ParseTransferDetails(const nlohmann::json& j, Segment& seg) {
  if (!j.contains("details") || !j["details"].is_array()) {
    return;
  }
  double total_dur = 0;
  for (const auto& sub : j["details"]) {
    try {
      if (sub.contains("is_transfer") && sub["is_transfer"].is_boolean()) {
        if (sub["is_transfer"].get<bool>()) {
          continue;
        }
      }
      Segment sub_seg = ParseSegment(sub);
      total_dur += sub_seg.duration;
      seg.transfer_segments.push_back(std::move(sub_seg));
    } catch (...) {
    }
  }
  if (seg.duration <= 0 && total_dur > 0) {
    seg.duration = total_dur;
  }
}

Segment RouteBuilder::ParseSegment(const nlohmann::json& j) {
  Segment seg;
  ParseThreadInfo(j, seg);
  ParseStations(j, seg);
  ParseTimes(j, seg);
  ParseTerminalsAndPlatforms(j, seg);
  seg.has_transfers = j.contains("details") && j["details"].is_array() && !j["details"].empty();
  if (seg.has_transfers) {
    ParseTransferDetails(j, seg);
  }
  return seg;
}

std::string RouteBuilder::GetNestedString(const nlohmann::json& j, const std::string& key, const std::string& nested_key) {
  if (j.contains(key) && j[key].is_object()) {
    return j[key].value(nested_key, "");
  }
  return "";
}

std::string RouteBuilder::GetJsonString(const nlohmann::json& j, const std::string& key) {
  if (j.contains(key) && j[key].is_string()) {
    return j[key].get<std::string>();
  }
  return "";
}


std::vector<Segment> RouteBuilder::ParseRoutes(const nlohmann::json& data, int max_transfers) {
  std::vector<Segment> segs;
  nlohmann::json seg_array = nlohmann::json::array();
  if (data.contains("segments") && data["segments"].is_array()) {
    seg_array = data["segments"];
  }
  for (auto& s : seg_array) {
    try {
      Segment seg = ParseSegment(s);
      if (max_transfers == 0 && seg.has_transfers) {
        continue;
      }
      if (seg.has_transfers && !seg.transfer_segments.empty()) {
        int num_transfers = static_cast<int>(seg.transfer_segments.size()) - 1;
        if (num_transfers > max_transfers) {
          continue;
        }
      }
      segs.push_back(std::move(seg));
    } catch (const std::exception& ex) {
      std::cerr << "Failure during route parsing: " << ex.what() << '\n';
    }
  }
  return segs;
}

std::vector<Segment> RouteBuilder::FindRoutes(const std::string& from_code, const std::string& to_code, const std::string& date, int max_transfers) {
  nlohmann::json j = api_.Search(from_code, to_code, date, (max_transfers > 0) ? true : false);
  std::vector<Segment> segs = ParseRoutes(j, max_transfers);
  if (static_cast<int>(segs.size() > kMaxRes)) {
    segs.resize(kMaxRes);
  }
  return segs;
}

void RouteBuilder::PrintStationInfo(const std::string& station, const std::string& terminal, const std::string& platform, const std::string& indent) {
  std::cout << indent << station;
  if (!platform.empty()) {
    std::cout << ", платформа " << platform;
  }
  if (!terminal.empty()) {
    std::cout << ", терминал " << terminal;
  }
}

void RouteBuilder::PrintSingleSegment(const Segment& seg, const std::string& indent) {
  if (!seg.thread_number.empty()) {
    std::cout << indent << seg.thread_number;
  }
  if (!seg.carrier.empty()) {
    std::cout << "  " << seg.carrier;
  }
  std::cout << '\n';
  if (!seg.thread_title.empty()) {
    std::cout << indent << "Маршрут " << seg.thread_title << '\n';
  }
  PrintStationInfo(seg.from_title, seg.departure_terminal, seg.departure_platform, indent);
  std::cout << "  —  ";
  std::cout << seg.to_title;
  if (!seg.arrival_platform.empty()) {
    std::cout << ", платформа " << seg.arrival_platform;
  }
  if (!seg.arrival_terminal.empty()) {
    std::cout << ", терминал " << seg.arrival_terminal;
  }
  std::cout << '\n';
  std::cout << "Отправление: " << seg.departure.substr(11, 5) << " " << seg.departure.substr(0,10) << '\n';
  std::cout << "Прибытие: " << seg.arrival.substr(11, 5) << " " << seg.arrival.substr(0,10) << '\n';
  std::string dur = DurToTime(seg.duration);
  if (!dur.empty()) {
    std::cout << indent << dur << "в пути\n";
  }
  if (!seg.stops.empty()) {
    std::cout << indent << "Остановки: " << seg.stops << '\n';
  }
}

void RouteBuilder::PrintDirectRoute(const Segment& seg) {
  std::cout << RusTransport(seg.transport_type);
  if (!seg.thread_number.empty()) {
    std::cout << " " << seg.thread_number;
  }
  if (!seg.carrier.empty()) {
    std::cout << " " << seg.carrier;
  }
  std::cout << '\n';
  if (!seg.thread_title.empty()) {
    std::cout << "Маршрут " << seg.thread_title << '\n';
  }
  PrintStationInfo(seg.from_title, seg.departure_terminal, seg.departure_platform, "");
  std::cout << "  —  ";
  std::cout << seg.to_title;
  if (!seg.arrival_platform.empty()) {
    std::cout << ", платформа " << seg.arrival_platform;
  }
  if (!seg.arrival_terminal.empty()) {
    std::cout << ", терминал " << seg.arrival_terminal;
  }
  std::cout << '\n';
  std::cout << "Отправление: " << seg.departure.substr(11, 5) << " " << seg.departure.substr(0,10) << '\n';
  std::cout << "Прибытие: " << seg.arrival.substr(11, 5) << " " << seg.arrival.substr(0,10) << '\n';
  std::string dur = DurToTime(seg.duration);
  if (!dur.empty()) {
    std::cout << dur << "в пути\n";
  }
  if (!seg.stops.empty()) {
    std::cout << "Остановки: " << seg.stops << '\n';
  }
}

void RouteBuilder::PrintTransferRoute(const Segment& seg) {
  int num_transfers = static_cast<int>(seg.transfer_segments.size()) - 1;
  std::cout << "С количеством пересадок: " << std::max(0, num_transfers) << '\n';
  std::cout << "Общий маршрут: " << seg.from_title << "  --->  " << seg.to_title << '\n';
  std::cout << "Отправление: " << seg.departure.substr(11, 5) << " " << seg.departure.substr(0,10) << '\n';
  std::cout << "Прибытие: " << seg.arrival.substr(11, 5) << " " << seg.arrival.substr(0,10) << '\n';
  std::string dur = DurToTime(seg.duration);
  if (!dur.empty()) {
    std::cout << dur << "в пути\n";
  }
  int sub_count = 1;
  for (const auto& sub : seg.transfer_segments) {
    std::cout << "Участок " << sub_count++ << '\n';
    PrintSingleSegment(sub, "  ");
  }
}

void RouteBuilder::PrintRoutes(const std::vector<Segment>& segs, int limit) {
  int to_print = (limit > 0 && limit < segs.size()) ? limit : segs.size();
  std::cout << "Найдено маршрутов: " << segs.size();
  if (to_print < segs.size()) {
    std::cout << " (показано первых " << to_print << ")";
  }
  std::cout << '\n';
  int count = 1;
  for (int i = 0; i < to_print; ++i) {
    const auto& seg = segs[i];
    std::cout << '\n' << (i + 1) << ". ";
    if (seg.has_transfers && !seg.transfer_segments.empty()) {
      PrintTransferRoute(seg);
    } else {
      PrintDirectRoute(seg);
    }
  }
}

void RouteBuilder::MakeAndPrint(const std::string& from_city, const std::string& to_city, const std::string& date, int max_transfers, int limit) {
  std::string from_code = api_.TakeCity(from_city);
  std::string to_code = api_.TakeCity(to_city);
  std::cout << from_city << " ----> " << to_city << "   " << date;
  if (max_transfers > 0) {
    std::cout << "   с количеством пересадок не более чем " << max_transfers;
  } else {
    std::cout << "   без пересадок";
  }
  std::cout << '\n';
  auto route = FindRoutes(from_code, to_code, date, max_transfers);
  PrintRoutes(route, limit);
}

std::string RouteBuilder::DurToTime(double dur) {
  if (dur <= 0) {
    return "";
  }
  int total_seconds = static_cast<int>(dur);
  int hours = (total_seconds / 3600);
  int min = (total_seconds % 3600) / 60;
  std::ostringstream oss;
  if (hours > 0) {
    oss << hours << "ч. ";
  }
  oss << min << "мин. ";
  return oss.str();
}

std::string RouteBuilder::RusTransport(const std::string& t) {
  if (t == "plane") {
    return "Самолет";
  }
  if (t == "train") {
    return "Поезд";
  }
  if (t == "suburban") {
    return "Электричка";
  }
  if (t == "bus") {
    return "Автобус";
  }
  if (t == "water") {
    return "Водный транспорт";
  }
  if (t == "helicopter") {
    return "Вертолет";
  }
  return t;
}