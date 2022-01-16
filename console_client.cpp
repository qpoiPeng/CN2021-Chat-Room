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
#include "HttpParser/HttpParser.hpp"
#include "HttpParser/HttpResponse.hpp"
#include "Db_manager/Db_manager.hpp"
#include "nlohmann/json.hpp"
using json = nlohmann::json;

struct Client {
  int conn_fd;
  char ip[128];
  unsigned short port;
  sockaddr_in sadr;
  socklen_t sadr_len;
  std::string token;
  char buf[BUF_SIZE+1] = {};
  char rbuf[BUF_SIZE+1] = {};  // only for recv.
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
  int request(std::string method, std::string path, json& j) {
    sadr.sin_family = AF_INET;
    sadr.sin_addr.s_addr = inet_addr(ip);
    sadr.sin_port = htons(port);
    sadr_len = sizeof(sadr);
    if ((conn_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      fprintf(stderr, "create socket failed...\n");
      return -1;
    }
    if (connect(conn_fd, (sockaddr*)&sadr, sizeof(sadr)) < 0) {
      fprintf(stderr, "connect failed...\n");
      return -1;
    }
    std::string req = method + " " + path + " HTTP/1.1\r\n";
    req += "Accept: */*\r\n";
    req += "Connection: keep-alive\r\n";
    if (token != "")
      req += "Cookie: id=" + token + "\r\n";
    if (j.dump() != "null")
      req += "Content-Length: " + std::to_string(j.dump().size()) + "\r\n";
    req += "\r\n";
    if (j.dump() != "null")
      req += j.dump();
    std::cerr << req << '\n';
    send(conn_fd, req.c_str(), req.size(), 0);
    std::cerr << "HERE!\n";
    int offset = 0;
    while (recv(conn_fd, buf+offset, 1, 0)) {
      if (buf[offset++] == '\xff')
	break;
    }
    buf[--offset] = 0;
    req = buf;
    std::cerr << req << '\n';
    HttpResponse resp(req);
    resp.j_content = nlohmann::json::parse(resp.content);
    std::cerr << resp.dump() << '\n';
    return 0;
  }
};

Client client;
/*
void sign_up(int fd) {
  constexpr char username_prompt[] = "Username: ";
  constexpr char password_prompt[] = "Password: ";
  constexpr char password_prompt2[] = "Confirm password: ";
  std::string s;
  printf("%s", username_prompt);
  std::getline(std::cin, s);
  strncpy(buf, s.c_str(), BUF_SIZE);
  send(fd, buf, BUF_SIZE, 0);
  printf("%s", password_prompt);
  std::getline(std::cin, s);
  strncpy(buf, s.c_str(), BUF_SIZE);
  send(fd, buf, BUF_SIZE, 0);
  printf("%s", password_prompt2);
  std::getline(std::cin, s);
  strncpy(buf, s.c_str(), BUF_SIZE);
  send(fd, buf, BUF_SIZE, 0);
  clear_buf();
  recv(fd, buf, BUF_SIZE, MSG_WAITALL);
  printf("%s", buf);
}

int sign_in(int fd) {
  constexpr char username_prompt[] = "Username: ";
  constexpr char password_prompt[] = "Password: ";
  printf("%s", username_prompt);
  std::string s;
  std::getline(std::cin, s);
  strncpy(buf, s.c_str(), BUF_SIZE);
  send(fd, buf, BUF_SIZE, 0);
  printf("%s", password_prompt);
  std::getline(std::cin, s);
  strncpy(buf, s.c_str(), BUF_SIZE);
  send(fd, buf, BUF_SIZE, 0);

  clear_buf();
  recv(fd, buf, BUF_SIZE, MSG_WAITALL);
  s = buf;
  if (s == sig::sign_in_success) {
    printf("log in successfully!\n");
    return 0;
  }
  printf("Wrong username or password.\n");
  return -1;
}

int create_friend_request(int fd) {
  printf("\nWho do you want to send a friend request to?\nHis/Her name: ");
  std::string s;
  std::getline(std::cin, s);
  strncpy(buf, s.c_str(), BUF_SIZE);
  send(fd, buf, BUF_SIZE, 0);
  clear_buf();
  recv(fd, buf, BUF_SIZE, 0);
  s = buf;
  if (s == sig::ok) {
    printf("The friend request is sent successfully!\n");
  }
  else if (s == sig::request_self) {
    printf("You can't send a friend request to yourself!\n");
  }
  else if (s == sig::duplicated_request) {
    printf("The friend request has been sent before.\n");
  }
  else {
    printf("User not exists.\n");
  }
  return 0;
}

int home(int fd) {
  constexpr char prompt[] = "\nHome\n"
    " (1) List all friends\n"
    " (2) Send friend request\n"
    " (3) Confirm friend request(s)\n"
    " (4) Delete friend\n"
    " (5) Direct message\n"
    " (6) Log out and quit\n";

  std::string s;
  while (true) {
    printf("%s", prompt);
    std::getline(std::cin, s);
    strncpy(buf, s.c_str(), BUF_SIZE);
    send(fd, buf, BUF_SIZE, 0);
    clear_buf();
    recv(fd, buf, BUF_SIZE, 0);
    s = buf;
    if (s == sig::create_friend_request) {
      create_friend_request(fd);
    }
    else if (s == sig::quit) {
      break;
    }
  }

  return 0;
}

*/
int start() {
  constexpr char prompt[] = "(1) login  (2) register\n";
  constexpr char name_prompt[] = "username: ";
  constexpr char password_prompt[] = "password: ";

  json j;
  printf("%s", prompt);
  std::string s;
  std::getline(std::cin, s);
  if (s == "1") {
    std::string name, password;
    printf("%s", name_prompt);
    std::getline(std::cin, name);
    printf("%s", password_prompt);
    std::getline(std::cin, password);
    j["name"] = name;
    j["password"] = password;
    client.request("POST", "/login", j);
    return 0;
  }
  else if (s == "2") {

  }
  else return 1;
}

int main(int argc, char *argv[]) {
    assert(argc > 1);
    std::string s;
    assert(argc > 1);
    if (client.init(argv[1]) < 0)
      return -1;
    
    /* login */
    while (start());
    /*    
    while (true) {
        clear_buf();

        std::cerr << "Waiting for server message......" << std::endl;

        recv(client.conn_fd, buf, BUF_SIZE, MSG_WAITALL);
        printf("%s", buf);

        std::getline(std::cin, s);
        strncpy(buf, s.c_str(), BUF_SIZE);
        send(client.conn_fd, buf, BUF_SIZE, 0);
        clear_buf();
        recv(client.conn_fd, buf, BUF_SIZE, MSG_WAITALL);

        if ((s = buf) == sig::sign_up) {
            sign_up(client.conn_fd);
        }
        else if (s == sig::sign_in) {
            int res = sign_in(client.conn_fd);
            if (res == 0) {
                home(client.conn_fd);
                break;
            }
        }
        else if (s == sig::quit) {
            break;
        }
    }
    */
}
