#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../include/api_handler.h"
#include "http_client_mock.h"
#include <filesystem>
#include <memory>
#include <chrono>

using ::testing::_;
using ::testing::Return;
using ::testing::HasSubstr;
using ::testing::Contains;
using ::testing::Pair;
using ::testing::AllOf;

class ApiHandlerTest : public ::testing::Test {
protected:
  void SetUp() override {
    test_cache_file_ = "test_api_cache_" + std::to_string(std::rand()) + ".json";
    cache_ = std::make_unique<Cache>(test_cache_file_, 100, std::chrono::seconds(3600));
    mock_http_ = std::make_shared<MockHttpClient>();
  }

  void TearDown() override {
    cache_.reset();
    std::error_code ec;
    std::filesystem::remove(test_cache_file_, ec);
  }

  std::string test_cache_file_;
  std::unique_ptr<Cache> cache_;
  std::shared_ptr<MockHttpClient> mock_http_;
};

const std::string kStationsJson = R"({
  "countries": [
    {
      "title": "Россия",
      "regions": [
        {
          "title": "Санкт-Петербург и Ленинградская область",
          "settlements": [
            {"title": "Санкт-Петербург", "codes": {"yandex_code": "c2"}},
            {"title": "Выборг", "codes": {"yandex_code": "c969"}},
            {"title": "Петергоф", "codes": {"yandex_code": "c20168"}}
          ]
        },
        {
          "title": "Московская область",
          "settlements": [
            {"title": "Москва", "codes": {"yandex_code": "c213"}},
            {"title": "Химки", "codes": {"yandex_code": "c10758"}},
            {"title": "Мытищи", "codes": {"yandex_code": "c10740"}},
            {"title": "Люберцы", "codes": {"yandex_code": "c10738"}}
          ]
        },
        {
          "title": "Челябинская область",
          "settlements": [
            {"title": "Челябинск", "codes": {"yandex_code": "c56"}},
            {"title": "Магнитогорск", "codes": {"yandex_code": "c235"}}
          ]
        },
        {
          "title": "Краснодарский край",
          "settlements": [
            {"title": "Сочи", "codes": {"yandex_code": "c239"}},
            {"title": "Краснодар", "codes": {"yandex_code": "c35"}},
            {"title": "Новороссийск", "codes": {"yandex_code": "c11065"}},
            {"title": "Анапа", "codes": {"yandex_code": "c1107"}},
            {"title": "Геленджик", "codes": {"yandex_code": "c10994"}}
          ]
        }
      ]
    }
  ]
})";

const std::string kSearchJson = R"({
  "segments": [
    {
      "thread": {
        "title": "Москва — Сочи",
        "number": "SU1114",
        "transport_type": "plane",
        "carrier": {"title": "Аэрофлот"}
      },
      "from": {"title": "Шереметьево"},
      "to": {"title": "Адлер"},
      "departure": "2026-03-22T08:00:00+03:00",
      "arrival": "2026-03-22T10:30:00+03:00",
      "duration": 9000.0
    }
  ]
})";


const std::string kMixedTransportJson = R"({
  "segments": [
    {
      "thread": {
        "title": "Москва — Казань",
        "number": "SU1190",
        "transport_type": "plane",
        "carrier": {"title": "Аэрофлот"}
      },
      "from": {"title": "Шереметьево"},
      "to": {"title": "Казань"},
      "departure": "2067-03-01T07:00:00+03:00",
      "arrival": "2067-03-01T08:30:00+03:00",
      "duration": 5400.0
    },
    {
      "thread": {
        "title": "Москва — Казань",
        "number": "002М",
        "transport_type": "train",
        "carrier": {"title": "РЖД"}
      },
      "from": {"title": "Казанский вокзал"},
      "to": {"title": "Казань-Пасс."},
      "departure": "2067-03-01T21:43:00+03:00",
      "arrival": "2067-03-01T08:17:00+03:00",
      "duration": 38040.0
    }
  ]
})";

const std::string kMultipleFlightsJson = R"({
  "segments": [
    {
      "thread": {
        "title": "Москва — Санкт-Петербург",
        "number": "SU30",
        "transport_type": "plane",
        "carrier": {"title": "Аэрофлот"}
      },
      "from": {"title": "Шереметьево"},
      "to": {"title": "Пулково"},
      "departure": "2026-03-22T06:00:00+03:00",
      "arrival": "2026-03-22T07:30:00+03:00",
      "duration": 5400.0
    },
    {
      "thread": {
        "title": "Москва — Санкт-Петербург",
        "number": "SU32",
        "transport_type": "plane",
        "carrier": {"title": "Аэрофлот"}
      },
      "from": {"title": "Шереметьево"},
      "to": {"title": "Пулково"},
      "departure": "2026-03-22T08:00:00+03:00",
      "arrival": "2026-03-22T09:30:00+03:00",
      "duration": 5400.0
    },
    {
      "thread": {
        "title": "Москва — Санкт-Петербург",
        "number": "DP6002",
        "transport_type": "plane",
        "carrier": {"title": "Победа"}
      },
      "from": {"title": "Внуково"},
      "to": {"title": "Пулково"},
      "departure": "2026-03-22T10:00:00+03:00",
      "arrival": "2026-03-22T11:30:00+03:00",
      "duration": 5400.0
    }
  ]
})";

const std::string kTerminalsJson = R"({
  "segments": [
    {
      "thread": {
        "title": "Москва — Сочи",
        "number": "SU1114",
        "transport_type": "plane",
        "carrier": {"title": "Аэрофлот"}
      },
      "from": {"title": "Шереметьево"},
      "to": {"title": "Адлер"},
      "departure": "2067-03-07T08:00:00+03:00",
      "arrival": "2067-03-07T10:30:00+03:00",
      "duration": 9000.0,
      "departure_terminal": "67",
      "arrival_terminal": "07"
    }
  ]
})";

TEST_F(ApiHandlerTest, TakeCitySaintPetersburgRegion) {
  Response response{200, kStationsJson, ""};
  EXPECT_CALL(*mock_http_, Get(HasSubstr("/stations_list"), _, _))
      .WillOnce(Return(response));

  ApiHandler api("test_key", *cache_, mock_http_);

  EXPECT_EQ(api.TakeCity("Санкт-Петербург"), "c2");
  EXPECT_EQ(api.TakeCity("Выборг"), "c969");
  EXPECT_EQ(api.TakeCity("Петергоф"), "c20168");
}

TEST_F(ApiHandlerTest, TakeCityMoscowRegion) {
  Response response{200, kStationsJson, ""};
  EXPECT_CALL(*mock_http_, Get(HasSubstr("/stations_list"), _, _))
      .WillOnce(Return(response));

  ApiHandler api("test_key", *cache_, mock_http_);

  EXPECT_EQ(api.TakeCity("Москва"), "c213");
  EXPECT_EQ(api.TakeCity("Химки"), "c10758");
  EXPECT_EQ(api.TakeCity("Мытищи"), "c10740");
  EXPECT_EQ(api.TakeCity("Люберцы"), "c10738");
}

TEST_F(ApiHandlerTest, TakeCityChelyabinskRegion) {
  Response response{200, kStationsJson, ""};
  EXPECT_CALL(*mock_http_, Get(HasSubstr("/stations_list"), _, _))
      .WillOnce(Return(response));

  ApiHandler api("test_key", *cache_, mock_http_);

  EXPECT_EQ(api.TakeCity("Челябинск"), "c56");
  EXPECT_EQ(api.TakeCity("Магнитогорск"), "c235");
}

TEST_F(ApiHandlerTest, TakeCityKrasnodarRegion) {
  Response response{200, kStationsJson, ""};
  EXPECT_CALL(*mock_http_, Get(HasSubstr("/stations_list"), _, _))
      .WillOnce(Return(response));

  ApiHandler api("test_key", *cache_, mock_http_);

  EXPECT_EQ(api.TakeCity("Сочи"), "c239");
  EXPECT_EQ(api.TakeCity("Краснодар"), "c35");
  EXPECT_EQ(api.TakeCity("Новороссийск"), "c11065");
  EXPECT_EQ(api.TakeCity("Анапа"), "c1107");
  EXPECT_EQ(api.TakeCity("Геленджик"), "c10994");
}

TEST_F(ApiHandlerTest, TakeCityNotFound) {
  Response stations_response{200, kStationsJson, ""};

  EXPECT_CALL(*mock_http_, Get(HasSubstr("/stations_list"), _, _))
      .WillOnce(Return(stations_response));

  ApiHandler api("test_key", *cache_, mock_http_);

  EXPECT_THROW(api.TakeCity("НесуществующийГород"), ApiErr);
}

TEST_F(ApiHandlerTest, TakeCityCachesStationList) {
  Response stations_response{200, kStationsJson, ""};

  EXPECT_CALL(*mock_http_, Get(HasSubstr("/stations_list"), _, _))
      .Times(1)
      .WillOnce(Return(stations_response));

  ApiHandler api("test_key", *cache_, mock_http_);

  api.TakeCity("Санкт-Петербург");
  api.TakeCity("Москва");
  api.TakeCity("Челябинск");
}

TEST_F(ApiHandlerTest, SearchMixedTransport) {
  Response response{200, kMixedTransportJson, ""};
  EXPECT_CALL(*mock_http_, Get(HasSubstr("/search"), _, _))
      .WillOnce(Return(response));

  ApiHandler api("test_key", *cache_, mock_http_);

  auto result = api.Search("c213", "c43", "2067-03-01", false);

  EXPECT_EQ(result["segments"].size(), 2);
  EXPECT_EQ(result["segments"][0]["thread"]["transport_type"], "plane");
  EXPECT_EQ(result["segments"][1]["thread"]["transport_type"], "train");
}

TEST_F(ApiHandlerTest, SearchSuccess) {
  Response search_response{200, kSearchJson, ""};

  EXPECT_CALL(*mock_http_, Get(HasSubstr("/search"), _, _))
      .WillOnce(Return(search_response));

  ApiHandler api("test_key", *cache_, mock_http_);

  auto result = api.Search("c213", "c239", "2026-03-22", false);

  EXPECT_TRUE(result.contains("segments"));
  EXPECT_EQ(result["segments"].size(), 1);
  EXPECT_EQ(result["segments"][0]["thread"]["number"], "SU1114");
}

TEST_F(ApiHandlerTest, SearchWithTransfers) {
  Response search_response{200, kSearchJson, ""};

  EXPECT_CALL(*mock_http_, Get(
      HasSubstr("/search"),
      Contains(Pair("transfers", "true")),
      _
  )).WillOnce(Return(search_response));

  ApiHandler api("test_key", *cache_, mock_http_);

  auto result = api.Search("c213", "c239", "2026-03-22", true);

  EXPECT_TRUE(result.contains("segments"));
}

TEST_F(ApiHandlerTest, SearchWithoutTransfers) {
  Response search_response{200, kSearchJson, ""};

  EXPECT_CALL(*mock_http_, Get(
      HasSubstr("/search"),
      Contains(Pair("transfers", "false")),
      _
  )).WillOnce(Return(search_response));

  ApiHandler api("test_key", *cache_, mock_http_);

  auto result = api.Search("c213", "c239", "2026-03-22", false);

  EXPECT_TRUE(result.contains("segments"));
}

TEST_F(ApiHandlerTest, SearchPassesCorrectParams) {
  Response search_response{200, kSearchJson, ""};

  EXPECT_CALL(*mock_http_, Get(
      _,
      AllOf(
          Contains(Pair("from", "c213")),
          Contains(Pair("to", "c239")),
          Contains(Pair("date", "2026-03-22")),
          Contains(Pair("apikey", "test_key"))
      ),
      _
  )).WillOnce(Return(search_response));

  ApiHandler api("test_key", *cache_, mock_http_);

  api.Search("c213", "c239", "2026-03-22", false);
}

TEST_F(ApiHandlerTest, SearchMultipleResults) {
  Response response{200, kMultipleFlightsJson, ""};
  EXPECT_CALL(*mock_http_, Get(HasSubstr("/search"), _, _))
      .WillOnce(Return(response));

  ApiHandler api("test_key", *cache_, mock_http_);

  auto result = api.Search("c213", "c2", "2026-03-22", false);

  EXPECT_EQ(result["segments"].size(), 3);
}

TEST_F(ApiHandlerTest, SearchPlaneRoutes) {
  Response response{200, kMultipleFlightsJson, ""};
  EXPECT_CALL(*mock_http_, Get(HasSubstr("/search"), _, _))
      .WillOnce(Return(response));

  ApiHandler api("test_key", *cache_, mock_http_);

  auto result = api.Search("c213", "c2", "2026-03-22", false);

  for (auto& seg : result["segments"]) {
    EXPECT_EQ(seg["thread"]["transport_type"], "plane");
  }
}

TEST_F(ApiHandlerTest, SearchResultWithTerminals) {
  Response response{200, kTerminalsJson, ""};
  EXPECT_CALL(*mock_http_, Get(_, _, _))
      .WillOnce(Return(response));

  ApiHandler api("test_key", *cache_, mock_http_);

  auto result = api.Search("c213", "c239", "2067-03-07", false);
  auto& seg = result["segments"][0];

  EXPECT_TRUE(seg.contains("departure_terminal"));
  EXPECT_TRUE(seg.contains("arrival_terminal"));
  EXPECT_EQ(seg["departure_terminal"], "67");
  EXPECT_EQ(seg["arrival_terminal"], "07");
}

TEST_F(ApiHandlerTest, NetworkError) {
  Response error_response{0, "", "Connection refused"};

  EXPECT_CALL(*mock_http_, Get(_, _, _))
      .WillOnce(Return(error_response));

  ApiHandler api("test_key", *cache_, mock_http_);

  auto result = api.Search("c213", "c239", "2026-03-22", false);
  EXPECT_TRUE(result.empty() || !result.contains("segments"));
}

TEST_F(ApiHandlerTest, NetworkErrorOnTakeCity) {
  Response error_response{0, "", "Connection refused"};

  EXPECT_CALL(*mock_http_, Get(_, _, _))
      .WillOnce(Return(error_response));

  ApiHandler api("test_key", *cache_, mock_http_);

  EXPECT_THROW(api.TakeCity("Москва"), ApiErr);
}

TEST_F(ApiHandlerTest, HttpError400) {
  Response bad_request{400, "", ""};

  EXPECT_CALL(*mock_http_, Get(_, _, _))
      .WillOnce(Return(bad_request));

  ApiHandler api("test_key", *cache_, mock_http_);

  EXPECT_THROW(api.TakeCity("Москва"), ApiErr);
}

TEST_F(ApiHandlerTest, HttpError404) {
  Response not_found{404, "", ""};

  EXPECT_CALL(*mock_http_, Get(_, _, _))
      .WillOnce(Return(not_found));

  ApiHandler api("test_key", *cache_, mock_http_);

  EXPECT_THROW(api.TakeCity("Москва"), ApiErr);
}

TEST_F(ApiHandlerTest, HttpError500) {
  Response server_error{500, "", ""};

  EXPECT_CALL(*mock_http_, Get(_, _, _))
      .WillOnce(Return(server_error));

  ApiHandler api("test_key", *cache_, mock_http_);

  EXPECT_THROW(api.TakeCity("Москва"), ApiErr);
}


TEST_F(ApiHandlerTest, SearchResultsCached) {
  Response search_response{200, kSearchJson, ""};

  EXPECT_CALL(*mock_http_, Get(HasSubstr("/search"), _, _))
      .Times(1)
      .WillOnce(Return(search_response));

  ApiHandler api("test_key", *cache_, mock_http_);

  auto result1 = api.Search("c213", "c239", "2026-03-22", false);
  
  auto result2 = api.Search("c213", "c239", "2026-03-22", false);

  EXPECT_EQ(result1, result2);
}

TEST_F(ApiHandlerTest, BuildCityIndexCorrectly) {
  Response stations_response{200, kStationsJson, ""};

  EXPECT_CALL(*mock_http_, Get(_, _, _))
      .WillOnce(Return(stations_response));

  ApiHandler api("test_key", *cache_, mock_http_);

  api.TakeCity("Москва");

  EXPECT_EQ(api.GetCityIndexSize(), 14);
}