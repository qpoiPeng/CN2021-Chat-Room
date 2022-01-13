#include "HttpParser.hpp"

void HttpRequest::show() {
  std::cout << method << '\n';
  std::cout << path << '\n';
  for (auto& h : header)
    std::cout << h.first << " : " << h.second << '\n';
  std::cout << "content : \n";
  std::cout << content << '\n';
  std::cout << "content size : " << content.size() << '\n';
  std::cout << j_content << '\n';
}

HttpRequest::HttpRequest(const std::string& req) {
  std::vector<std::string> lines = db::split_string(req, "\r\n");
  std::vector<std::string> cur = db::split_string(lines[0]);
  method = cur[0]; path = cur[1];
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

HttpRequest::HttpRequest(const std::string& req, int client_fd) : HttpRequest(req) {
  char buf[BUF_SIZE+1];
  if (header.find("Content-Length") != header.end()) {
    while (content.size() < std::stoi(header["Content-Length"])) {
      memset(buf, 0, BUF_SIZE+1);
      if (recv(client_fd, buf, BUF_SIZE, 0) <= 0) break;
      content += buf;
    }
    j_content = nlohmann::json::parse(content);
  }
}
