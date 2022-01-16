#include "mysig.hpp"
#include "const.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstdio>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <assert.h>
#include <dirent.h>
#include <iostream>
#include <cstring>
#include <thread>
#include <mutex>
#include "HttpParser/HttpParser.hpp"
#include "HttpParser/HttpResponse.hpp"
#include "Db_manager/Db_manager.hpp"
#include "nlohmann/json.hpp"
using json = nlohmann::json;

std::mutex mtx;
bool readMsg = false;
struct Client {
  int conn_fd;
  char ip[128] = {};
  unsigned short port;
  sockaddr_in sadr;
  socklen_t sadr_len;
  std::string token;
  char buf[BUF_SIZE+1] = {};
  int init(char ip[]) {
    std::string s = ip;
    strncpy(this->ip, ip, 127);
    token = "";
    port = std::stoi(s.substr(s.find(":") + 1, s.length() - s.find(":") - 1));
    strcpy(this->ip, s.substr(0, s.find(":")).c_str());
    DIR *dir = opendir("client_dir");
    if (dir) {
      closedir(dir);
    }
    if (errno == ENOENT) {
      mkdir("client_dir", 0744);
    }
    return 0;
  }
  int send_file(FILE* fp, std::string filename, std::string frd) {
    json j;
    struct stat st;
    std::string filepath = "client_dir/" + filename;
    stat(filepath.c_str(), &st);
    int size = st.st_size, bytes = 0, tmp = 0;
    j["filename"] = filename;
    j["friend_name"] = frd;
    j["size"] = size;
    request("POST", "/console/upload", j, false);
    sadr.sin_family = AF_INET;
    sadr.sin_addr.s_addr = inet_addr(ip);
    sadr.sin_port = htons(port);
    sadr_len = sizeof(sadr);
    while (bytes < size) {
      memset(buf, 0, BUF_SIZE+1);
      bytes += fread(buf, 1, std::min(size - bytes, BUF_SIZE), fp);
      if (send(conn_fd, buf, BUF_SIZE, 0) < 0)
	return -1;
    }
    close(conn_fd);
    return 0;
  }
  HttpResponse request(std::string method, std::string path, json& j, bool needclose = true) {
    sadr.sin_family = AF_INET;
    sadr.sin_addr.s_addr = inet_addr(ip);
    sadr.sin_port = htons(port);
    sadr_len = sizeof(sadr);
    if ((conn_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      fprintf(stderr, "create socket failed...\n");
      exit(-1);
    }
    if (connect(conn_fd, (sockaddr*)&sadr, sizeof(sadr)) < 0) {
      fprintf(stderr, "connect failed...\n");
      exit(-1);
    }
    std::string req = method + " " + path + " HTTP/1.1\r\n";
    req += "Accept: */*\r\n";
    req += "Connection: keep-alive\r\n";
    if (token != "")
      req += "Cookie: " + token + "\r\n";
    if (j.dump() != "null")
      req += "Content-Length: " + std::to_string(j.dump().size()) + "\r\n";
    req += "\r\n";
    if (j.dump() != "null")
      req += j.dump();
    send(conn_fd, req.c_str(), req.size(), 0);
    int offset = 0;
    while (recv(conn_fd, buf+offset, 1, 0)) {
      if (buf[offset++] == '\xff')
	break;
    }
    if (offset != 0)
      buf[--offset] = 0;
    req = buf;
    HttpResponse resp(req);
    resp.j_content = nlohmann::json::parse(resp.content);
    if (resp.header.find("Set-Cookie") != resp.header.end())    // token
      token = resp.header["Set-Cookie"];
    if (needclose)
      close(conn_fd);
    return resp;
  }
};

Client client;
void getMsg(std::string name) {
  Client c2;
  strcpy(c2.ip, client.ip);
  c2.port = client.port;
  c2.token = client.token;
  json j;
  HttpResponse resp;
  resp = c2.request("GET", "/chat/"+name, j);
  for (int i = 0; i < resp.j_content["messages"].size(); ++i) {
    std::cout << resp.j_content["messages"][i]["from"] << " : " << resp.j_content["messages"][i]["content"];
    if (resp.j_content["messages"][i]["type"] == "file")
      std::cout << "[FILE]\n";
    else 
      std::cout << "\n";
  }
  sleep(3);
  while (readMsg) {
    resp = c2.request("GET", "/unread/"+name, j);
    for (int i = 0; i < resp.j_content["messages"].size(); ++i) {
      std::cout << resp.j_content["messages"][i]["from"] << " : " << resp.j_content["messages"][i]["content"];
      if (resp.j_content["messages"][i]["type"] == "file")
	std::cout << "[FILE]\n";
      else 
	std::cout << "\n";
    }
    sleep(1);
  }
}

void chatroom(std::string frd) {
  readMsg = true;
  std::thread thr(getMsg, frd);
  std::string s, path;
  path = "/chat/" + frd;
  sleep(1);
  constexpr char prompt[] = "(1) Type text  (2) Send file  (3) quit\n";
  HttpResponse resp;
  json j;
  while (true) {
    std::cout << prompt;
    std::getline(std::cin, s);
    if (s == "1") {
      std::getline(std::cin, s);
      j["message"] = s;
      mtx.lock();
      resp = client.request("POST", path, j);
      mtx.unlock();
    }
    else if (s == "2") {
      std::getline(std::cin, s);
      std::string filepath = "client_dir/" + s;
      FILE* fp = fopen(filepath.c_str(), "r");
      if (!fp) {
	std::cout << "Open file failed.\n";
	continue;
      }

      client.send_file(fp, s, frd);
      fclose(fp);
      std::cout << "SEND FINISH\n";
    }
    else {
      break;
    }
  }
  readMsg = false;
  thr.join();
}

int home() {
  constexpr char prompt[] = "\n===================================\nHome\n"
    " (1) List all friends\n"
    " (2) Send friend request\n"
    " (3) Confirm friend request\n"
    " (4) Delete friend\n"
    " (5) Direct message\n"
    " (6) Download file by link\n"
    " (7) Log out and quit\n===================================\n";

  std::string s;
  printf("%s", prompt);
  std::getline(std::cin, s);
  HttpResponse resp;
  json j;
  if (s == "1") {
    resp = client.request("GET", "/friends", j);
    if (resp.j_content["status"] == "Success") {
      std::vector<std::string> fl = resp.j_content["friends"];
      for (auto& f : fl) {
	std::cout << f << '\n';
      }
    }
    else {
      s = resp.j_content["status"];
      std::cout << s << '\n';
    }
  }
  else if (s == "2") {
    std::string name;
    std::cout << "Who do you want to send friend request?\n";
    std::getline(std::cin, name);
    j["action"] = "add";
    j["friend_name"] = name;
    resp = client.request("POST", "/friends", j);
    if (resp.j_content["status"] == "Success") {
      std::cout << "Send friend request to " << name << " successfully!\n";
    }
    else {
      s = resp.j_content["status"];
      std::cout << s << '\n';
    }
  }
  else if (s == "3") {
    resp = client.request("GET", "/friends/requests", j);
    if (resp.j_content["status"] == "Success") {
      std::vector<std::string> reqlist = resp.j_content["request list"];
      if (reqlist.size() == 0) {
	std::cout << "No friend request now!\n";
	return 0;
      }
      else {
	for (auto& f : reqlist) {
	  std::cout << f << '\n';
	}
	std::cout << "\n(1) Accept a friend request  (2) Decline a friend request  (3) Cancel\n";
	std::getline(std::cin, s);
	if (s == "1") {
	  std::cout << "Which request do you want to accept?\n";
	  std::getline(std::cin, s);
	  j["action"] = "accept";
	  j["friend_name"] = s;
	  resp = client.request("POST", "/friends/requests", j);
	  if (resp.j_content["status"] == "Success") {
	    std::cout << "Request accepted!\n";
	  }
	  else {
	    s = resp.j_content["status"];
	    std::cout << s << '\n';
	  }
	}
	else if (s == "2") {
	  std::cout << "Which request do you want to decline?\n";
	  std::getline(std::cin, s);
	  j["action"] = "reject";
	  j["friend_name"] = s;
	  resp = client.request("POST", "/friends/requests", j);
	  if (resp.j_content["status"] == "Success") {
	    std::cout << "Request rejected!\n";
	  }
	  else {
	    s = resp.j_content["status"];
	    std::cout << s << '\n';
	  }
	}
	else return 0;
      }
    }
  }
  else if (s == "4") {
    std::cout << "Who do you want to unfriend?\n";
    std::getline(std::cin, s);
    j["action"] = "delete";
    j["friend_name"] = s;
    resp = client.request("POST", "/friends", j);
    if (resp.j_content["status"] == "Success") {
      std::cout << s << " is no longer your friend.\n";
    }
    else {
      s = resp.j_content["status"];
      std::cout << s << '\n';
    }
  }
  else if (s == "5") {
    std::vector<std::string> fl;
    resp = client.request("GET", "/friends", j);
    if (resp.j_content["status"] == "Success") {
      std::vector<std::string> fl = resp.j_content["friends"];
      for (int i = 0; i < fl.size(); ++i) {
	printf("(%d)  %s\n", i+1, fl[i].c_str());
      }
      std::cout << "Who do you want to chat with?\n";
      std::getline(std::cin, s);
      int id;
      try {
	id = std::stoi(s);
      }
      catch (std::exception e) {
	std::cout << "Please enter a valid index.\n";
	return 0;
      }
      if (id - 1 < 0 || id > fl.size()) {
	std::cout << "Please enter a valid index.\n";
	return 0;
      }
      chatroom(fl[id-1]);
    }
  }
  else if (s == "6") {
    std::cout << "Input the file token:\n";
    std::getline(std::cin, s);
    j["filetoken"] = s;
    resp = client.request("POST", "/console/download", j, false);
    std::string filename;
    filename = resp.j_content["filename"];
    int size = resp.j_content["size"], bytes = 0, tmp;
    std::string filepath = "client_dir/" + filename;
    FILE* fp = fopen(filepath.c_str(), "wb");
    char buf[BUF_SIZE+1] = {};
    while (bytes < size) {
      memset(buf, 0, BUF_SIZE+1);
      recv(client.conn_fd, buf, BUF_SIZE, MSG_WAITALL);
      tmp = fwrite(buf, 1, std::min(size-bytes, BUF_SIZE), fp);
      bytes += tmp;
    }
    fclose(fp);
    std::cout << "Download successfully!\n";
    close(client.conn_fd);
  }
  else if (s == "7") {
    std::cout << "Goodbye!\n";
    return 1;
  }
  else {
    std::cout << "Please enter a single number.\n";
  }
  return 0;
}

int start() {
  constexpr char prompt[] = "(1) login  (2) register\n";
  constexpr char name_prompt[] = "username: ";
  constexpr char password_prompt[] = "password: ";

  json j;
  printf("%s", prompt);
  std::string s;
  std::getline(std::cin, s);
  HttpResponse resp;
  if (s == "1") {
    std::string name, password;
    printf("%s", name_prompt);
    std::getline(std::cin, name);
    printf("%s", password_prompt);
    std::getline(std::cin, password);
    j["name"] = name;
    j["password"] = password;
    resp = client.request("POST", "/login", j);
    if (resp.j_content["status"] == "Success") {
      s = j["name"];
      std::cout << "Welcome! " << s << '\n';
      return 1;
    }
    else {
      s = resp.j_content["status"];
      std::cout << s << '\n';
    }
    return 0;
  }
  else if (s == "2") {
    std::string name, password;
    printf("%s", name_prompt);
    std::getline(std::cin, name);
    printf("%s", password_prompt);
    std::getline(std::cin, password);
    j["name"] = name;
    j["password"] = password;
    resp = client.request("POST", "/register", j);
    if (resp.j_content["status"] == "Success") {
      s = j["name"];
      std::cout << "Register successfully! " << s << '\n';
    }
    else {
      s = resp.j_content["status"];
      std::cout << s << '\n';
    }
    return 0;
  }
  else return 0;
}

int main(int argc, char *argv[]) {
    assert(argc > 1);
    std::string s;
    if (client.init(argv[1]) < 0)
      return -1;
    
    /* login */
    while (!start());
    while (!home());
}
