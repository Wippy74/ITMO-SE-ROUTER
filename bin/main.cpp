#include "../include/api_handler.h"
#include "../include/cache.h"
#include "../include/route_builder.h"
#include "../include/config.h"

#ifdef _WIN32
#include <windows.h>
#endif

void PrintGuide(const char* p) {
  std::cout << "Поиск маршрутов между городами с помощью API Яндекс Расписаний \n\n";
  std::cout << "Режимы работы: \n\n";
  std::cout << "1. Одиночный запрос: \n" << p << " <откуда> <куда> <дата> <максимальное количество пересадок> \n\n";
  std::cout << "2. Интерактивный режим: \n" << p << " (далее вводить запросы)\n" << "Выход из интерактивного режима осуществляется командой quit \n";
}

bool ParseCLI(const std::string& l, std::string& from, std::string& to, std::string& date, int& transfers) {
  std::istringstream iss(l);
  std::string TransfersString;
  if (!(iss >> from >> to >> date >> TransfersString)) {
    return false;
  }
  try {
    transfers = std::stoi(TransfersString);
  } catch (...) {
    return false;
  }
  return true;
}

bool ProcessQuery(const std::string& FromCity, const std::string& ToCity, const std::string& date, int MaxTransfers, RouteBuilder& router) {
  if (MaxTransfers < 0) {
    std::cerr << "Максимальное количество пересадок должно быть больше 0 \n";
    return false;
  }
  if (FromCity == ToCity) {
    std::cerr << "Города отправления и прибытия должны отличаться \n";
    return false;
  }
  try {
    router.MakeAndPrint(FromCity, ToCity, date, MaxTransfers);
    return true;
  } catch (const ApiErr& ApiEx) {
    std::cerr << "Ошибка API: " << ApiEx.what() << '\n';
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
    std::string FromCity;
    std::string ToCity;
    std::string date;
    int MaxTransfers = 0;
    if (!ParseCLI(l, FromCity, ToCity, date, MaxTransfers)) {
      std::cerr << "Неверный формат ввода\n";
      continue;
    }
    ProcessQuery(FromCity, ToCity, date, MaxTransfers, router);
    std::cout << '\n';
  }
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
#endif
  if (argc == 1) {
    PrintGuide(argv[0]);
  }
  std::string ApiKey;
  try {
    Config c = Config::load();
    ApiKey = c.api_key;
  } catch (const std::exception& ex) {
    std::cerr << "Config error: " << ex.what() << '\n';
    return 1;
  }
  Cache cache("route_cache.json", 100, std::chrono::seconds(1800));
  ApiHandler api(ApiKey, cache);
  RouteBuilder router(api);
  if (argc == 5) {
    std::string FromCity = argv[1];
    std::string ToCity = argv[2];
    std::string date = argv[3];
    int MaxTransfers;
    try {
      MaxTransfers = std::stoi(argv[4]);
    } catch (...) {
      std::cerr << "Максимальное количество пересадок должно быть числом\n";
      return 1;
    }
    ProcessQuery(FromCity, ToCity, date, MaxTransfers, router);
    return 0;
  }
  Interactive(router, cache);
  return 0;
}