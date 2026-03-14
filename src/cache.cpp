#include "../include/cache.h"

std::optional<std::string> Cache::get(const std::string& key) {
  auto it = fresh_.find(key);
  if (it != fresh_.end()) {
    if (std::chrono::steady_clock::now() - it->second.created > TTL_) {
      fresh_.erase(it);
      return std::nullopt;
    } else {
      return it->second.val;
    }
  }
  auto it = old_.find(key);
  if (it != old_.end()) {
    if (std::chrono::steady_clock::now() - it->second.created > TTL_) {
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
    it->second.created = std::chrono::steady_clock::now();
    return;
  }
  if (fresh_.size() >= gen_limit_) {
    UpdateGen;
  }
  fresh_.emplace(key, Entry{key, std::chrono::steady_clock::now()});
}

void Cache::clear() {
  old_.clear();
  fresh_.clear();
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