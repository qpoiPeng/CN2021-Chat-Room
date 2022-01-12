#include<unordered_map>
#include<vector>

std::vector<std::string> http_split_string(std::string s, std::string delim = " ") {
    std::vector<std::string> ret;
    int cur = 0, pos = 0;
    while ((pos = s.find(delim, cur)) != -1) {
        ret.push_back(s.substr(cur, pos - cur));
        cur = pos + delim.size();
    }
    if (s.substr(cur) != "")
        ret.push_back(s.substr(cur));
    return ret;
}

struct HttpRequest {
  std::unordered_map<std::string, std::string> header;
  std::string method;
  std::string path;
  std::string content;
  HttpRequest(std::string req) {
    std::vector<std::string> lines = http_split_string(req, "\r\n");
    std::vector<std::string> cur = http_split_string(lines[0]);
    method = cur[0]; path = cur[1];
    int i = 1;
    for (; i < lines.size(); ++i) {
      cur = http_split_string(lines[i], ": ");
      if (cur.size() < 1) break;
      header[cur[0]] = cur[1];
    }
    ++i;
    while (i < lines.size())
      content += lines[i++];
  }
  void show() {
    std::cout << method << '\n';
    std::cout << path << '\n';
    for (auto& h : header)
      std::cout << h.first << " : " << h.second << '\n';
    std::cout << "content:\n";
    std::cout << content << '\n';
  }
};
