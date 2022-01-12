#include<unordered_map>
#include<vector>
#include<sys/socket.h>
#include<nlohmann/json.hpp>
#include "../const.hpp"
#include "../Db_manager/Db_manager.hpp"

struct HttpRequest {
  std::unordered_map<std::string, std::string> header;
  std::string method, path, content;
  nlohmann::json j_content;
  HttpRequest(const std::string& req);
  HttpRequest(const std::string& req, int client_fd);
  void show();
};
