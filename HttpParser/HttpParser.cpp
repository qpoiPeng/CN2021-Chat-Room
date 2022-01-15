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
  curpos = lines[0].size() + 2;
  method = cur[0]; path = cur[1];
  int i = 1;
  for (; i < lines.size(); ++i) {
    curpos += lines[i].size() + 2;
    cur = db::split_string(lines[i], ": ");
    if (cur.size() < 1) break;
    header[cur[0]] = cur[1];
  }
  if (path == "/file") return;
  ++i;
  while (i < lines.size())
    content += lines[i++];
}

HttpRequest::HttpRequest(const std::string& req, int client_fd) : HttpRequest(req) {
  char buf[BUF_SIZE+1];
  if (header.find("Content-Length") != header.end() && path.substr(0, 5) != "/file") {
    while (content.size() < std::stoi(header["Content-Length"])) {
      memset(buf, 0, BUF_SIZE+1);
      if (recv(client_fd, buf, BUF_SIZE, 0) <= 0) break;
      content += buf;
    }
    j_content = nlohmann::json::parse(content);
  }
}

int HttpRequest::download(const char* req, int client_fd) {
  char buf[BUF_SIZE+1];
  memcpy(buf, req, BUF_SIZE+1);
  int bytes = 0, recv_size = BUF_SIZE, tmp;
  FILE *fp = fopen("123", "wb");    // TODO: create proper file name.
  while (bytes < stoi(header["Content-Length"])) {
    if (curpos == recv_size) {
      curpos = 0;
      memset(buf, 0, BUF_SIZE+1);
      recv_size = recv(client_fd, buf, BUF_SIZE, 0);
      if (recv_size <= 0) return -1;
    }
    //    tmp = fwrite(buf, 1, std::min(header["Content-Length"]-curpos, recv_size-curpos), fp);
    //    bytes += tmp;
    //    curpos += tmp;
    fputc(buf[curpos++], fp);
    ++bytes;
  }
  fclose(fp);
  return 0;
}