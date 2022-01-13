#include<unordered_map>
#include<vector>
#include<nlohmann/json.hpp>
#include "../const.hpp"

struct HttpResponse {
  std::unordered_map<std::string, std::string> header;
  std::string content;
  nlohmann::json j_content;
  HttpResponse();
  HttpResponse(std::string content);
  void set_header(std::string key, std::string value);
  std::string dump();
};

/*
  Sample Usage:

  HttpResponse resp("Success");
  std::string output = resp.dump();
  
 */
