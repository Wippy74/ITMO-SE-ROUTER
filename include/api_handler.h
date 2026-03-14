#pragma once

#include <nlohmann/json.hpp>
#include <string>

struct thread {
  std::string uid;
  std::string title;
  std::string interval;
  std::string number;
  std::string short_title;
  std::string thread_method_link;
  std::string carrier;
  std::string transport_type;
  std::string vehicle;
  std::string transport_subtype;
  std::string express_type;
  std::string from;
  std::string to;
};

class ApiHandler {
  
};