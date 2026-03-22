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

std::vector<Segment> RouteBuilder::FindRoutes(const std::string& FromCode, const std::string& ToCode, const std::string& date, int MaxTransfers) {
  nlohmann::json j = api_.Search(FromCode, ToCode, date, (MaxTransfers > 0) ? true : false);
  std::vector<Segment> segs = ParseRoutes(j, MaxTransfers);
  if (static_cast<int>(segs.size() > kMaxRes)) {
    segs.resize(kMaxRes);
  }
  return segs;
}

void RouteBuilder::PrintRoutes(const std::vector<Segment>& segs) {
  std::cout << "Найденно маршрутов: " << segs.size() << '\n';
  int count = 1;
  for (auto& s : segs) {
    std::cout << '\n' << count++ << ". " << RusTransport(s.transport_type) << s.thread_number;
    if (!s.carrier.empty()) {
      std::cout << s.carrier;
    }
    if (s.has_transfers) {
      std::cout << " с пересадками ";
    }
    std::cout << '\n';
    std::cout << s.thread_title << '\n';
    std::cout << s.from_title << '\n';
    if (!s.departure_platform.empty()) {
      std::cout << ", терминал" << s.departure_platform;
    }
    if (!s.departure_terminal.empty()) {
      std::cout << ", платформа" << s.departure_terminal;
    }
    std::cout << '\n';
    std::cout << s.to_title << '\n';
    if (!s.arrival_platform.empty()) {
      std::cout << ", терминал" << s.arrival_platform;
    }
    if (!s.arrival_terminal.empty()) {
      std::cout << ", платформа" << s.arrival_terminal;
    }
    std::cout << '\n';
    std::cout << "Отправление: " << s.departure.substr(11, 5);
    std::cout << "Прибытие: " << s.arrival.substr(11, 5);
    std::cout << s.arrival.substr(0,10) << '\n';
    std::cout << DurToTime(s.duration) << "в пути" << '\n';
    if (!s.stops.empty()) {
      std::cout << "Остановки: " << s.stops << '\n';
    }
  }
}

void RouteBuilder::MakeAndPrint(const std::string& FromCity, const std::string& ToCity, const std::string& date, int MaxTransfers) {
  std::string FromCode = api_.TakeCity(FromCity);
  std::string ToCode = api_.TakeCity(ToCity);
  std::cout << FromCity << " ----> " << ToCity << "   " << date;
  if (MaxTransfers > 0) {
    std::cout << "   с количеством пересадок не более чем " << MaxTransfers;
  } else {
    std::cout << "   без пересадок";
  }
  std::cout << '\n';
  auto route = FindRoutes(FromCode, ToCode, date, MaxTransfers);
  PrintRoutes(route);
}

std::string RouteBuilder::DurToTime(double dur) {
  if (dur <= 0) {
    return "0";
  }
  int min = (static_cast<int>(dur) / 60) % 60;
  int hours = min / 60;
  std::ostringstream oss;
  if (hours > 0) {
    oss << hours << "ч.";
  }
  oss << min << "мин.";
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
}
