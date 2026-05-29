#include "../include/http_interface.h"
#include <cpr/cpr.h>
#include <string>
#include <map>

Response HttpClient::Get(const std::string& URL, const std::map<std::string, std::string>& params, int timeout) {
  cpr::Parameters cpr_params;
  for (const auto& [key, value] : params) {
    cpr_params.Add({key, value});
  }
  cpr::Response r = cpr::Get(cpr::Url(URL), cpr_params, cpr::Timeout(timeout));
  Response resp;
  resp.status_code = r.status_code;
  resp.text = r.text;
  resp.error_message = r.error.message;
  return resp;
}