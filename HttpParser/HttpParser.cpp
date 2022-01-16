#include "HttpParser.hpp"
#include <fstream>

void HttpRequest::show() {
  std::cout << method << '\n';
  std::cout << path << '\n';
  for (auto& h : header)
    std::cout << h.first << " : " << h.second << '\n';
  std::cout << "content : \n";
  if(!isFile) {
    std::cout << content << '\n';
    std::cout << "content size read: " << content.size() << '\n';
  }
  else {
    // for(auto ch : raw_content)
      // std::cout << ch;
    // std::cout << std::endl;
    std::cout << "raw content size read: " << raw_content.size() << '\n';
  }
  std::cout << j_content << '\n';
}

HttpRequest::HttpRequest(const std::string& req) {
  std::vector<std::string> lines = db::split_string(req, "\r\n");
  std::vector<std::string> cur = db::split_string(lines[0]);
  header["Origin"] = "0.0.0.1";
  curpos = lines[0].size() + 2;
  method = cur[0]; path = cur[1];
  int i = 1;
  for (; i < lines.size(); ++i) {
    curpos += lines[i].size() + 2;
    cur = db::split_string(lines[i], ": ");
    if (cur.size() < 1) break;
    header[cur[0]] = cur[1];
  }
  if (path.substr(0, 5) == "/file" && method == "POST") return;
  ++i;
  while (i < lines.size())
    content += lines[i++];
}

HttpRequest::HttpRequest(const std::string& req, int client_fd) : HttpRequest(req) {
  char buf[BUF_SIZE+1];
  isFile = false;

  if(header.find("Content-Type") != header.end() && header["Content-Type"].find("multipart/form-data") != std::string::npos)
    isFile = true;

  if (header.find("Content-Length") != header.end()) {
    if(!isFile) {
      while (content.size() < std::stoi(header["Content-Length"])) {
        memset(buf, 0, BUF_SIZE+1);
        if (recv(client_fd, buf, BUF_SIZE, 0) <= 0) break;
        content += buf;
      }
    }
    else {
      while (raw_content.size() < std::stoi(header["Content-Length"])) {
        memset(buf, 0, BUF_SIZE+1);
        int size = 0;
        if ((size = recv(client_fd, buf, BUF_SIZE, 0)) <= 0) break;
        for(int i = 0; i < size; ++i)
          raw_content.push_back(buf[i]);
      }
    }
    if(header.find("Content-Type") != header.end() && header["Content-Type"] == "application/json")
      j_content = nlohmann::json::parse(content);
  }
}

int HttpRequest::download(std::string& filetoken, db::Db_manager& db_manager) {

  std::string content(raw_content.begin(), raw_content.end());

  std::vector<std::string> cur, lines = db::split_string(content, "\r\n");
  std::unordered_map<std::string, std::string> form_header;
  std::string file_content;

  curpos = lines[0].size() + 2;
  int i = 1;

  for (; i < lines.size(); ++i) {
    curpos += lines[i].size() + 2;
    cur = db::split_string(lines[i], ": ");
    if (cur.size() < 1) break;
    form_header[cur[0]] = cur[1];
  }

  ++i;
  while (i < lines.size() - 1)
    file_content += lines[i++];

  // Add filename and file token to db
  std::string filename;
  std::vector<std::string> content_dispositions = db::split_string(form_header["Content-Disposition"], "; ");
  for(auto ct : content_dispositions) {
    if(ct.find("filename") != std::string::npos) {
      std::vector<std::string> _tmp = db::split_string(ct, "=");
      filename = _tmp[1];
      filename = filename.substr(1, filename.size() - 2);
    }
  }

  db_manager.add_file(filename, filetoken);

  std::ofstream write;
  filetoken += filename;
  std::string file_path = "server_dir/" + filetoken;
  write.open(file_path.c_str(), std::ios::out | std::ios::binary);
  write.write(file_content.c_str(), sizeof(char)*file_content.size());

  return 1;
}
