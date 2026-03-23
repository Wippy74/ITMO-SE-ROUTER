#include "../include/api_handler.h"
#include "../include/cache.h"
#include "../include/route_builder.h"
#include "../include/config.h"

#include <iostream>
#include <string>
#include <exception>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#endif

void PrintGuide(const char* p) {
  std::cout << "Поиск маршрутов между городами с помощью API Яндекс Расписаний \n\n";
  std::cout << "Режимы работы: \n\n";
  std::cout << "1. Одиночный запрос: \n" << p << " <откуда> <куда> <дата> <максимальное количество пересадок> <количество выводимых маршрутов>\n\n";
  std::cout << "2. Интерактивный режим: \n" << p << " (далее вводить запросы)\n" << "Выход из интерактивного режима осуществляется командой quit \n";
}

bool ParseCLI(const std::string& l, std::string& from, std::string& to, std::string& date, int& transfers, int& limit) {
  std::istringstream iss(l);
  std::string transfer_string;
  std::string limit_string;
  if (!(iss >> from >> to >> date >> transfer_string >> limit_string)) {
    return false;
  }
  try {
    transfers = std::stoi(transfer_string);
    limit = std::stoi(limit_string);
  } catch (...) {
    return false;
  }
  return true;
}

bool ProcessQuery(const std::string& from_city, const std::string& to_city, const std::string& date, int max_transfers, int limit, RouteBuilder& router) {
  if (max_transfers < 0) {
    std::cerr << "Максимальное количество пересадок должно быть больше 0 \n";
    return false;
  }
  if (from_city == to_city) {
    std::cerr << "Города отправления и прибытия должны отличаться \n";
    return false;
  }
  try {
    router.MakeAndPrint(from_city, to_city, date, max_transfers, limit);
    return true;
  } catch (const ApiErr& api_ex) {
    std::cerr << "Ошибка API: " << api_ex.what() << '\n';
  } catch (const std::exception& ex) {
    std::cerr << "Ошибка: " << ex.what() << '\n';
  }
  return false;
}

void Interactive(RouteBuilder& router, Cache& cache) {
  std::string l;
  while (true) {
    std::cout << "Введите запрос: ";
    std::cout.flush();
    if (!std::getline(std::cin, l)) {
      break;
    }
    if (l.empty()) {
      continue;
    }
    if (l == "quit") {
      std::cout << "Выход\n";
      break;
    }
    std::string from_city;
    std::string to_city;
    std::string date;
    int max_transfers = 0;
    int limit = 10;
    if (!ParseCLI(l, from_city, to_city, date, max_transfers, limit)) {
      std::cerr << "Неверный формат ввода\n";
      continue;
    }
    ProcessQuery(from_city, to_city, date, max_transfers, limit, router);
    std::cout << '\n';
  }
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
#endif
  if (argc != 1 && argc != 6) {
    PrintGuide(argv[0]);
    return 1;
  }
  std::string api_key;
  try {
    Config c = Config::load();
    api_key = c.api_key;
  } catch (const std::exception& ex) {
    std::cerr << "Config error: " << ex.what() << '\n';
    return 1;
  }
  Cache cache("route_cache.json", 100, std::chrono::seconds(1800));
  ApiHandler api(api_key, cache);
  RouteBuilder router(api);
  if (argc == 6) {
    std::string from_city = argv[1];
    std::string to_city = argv[2];
    std::string date = argv[3];
    int max_transfers;
    int limit;
    try {
      max_transfers = std::stoi(argv[4]);
    } catch (...) {
      std::cerr << "Максимальное количество пересадок должно быть числом\n";
      return 1;
    }
    try {
      limit = std::stoi(argv[5]);
    } catch (...) {
      std::cerr << "Число выводимых маршрутов должно быть числом\n";
      return 1;
    }
    ProcessQuery(from_city, to_city, date, max_transfers, limit, router);
    return 0;
  }
  Interactive(router, cache);
  return 0;
}