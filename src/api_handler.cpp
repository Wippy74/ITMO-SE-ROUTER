#include "../include/api_handler.h"

#include <cpr/cpr.h>

ApiHandler::ApiHandler(const std::string& ApiKey, Cache& cache) : ApiKey_(ApiKey), cache_(cache) {};

std::string ApiHandler::TakeCity(const std::string& city) {
  EnsureStationList();
  auto it = CityIdx_.find(city);
  if (it != CityIdx_.end()) {
    return it->second;
  }
  throw ApiErr("Город " + city + " не найден");
}

void ApiHandler::EnsureStationList() {
  auto now = std::chrono::system_clock::now();
  if (!CityIdx_.empty() && ((now - StationListLoaded_) < StationListTTL_)) {
    return;
  }
  try {
    nlohmann::json data = HttpGet("/stations_list", {{"lang", "ru_RU"}, {"format", "json"}});
    BuildCityIdx(data);
    StationListLoaded_ = std::chrono::system_clock::now();
  } catch (const std::exception& ex) {
    std::cerr << "Stations list error: " << ex.what() << '\n';
  }
}

nlohmann::json ApiHandler::HttpGet(const std::string& endpoint, const std::map<std::string, std::string>& params) {
  std::string key = MakeCacheKey(endpoint, params);
  if (auto in_cache = cache_.get(key)) {
    return nlohmann::json::parse(*in_cache);
  }
  std::string total_url = std::string(BaseURL) + endpoint;
  cpr::Parameters CprParams;
  CprParams.Add({"apikey", ApiKey_});
  for (const auto& [Key, Value] : params) {
    CprParams.Add({Key, Value});
  }
  cpr::Response r = cpr::Get(cpr::Url{total_url}, CprParams, cpr::Timeout{30000});
  if (r.status_code == 0) {
    throw ApiErr("Нет такой сети: " + r.error.message);
  }
  if (r.status_code == 400) {
    throw ApiErr("Bad request");
  }
  if (r.status_code == 404) {
    throw ApiErr("Not Found");
  }
  if (r.status_code != 200) {
    throw ApiErr(std::to_string(r.status_code));
  }
  cache_.put(key, r.text);
  return nlohmann::json::parse(r.text);
}

void ApiHandler::BuildCityIdx(const nlohmann::json& data) {
  std::unordered_map<std::string, std::string> idx;
  for (auto& country : data.value("countries", nlohmann::json::array())) {
    for (auto& region : country.value("regions", nlohmann::json::array())) {
      for (auto& settlement: region.value("settlements", nlohmann::json::array())) {
        std::string title = settlement.value("title", "");
        if (title.empty()) {
          continue;
        }
        auto codes = settlement.value("codes", nlohmann::json::object());
        std::string code = codes.value("yandex_code", "");
        if (code.empty()) {
          continue;
        }
        idx[title] = code;
      }
    }
  }
  CityIdx_ = std::move(idx);
}

nlohmann::json ApiHandler::Search(const std::string& from, const std::string& to, const std::string& date, bool transfers) {
  std::map<std::string, std::string> params = {{"from", from}, {"to", to}, {"date", date}, {"lang", "ru_RU"} , {"format", "json"} , {"limit", "100"}, {"transfers", transfers ? "true" : "false"}};
  try {
    return HttpGet("/search", params);
  } catch (const std::exception& ex) {
    std::cerr << "Ошибка при поиске рейса из " << from << " в " << to << ": " << ex.what() << '\n';
    return nlohmann::json::object();
  }
}

std::string ApiHandler::MakeCacheKey(const std::string& endpoint, const std::map<std::string, std::string>& params) {
  std::ostringstream oss;
  oss << endpoint;
  for (auto& [key, value] : params) {
    oss << "|" << key << "=" << value;
  }
  return oss.str();
}