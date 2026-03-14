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