#include<unordered_map>
#include<vector>
#include<nlohmann/json.hpp>
#include<sys/socket.h>
#include "../const.hpp"
#include "../Db_manager/Db_manager.hpp"
struct HttpResponse {
  std::unordered_map<std::string, std::string> header;
  std::string status_code, content;
  nlohmann::json j_content;
  HttpResponse();
  HttpResponse(const std::string& resp);
  HttpResponse(const std::string& resp, int fd);
  void set_header(std::string key, std::string value);
  void set_content(std::string cntnt);
  std::string dump();
};
