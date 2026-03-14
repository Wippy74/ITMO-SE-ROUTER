#pragma once

#include <iostream>
#include <list>
#include <optional>
#include <string>
#include <chrono>
#include <unordered_map>

class Cache {
public:
  explicit Cache(const size_t limit, std::chrono::seconds ttl) : gen_limit_(limit), TTL_(ttl) {}

  std::optional<std::string> get(const std::string& key);
  void put(const std::string& key, const std::string& value);

  void clear();
  size_t size() const;
  size_t FreshSize() const;
  size_t OldSize() const;
private:
  struct Entry {
    std::string val;
    std::chrono::steady_clock::time_point created;
  };

  const size_t gen_limit_;
  const std::chrono::seconds TTL_;

  std::unordered_map<std::string, Entry> fresh_;
  std::unordered_map<std::string, Entry> old_;
};