#include "HttpResponse.hpp"

HttpResponse::HttpResponse() {
  header["Cache-Control"] = "no-cache, private";
  header["Content-Type"] = "text/plain";
  header["Access-Control-Allow-Origin"] = "*";
  header["Access-Control-Allow-Methods"] = "*";
  header["Access-Control-Allow-Headers"] = "*";
}

HttpResponse::HttpResponse(std::string cntnt) : HttpResponse() {
  header["Content-Length"] = std::to_string(cntnt.size());
  content = cntnt;
}

void HttpResponse::set_header(std::string key, std::string value) {
  header[key] = value;
}

std::string HttpResponse::dump() {
  std::string rsp = "HTTP/1.1 200 OK\r\n";
  for (auto&& h : header)
    rsp += h.first + ": " + h.second + "\r\n";
  rsp += "\r\n" + content + "\r\n";
  return rsp;
}
