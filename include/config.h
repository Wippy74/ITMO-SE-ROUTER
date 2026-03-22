#pragma once

#include <string>
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

struct Config {
  std::string api_key;
  static Config load(const std::string& path = "") {
    std::vector<std::string> paths;
    if (!path.empty()) {
      paths.push_back(path);
    }
    paths.push_back("config.json");
    paths.push_back("../config.json");
    for (auto& p : paths) {
      std::ifstream ifs(p);
      if (!ifs.is_open()) {
        continue;
      }
      try {
        auto j = nlohmann::json::parse(ifs);
        Config cfg;
        cfg.api_key = j.value("api_key", "");
        return cfg;
      } catch (const nlohmann::json::exception& e) {
          throw std::runtime_error("Ошибка парсинга " + p + ": " + e.what());
      }
    }
    throw std::runtime_error("Файл конфига не найден.");
  }
};