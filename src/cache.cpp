#include "../include/cache.h"
#include <fstream>
#include <filesystem>
#include <exception>

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
  auto it = old_.find(key);
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
    UpdateGen;
  }
  fresh_.emplace(key, Entry{key, std::chrono::system_clock::now()});
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
