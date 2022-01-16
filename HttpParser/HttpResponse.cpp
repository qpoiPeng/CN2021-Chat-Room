#include "HttpResponse.hpp"

HttpResponse::HttpResponse() {
  header["Cache-Control"] = "no-cache, private";
  header["Content-Type"] = "text/html";
}

HttpResponse::HttpResponse(const std::string& resp) : HttpResponse() {
  std::vector<std::string> lines = db::split_string(resp, "\r\n");
  std::vector<std::string> cur = db::split_string(lines[0]);
  status_code = cur[1];
  int i = 1;
  for (; i < lines.size(); ++i) {
    cur = db::split_string(lines[i], ": ");
    if (cur.size() < 1) break;
    header[cur[0]] = cur[1];
  }
  ++i;
  while (i < lines.size())
    content += lines[i++];
}

HttpResponse::HttpResponse(const std::string& resp, int fd) : HttpResponse(resp) {
  char buf[BUF_SIZE+1];
  if (header.find("Content-Length") != header.end()) {
    while (content.size() < std::stoi(header["Content-Length"])) {
      memset(buf, 0, BUF_SIZE+1);
      if (recv(fd, buf, BUF_SIZE, 0) <= 0) break;
      content += buf;
    }
    j_content = nlohmann::json::parse(content);
  }
}

void HttpResponse::set_header(std::string key, std::string value) {
  header[key] = value;
}

void HttpResponse::set_content(std::string cntnt) {
  header["Content-Length"] = std::to_string(cntnt.size() + 2);
  content = cntnt;
}

std::string HttpResponse::dump() {
  std::string rsp = "HTTP/1.1 200 OK\r\n";
  for (auto&& key : header)
    rsp += key.first + ": " + key.second + "\r\n";
  rsp += "\r\n" + content + "\r\n";
  rsp += "\xff";
  return rsp;
}
