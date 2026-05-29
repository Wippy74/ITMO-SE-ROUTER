#include "../include/cache.h"
#include <fstream>
#include <filesystem>
#include <exception>
#include <chrono>
#include <string>
#include <iostream>
#include <optional>
#include <unordered_map>

Cache::Cache(const std::string& filepath, size_t limit, std::chrono::seconds TTL): filepath_(filepath), gen_limit_(limit), TTL_(TTL) {
  fresh_.reserve(limit);
  LoadFromFile();
};

Cache::~Cache() {
  try {
    SaveToFile();
  } catch (const std::exception& ex) {
    std::cerr << "Ошибка сохранения: " << ex.what() << '\n';
  }
}

std::optional<std::string> Cache::get(const std::string& key) {
  auto it = fresh_.find(key);
  if (it != fresh_.end()) {
    if (std::chrono::system_clock::now() - it->second.created > TTL_) {
      fresh_.erase(it);
      return std::nullopt;
    } else {
      return it->second.val;
    }
  }
  it = old_.find(key);
  if (it != old_.end()) {
    if (std::chrono::system_clock::now() - it->second.created > TTL_) {
      old_.erase(it);
      return std::nullopt;
    }
    Entry promoted = std::move(it->second);
    old_.erase(it);
    if (fresh_.size() >= gen_limit_) {
      UpdateGen();
    }
    fresh_.emplace(key, std::move(promoted));
    return fresh_[key].val;
  }
  return std::nullopt;
}

void Cache::put(const std::string& key, const std::string& value) {
  old_.erase(key);
  auto it = fresh_.find(key);
  if (it != fresh_.end()) {
    it->second.val = value;
    it->second.created = std::chrono::system_clock::now();
    return;
  }
  if (fresh_.size() >= gen_limit_) {
    UpdateGen();
  }
  fresh_.emplace(key, Entry{value, std::chrono::system_clock::now()});
}

void Cache::clear() {
  old_.clear();
  fresh_.clear();
  std::error_code e;
  std::filesystem::remove(filepath_, e);
}

size_t Cache::size() const {
  return fresh_.size() + old_.size();
}

size_t Cache::FreshSize() const {
  return fresh_.size();
}

size_t Cache::OldSize() const {
  return old_.size();
}

void Cache::UpdateGen() {
  old_ = std::move(fresh_);
  fresh_.clear();
  fresh_.reserve(gen_limit_);
}

nlohmann::json Cache::Serialize(const std::unordered_map<std::string, Entry>& m) const {
  nlohmann::json o = nlohmann::json::object();
  for (auto& [key, entry] : m) {
    o[key] = {{"v", entry.val}, {"t", std::chrono::duration_cast<std::chrono::seconds>(entry.created.time_since_epoch()).count()}};
  }
  return o;
}

std::unordered_map<std::string, Cache::Entry> Cache::Deserialize(const nlohmann::json& j, std::chrono::seconds ttl) {
  std::unordered_map<std::string, Entry> m;
  if (!j.is_object()) {
    return m;
  }
  auto now = std::chrono::system_clock::now();
  for (auto& [key, val] : j.items()) {
    if (!val.contains("v") || !val.contains("t")) {
      continue;
    }
    int64_t epoch = val["t"].get<int64_t>();
    auto created = std::chrono::system_clock::time_point(std::chrono::seconds(epoch));
    if ((now - created) > ttl) continue;
    Entry e;
    e.val = val["v"].get<std::string>();
    e.created = created;
    m.emplace(key, std::move(e));
  }
  return m;
}

void Cache::LoadFromFile() {
  if (!std::filesystem::exists(filepath_)) {
    return;
  }
  try {
    std::ifstream in(filepath_);
    if (!in) {
      std::cerr << "Failed to open :" << filepath_ << '\n';
      return;
    }
    nlohmann::json root = nlohmann::json::parse(in);
    fresh_ = Deserialize(root.value("fresh", nlohmann::json::object()), TTL_);
    old_ = Deserialize(root.value("old", nlohmann::json::object()), TTL_);
    if (fresh_.size() > gen_limit_) {
      UpdateGen();
    }
  } catch (const std::exception& ex) {
    std::cerr << "Failed to read cache: " << ex.what() << '\n';
    fresh_.clear();
    old_.clear();
  }
}

void Cache::SaveToFile() const {
  try {
    auto p = std::filesystem::path(filepath_).parent_path();
    if (!p.empty() && !std::filesystem::exists(p)) {
      std::filesystem::create_directories(p);
    }
    nlohmann::json root = {{"fresh", Serialize(fresh_)} , {"old", Serialize(old_)}};
    std::string temp_path = filepath_ + ".tmp";
    {
      std::ofstream out(temp_path, std::ios::trunc);
      if (!out.is_open()) {
        std::cerr << "Failed to create: " << temp_path << '\n';
        return;
      }
      out << root.dump(2);
    }
    std::filesystem::rename(temp_path, filepath_);
  } catch (const std::exception& ex) {
    std::cerr << "Failed to write to cache: " << ex.what() << '\n';
  }
}
