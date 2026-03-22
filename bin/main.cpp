#include "../include/api_handler.h"
#include "../include/cache.h"
#include "../include/route_builder.h"
#include "../include/config.h"

void PrintGuide(const char* p) {
  std::cout << "Поиск маршрутов между городами с помощью API Яндекс Расписаний" << '\n\n';
  std::cout << "Режимы работы: \n\n";
  std::cout << "1. Одиночный запрос: \n" << p << " <откуда> <куда> <дата> <максимальное количество пересадок> \n\n";
  std::cout << "2. Интерактивный режим: \n" << p << "(даллее вводить запросы)\n" << "Выход из интерактивного режима осуществляется командой quit";
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

int main() {

}