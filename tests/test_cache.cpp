#include <gtest/gtest.h>
#include "../include/cache.h"
#include <filesystem>
#include <thread>
#include <string>

class CacheTest : public ::testing::Test {
protected:
  void SetUp() override {
    test_file_ = "test_cache_" + std::to_string(std::rand()) + ".json";
  }

  void TearDown() override {
    std::error_code ec;
    std::filesystem::remove(test_file_, ec);
  }

  std::string test_file_;
};

TEST_F(CacheTest, PutAndGet) {
  Cache cache(test_file_, 10, std::chrono::seconds(3600));

  cache.put("key1", "value1");
  cache.put("key2", "value2");

  auto result1 = cache.get("key1");
  auto result2 = cache.get("key2");

  ASSERT_TRUE(result1.has_value());
  EXPECT_EQ(*result1, "value1");
  ASSERT_TRUE(result2.has_value());
  EXPECT_EQ(*result2, "value2");
}

TEST_F(CacheTest, GetNonExistent) {
  Cache cache(test_file_, 10, std::chrono::seconds(3600));

  auto result = cache.get("nonexistent");

  EXPECT_FALSE(result.has_value());
}

TEST_F(CacheTest, UpdateExistingKey) {
  Cache cache(test_file_, 10, std::chrono::seconds(3600));

  cache.put("key1", "value1");
  cache.put("key1", "value2");

  auto result = cache.get("key1");

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, "value2");
}

TEST_F(CacheTest, TTLExpiration) {
  Cache cache(test_file_, 10, std::chrono::seconds(1));

  cache.put("key1", "value1");

  auto result1 = cache.get("key1");
  ASSERT_TRUE(result1.has_value());

  std::this_thread::sleep_for(std::chrono::seconds(2));

  auto result2 = cache.get("key1");
  EXPECT_FALSE(result2.has_value());
}

TEST_F(CacheTest, GenerationShift) {
  Cache cache(test_file_, 3, std::chrono::seconds(3600));

  cache.put("key1", "value1");
  cache.put("key2", "value2");
  cache.put("key3", "value3");

  EXPECT_EQ(cache.FreshSize(), 3u);
  EXPECT_EQ(cache.OldSize(), 0u);

  cache.put("key4", "value4");

  EXPECT_EQ(cache.FreshSize(), 1u);
  EXPECT_EQ(cache.OldSize(), 3u);

  EXPECT_TRUE(cache.get("key1").has_value());
  EXPECT_TRUE(cache.get("key2").has_value());
  EXPECT_TRUE(cache.get("key3").has_value());
  EXPECT_TRUE(cache.get("key4").has_value());
}

TEST_F(CacheTest, PromoteFromOldToFresh) {
  Cache cache(test_file_, 2, std::chrono::seconds(3600));

  cache.put("key1", "value1");
  cache.put("key2", "value2");
  cache.put("key3", "value3");

  EXPECT_EQ(cache.OldSize(), 2u);
  EXPECT_EQ(cache.FreshSize(), 1u);

  auto result = cache.get("key1");
  ASSERT_TRUE(result.has_value());

  EXPECT_EQ(cache.OldSize(), 1u);
  EXPECT_EQ(cache.FreshSize(), 2u);
}

TEST_F(CacheTest, PersistenceToFile) {
  {
    Cache cache(test_file_, 10, std::chrono::seconds(3600));
    cache.put("persistent_key", "persistent_value");
  }

  {
    Cache cache2(test_file_, 10, std::chrono::seconds(3600));
    auto result = cache2.get("persistent_key");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "persistent_value");
  }
}

TEST_F(CacheTest, ClearCache) {
  Cache cache(test_file_, 10, std::chrono::seconds(3600));

  cache.put("key1", "value1");
  cache.put("key2", "value2");

  cache.clear();

  EXPECT_EQ(cache.size(), 0u);
  EXPECT_FALSE(cache.get("key1").has_value());
  EXPECT_FALSE(cache.get("key2").has_value());
}

TEST_F(CacheTest, SizeCalculation) {
  Cache cache(test_file_, 10, std::chrono::seconds(3600));

  EXPECT_EQ(cache.size(), 0u);

  cache.put("key1", "value1");
  EXPECT_EQ(cache.size(), 1u);

  cache.put("key2", "value2");
  EXPECT_EQ(cache.size(), 2u);

  cache.put("key1", "new_value");
  EXPECT_EQ(cache.size(), 2u);
}