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

int main() {

}