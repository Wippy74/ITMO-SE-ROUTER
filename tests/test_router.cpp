#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../include/route_builder.h"
#include "http_client_mock.h"
#include <filesystem>
#include <sstream>

using ::testing::_;
using ::testing::Return;
using ::testing::HasSubstr;

class RouteBuilderTest : public ::testing::Test {
protected:
  void SetUp() override {
    test_cache_file_ = "test_rb_cache_" + std::to_string(std::rand()) + ".json";
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
  "countries": [{
    "title": "Россия",
    "regions": [{
      "title": "Тест",
      "settlements": [
        {"title": "Москва", "codes": {"yandex_code": "c213"}},
        {"title": "Сочи", "codes": {"yandex_code": "c239"}}
      ]
    }]
  }]
})";

const std::string kDirectRouteJson = R"({
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
    },
    {
      "thread": {
        "title": "Москва — Сочи",
        "number": "DP407",
        "transport_type": "plane",
        "carrier": {"title": "Победа"}
      },
      "from": {"title": "Внуково"},
      "to": {"title": "Адлер"},
      "departure": "2026-03-22T12:00:00+03:00",
      "arrival": "2026-03-22T14:30:00+03:00",
      "duration": 9000.0
    }
  ]
})";

const std::string kTransferRouteJson = R"({
  "segments": [
    {
      "departure": "2026-03-22T07:00:00+03:00",
      "arrival": "2026-03-22T15:00:00+03:00",
      "departure_from": {"title": "Москва "},
      "arrival_to": {"title": "Сочи"},
      "duration": 28800.0,
      "details": [
        {
          "thread": {
            "title": "Москва — Ростов",
            "number": "101А",
            "transport_type": "train",
            "carrier": {"title": "РЖД"}
          },
          "from": {"title": "Москва"},
          "to": {"title": "Ростов"},
          "departure": "2026-03-22T07:00:00+03:00",
          "arrival": "2026-03-22T11:00:00+03:00",
          "duration": 14400.0
        },
        {
          "thread": {
            "title": "Ростов — Сочи",
            "number": "102Б",
            "transport_type": "train",
            "carrier": {"title": "РЖД"}
          },
          "from": {"title": "Ростов"},
          "to": {"title": "Сочи"},
          "departure": "2026-03-22T12:00:00+03:00",
          "arrival": "2026-03-22T15:00:00+03:00",
          "duration": 10800.0
        }
      ]
    }
  ]
})";

const std::string kEmptyRouteJson = R"({"segments": []})";

TEST_F(RouteBuilderTest, FindDirectRoutes) {
  Response stations_response{200, kStationsJson, ""};
  Response search_response{200, kDirectRouteJson, ""};

  EXPECT_CALL(*mock_http_, Get(HasSubstr("/stations_list"), _, _))
      .WillOnce(Return(stations_response));
  EXPECT_CALL(*mock_http_, Get(HasSubstr("/search"), _, _))
      .WillOnce(Return(search_response));

  ApiHandler api("test_key", *cache_, mock_http_);
  RouteBuilder builder(api);

  std::ostringstream output;
  std::streambuf* old_cout = std::cout.rdbuf(output.rdbuf());

  builder.MakeAndPrint("Москва", "Сочи", "2026-03-22", 0, 10);

  std::cout.rdbuf(old_cout);

  std::string out = output.str();
  EXPECT_THAT(out, HasSubstr("маршрутов: 2"));
  EXPECT_THAT(out, HasSubstr("SU1114"));
  EXPECT_THAT(out, HasSubstr("DP407"));
  EXPECT_THAT(out, HasSubstr("Аэрофлот"));
  EXPECT_THAT(out, HasSubstr("Победа"));
  EXPECT_THAT(out, HasSubstr("08:00"));
  EXPECT_THAT(out, HasSubstr("12:00"));
}

TEST_F(RouteBuilderTest, FindTransferRoutes) {
  Response stations_response{200, kStationsJson, ""};
  Response search_response{200, kTransferRouteJson, ""};

  EXPECT_CALL(*mock_http_, Get(HasSubstr("/stations_list"), _, _))
      .WillOnce(Return(stations_response));
  EXPECT_CALL(*mock_http_, Get(HasSubstr("/search"), _, _))
      .WillOnce(Return(search_response));

  ApiHandler api("test_key", *cache_, mock_http_);
  RouteBuilder builder(api);

  std::ostringstream output;
  std::streambuf* old_cout = std::cout.rdbuf(output.rdbuf());

  builder.MakeAndPrint("Москва", "Сочи", "2026-03-22", 1, 10);

  std::cout.rdbuf(old_cout);

  std::string out = output.str();
  EXPECT_THAT(out, HasSubstr("пересадок"));
  EXPECT_THAT(out, HasSubstr("Участок 1"));
  EXPECT_THAT(out, HasSubstr("Участок 2"));
  EXPECT_THAT(out, HasSubstr("101А"));
  EXPECT_THAT(out, HasSubstr("102Б"));
}

TEST_F(RouteBuilderTest, LimitRoutesOutput) {
  Response stations_response{200, kStationsJson, ""};
  Response search_response{200, kDirectRouteJson, ""};

  EXPECT_CALL(*mock_http_, Get(HasSubstr("/stations_list"), _, _))
      .WillOnce(Return(stations_response));
  EXPECT_CALL(*mock_http_, Get(HasSubstr("/search"), _, _))
      .WillOnce(Return(search_response));

  ApiHandler api("test_key", *cache_, mock_http_);
  RouteBuilder builder(api);

  std::ostringstream output;
  std::streambuf* old_cout = std::cout.rdbuf(output.rdbuf());

  builder.MakeAndPrint("Москва", "Сочи", "2026-03-22", 0, 1);

  std::cout.rdbuf(old_cout);

  std::string out = output.str();
  EXPECT_THAT(out, HasSubstr("показано первых 1"));
  EXPECT_THAT(out, HasSubstr("SU1114"));
  EXPECT_THAT(out, ::testing::Not(HasSubstr("DP407")));
}

TEST_F(RouteBuilderTest, NoRoutesFound) {
  Response stations_response{200, kStationsJson, ""};
  Response search_response{200, kEmptyRouteJson, ""};

  EXPECT_CALL(*mock_http_, Get(HasSubstr("/stations_list"), _, _))
      .WillOnce(Return(stations_response));
  EXPECT_CALL(*mock_http_, Get(HasSubstr("/search"), _, _))
      .WillOnce(Return(search_response));

  ApiHandler api("test_key", *cache_, mock_http_);
  RouteBuilder builder(api);

  std::ostringstream output;
  std::streambuf* old_cout = std::cout.rdbuf(output.rdbuf());

  builder.MakeAndPrint("Москва", "Сочи", "2026-03-22", 0, 10);

  std::cout.rdbuf(old_cout);

  std::string out = output.str();
  EXPECT_THAT(out, HasSubstr("маршрутов: 0"));
}

TEST_F(RouteBuilderTest, CityNotFound) {
  Response stations_response{200, kStationsJson, ""};

  EXPECT_CALL(*mock_http_, Get(HasSubstr("/stations_list"), _, _))
      .WillOnce(Return(stations_response));

  ApiHandler api("test_key", *cache_, mock_http_);
  RouteBuilder builder(api);

  EXPECT_THROW(
      builder.MakeAndPrint("НеsуществующийГород", "Сочи", "2026-03-22", 0, 10),
      ApiErr
  );
}

TEST_F(RouteBuilderTest, DurationFormatting) {
  Response stations_response{200, kStationsJson, ""};
  Response search_response{200, kDirectRouteJson, ""};

  EXPECT_CALL(*mock_http_, Get(HasSubstr("/stations_list"), _, _))
      .WillOnce(Return(stations_response));
  EXPECT_CALL(*mock_http_, Get(HasSubstr("/search"), _, _))
      .WillOnce(Return(search_response));

  ApiHandler api("test_key", *cache_, mock_http_);
  RouteBuilder builder(api);

  std::ostringstream output;
  std::streambuf* old_cout = std::cout.rdbuf(output.rdbuf());

  builder.MakeAndPrint("Москва", "Сочи", "2026-03-22", 0, 10);

  std::cout.rdbuf(old_cout);

  std::string out = output.str();
  EXPECT_THAT(out, HasSubstr("2ч."));
  EXPECT_THAT(out, HasSubstr("30мин."));
}