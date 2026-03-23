#include "../include/route_builder.h"

RouteBuilder::RouteBuilder(ApiHandler& api) : api_(api) {}

Segment RouteBuilder::ParseSegment(const nlohmann::json& j) {
  Segment r;
  auto thr = j.value("thread", nlohmann::json::object());
  r.thread_title = thr.value("title", "");
  r.thread_number = thr.value("number", "");
  r.transport_type = thr.value("transport_type", "");
  auto car = thr.value("carrier", nlohmann::json::object());
  r.carrier = car.value("title", "");
  if (j.contains("from") && j["from"].is_object()) {
    r.from_title = j["from"].value("title", "");
  }
  if (j.contains("to") && j["to"].is_object()) {
    r.to_title = j["to"].value("title", "");
  }
  if (j.contains("departure_from") && j["departure_from"].is_object()) {
    std::string dep_title = j["departure_from"].value("title", "");
    if (!dep_title.empty()) {
      r.from_title = dep_title;
    }
  }
  if (j.contains("arrival_to") && j["arrival_to"].is_object()) {
    std::string arr_title = j["arrival_to"].value("title", "");
    if (!arr_title.empty()) {
      r.to_title = arr_title;
    }
  }
  if (j.contains("departure") && j["departure"].is_string()) {
    r.departure = j["departure"].get<std::string>();
  }
  if (j.contains("arrival") && j["arrival"].is_string()) {
    r.arrival = j["arrival"].get<std::string>();
  }
  r.duration = j.value("duration", 0.0);
  r.has_transfers = j.contains("details") && j["details"].is_array() && !j["details"].empty();
  if (j.contains("stops") && j["stops"].is_string()) {
    r.stops = j["stops"].get<std::string>();
  }
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
  if (r.has_transfers) {
    double total_dur = 0;
    for (auto& sub : j["details"]) {
      try {
        bool is_transfer_wait = false;
        if (sub.contains("is_transfer") && sub["is_transfer"].is_boolean()) {
          is_transfer_wait = sub["is_transfer"].get<bool>();
        }
        if (is_transfer_wait) {
          continue;
        }
        Segment sub_seg = ParseSegment(sub);
        total_dur += sub_seg.duration;
        r.transfer_segments.push_back(std::move(sub_seg));
      } catch (...) {}
    }
    if (r.duration <= 0 && total_dur > 0) {
      r.duration = total_dur;
    }
  }
  return r;
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

void RouteBuilder::PrintRoutes(const std::vector<Segment>& segs, int limit) {
  int to_print = (limit > 0 && limit < segs.size()) ? limit : segs.size();
  std::cout << "Найдено маршрутов: " << segs.size();
  if (to_print < segs.size()) {
    std::cout << " (показано первых " << to_print << ")";
  }
  std::cout << '\n';
  int count = 1;
  for (int i = 0; i < to_print; ++i) {
    auto& s = segs[i];
    std::cout << '\n' << count++ << ". ";
    if (s.has_transfers && !s.transfer_segments.empty()) {
      std::cout << "С количеством пересадок: " << s.transfer_segments.size() << '\n';
      std::cout << "Общий маршрут: " << s.from_title << " ---> " << s.to_title << '\n';
      std::cout << "Отправление: " << s.departure.substr(11, 5) << " " << s.departure.substr(0,10) << '\n';
      std::cout << "Прибытие: " << s.arrival.substr(11, 5) << " " << s.arrival.substr(0,10) << '\n';
      std::cout << DurToTime(s.duration) << "в пути" << '\n';
      int SubCount = 1;
      for (auto& sub : s.transfer_segments) {
        std::cout << "Участок " << SubCount++ << '\n';
        if (!sub.thread_number.empty()) {
          std::cout << "  " << sub.thread_number;
        }
        if (!sub.carrier.empty()) {
          std::cout << "  " << sub.carrier;
        }
        std::cout << '\n';
        if (!sub.thread_title.empty()) {
          std::cout << "  Маршрут " << sub.thread_title << '\n';
        }
        std::cout << "  " << sub.from_title;
        if (!sub.departure_platform.empty()) {
          std::cout << ", платформа" << sub.departure_platform;
        }
        if (!sub.departure_terminal.empty()) {
          std::cout << ", терминал" << sub.departure_terminal;
        }
        std::cout << "  —  ";
        std::cout << sub.to_title;
        if (!sub.arrival_terminal.empty()) {
          std::cout << ", терминал " << sub.arrival_terminal;
        }
        if (!sub.arrival_platform.empty()) {
          std::cout << ", платформа " << sub.arrival_platform;
        }
        std::cout << '\n';
        std::cout << "  Отправление: " << sub.departure.substr(11, 5) << " " << sub.departure.substr(0,10) << '\n';
        std::cout << "  Прибытие: " << sub.arrival.substr(11, 5) << " " << sub.arrival.substr(0,10) << '\n';
        std::cout << "  " << DurToTime(sub.duration) << "в пути" << '\n';
        if (!s.stops.empty()) {
          std::cout << "  Остановки: " << sub.stops << '\n';
        }
      }
    } else {
      std::cout << RusTransport(s.transport_type);
      if (!s.thread_number.empty()) {
        std::cout << " " << s.thread_number;
      }
      if (!s.carrier.empty()) {
        std::cout << " " << s.carrier;
      }
      std::cout << '\n';
      if (!s.thread_title.empty()) {
        std::cout << "Маршрут " << s.thread_title << '\n';
      }
      std::cout << s.from_title;
      if (!s.departure_platform.empty()) {
        std::cout << ", платформа" << s.departure_platform;
      }
      if (!s.departure_terminal.empty()) {
        std::cout << ", терминал" << s.departure_terminal;
      }
      std::cout << "  —  ";
      std::cout << s.to_title;
      if (!s.arrival_platform.empty()) {
        std::cout << ", платформа" << s.arrival_platform;
      }
      if (!s.arrival_terminal.empty()) {
        std::cout << ", терминал " << s.arrival_terminal;
      }
      std::cout << '\n';
      std::cout << "Отправление: " << s.departure.substr(11, 5) << " " << s.departure.substr(0,10) << '\n';
      std::cout << "Прибытие: " << s.arrival.substr(11, 5) << " " << s.arrival.substr(0,10) << '\n';
      std::cout << DurToTime(s.duration) << "в пути" << '\n';
      if (!s.stops.empty()) {
        std::cout << "Остановки: " << s.stops << '\n';
      }
    }
  }
}

void RouteBuilder::MakeAndPrint(const std::string& from_city, const std::string& to_city, const std::string& date, int max_transfers, int limit) {
  std::string FromCode = api_.TakeCity(from_city);
  std::string ToCode = api_.TakeCity(to_city);
  std::cout << from_city << " ----> " << to_city << "   " << date;
  if (max_transfers > 0) {
    std::cout << "   с количеством пересадок не более чем " << max_transfers;
  } else {
    std::cout << "   без пересадок";
  }
  std::cout << '\n';
  auto route = FindRoutes(FromCode, ToCode, date, max_transfers);
  PrintRoutes(route, limit);
}

std::string RouteBuilder::DurToTime(double dur) {
  if (dur <= 0) {
    return "";
  }
  int TotalSeconds = static_cast<int>(dur);
  int hours = (TotalSeconds / 3600);
  int min = (TotalSeconds % 3600) / 60;
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