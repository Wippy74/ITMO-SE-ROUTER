#pragma once

#include <nlohmann/json.hpp>

#include <iostream>
#include <list>
#include <optional>
#include <string>
#include <chrono>
#include <unordered_map>

class Cache {
public:
  explicit Cache(const std::string& filepath, const size_t limit = 100, std::chrono::seconds ttl = std::chrono::seconds(1800));

  ~Cache();

  std::optional<std::string> get(const std::string& key);

  void put(const std::string& key, const std::string& value);

  void clear();
  size_t size() const;
  size_t FreshSize() const;
  size_t OldSize() const;
private:
  struct Entry {
    std::string val;
    std::chrono::system_clock::time_point created;
  };

  const std::string path_;
  const size_t gen_limit_;
  const std::chrono::seconds TTL_;

  std::unordered_map<std::string, Entry> fresh_;
  std::unordered_map<std::string, Entry> old_;

  void LoadFromFile();
  void SaveToFile() const;

  nlohmann::json Serialize(const std::unordered_map<std::string, Entry>& m);
  std::unordered_map<std::string, Entry> Deserialize(const nlohmann::json& j, std::chrono::seconds ttl);

  void UpdateGen();
};