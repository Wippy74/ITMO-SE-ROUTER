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


int main() {

}