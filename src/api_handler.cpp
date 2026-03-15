#include "../include/api_handler.h"

#include <cpr/cpr.h>

nlohmann::json ApiHandler::HttpGet(const std::string& endpoint, const std::map<std::string, std::string>& params) {
  std::string key = MakeCacheKey(endpoint, params);
  if (auto in_cache = cache_.get(key)) {
    nlohmann::json::parse(*in_cache);
  }
  std::string total_url = std::string(BaseURL) + endpoint;
  cpr::Parameters CprParams;
  CprParams.Add({"apikey", ApiKey_});
  for (const auto& [Key, Value] : params) {
    CprParams.Add({Key, Value});
  }
  cpr::Response r = cpr::Get(cpr::Url{total_url}, CprParams, cpr::Timeout{150000});
  if (r.status_code == 0) {
    throw std::runtime_error("Нет такой сети: " + r.error.message);
  }
  if (r.status_code == 400) {
    throw std::runtime_error("Bad request");
  }
  if (r.status_code == 403) {
    throw std::runtime_error("Неверный ключ API");
  }
  if (r.status_code == 404) {
    throw std::runtime_error("Not Found");
  }
  if (r.status_code != 200) {
    throw std::runtime_error(std::to_string(r.status_code));
  }
  cache_.put(key, r.text);
  return nlohmann::json::parse(r.text);
}

Segment ApiHandler::ParseSegment(const nlohmann::json& j) {
  Segment seg;
  seg.from = j.value("from", nlohmann::json::object().value("title", ""));
  auto t = j.value("thread", nlohmann::json::object());
  seg.thread_title = t.value("title", "");
  seg.thread_number = t.value("number", "");
  auto c = j.value("carrier", nlohmann::json::object());
  seg.carrier = c.value("title", "");
  seg.transport_type = t.value("transport_type", "");
  seg.to = j.value("to", nlohmann::json::object().value("title", ""));
  if (j.contains("departure") && j["departure"].is_string()) {
    seg.departure = j["departure"].get<std::string>();
  }
  if (j.contains("arrival") && j["arrival"].is_string()) {
    seg.arrival = j["arrival"].get<std::string>();
  }
  seg.duration = j.value("duration", 0.0);
  return seg;
}

std::vector<Segment> ApiHandler::search(const std::string& from, const std::string& to, const std::string& date, const std::string& transport_type = "") {
  std::map<std::string, std::string> params = {{"from", from}, {"to", to}, {"date", date}, {"lang", "ru_RU"} , {"format", "json"} , {"limit", "100"}, {"transfers", "false"}, {"transport_types", transport_type}};
  nlohmann::json data; 
  try {
    data = HttpGet("/v3.0/search", params);
  } catch (const std::exception& ex) {
    std::cerr << "Ошибка при поиске маршрута из " << from << " в " << to << ": " << ex.what() << '\n';
    return {};
  }
  std::vector<Segment> res;
  for (auto& s : data.value("segments", nlohmann::json::array())) {
    try {
      auto parsed_s = ParseSegment(s);
      if (!parsed_s.from.empty() && !parsed_s.to.empty()) {
        res.push_back(parsed_s);
      }
    } catch (...) {

    }
  }
  return res;
}