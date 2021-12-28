#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<cstdio>
#include<unistd.h>
#include<errno.h>
#include<string>
#include<assert.h>
#include<dirent.h>
#include<iostream>
#include<cstring>
#include "mysig.hpp"
constexpr int BUF_SIZE = 2048;
char buf[BUF_SIZE+1] = {};
char rbuf[BUF_SIZE+1] = {};  // only for recv.

struct Client {
  int conn_fd;
  char ip[128];
  unsigned short port;
  sockaddr_in sadr;
  socklen_t sadr_len;
};

void clear_buf() {
  memset(buf, 0, BUF_SIZE+1);
}
void clear_rbuf() {
  memset(rbuf, 0, BUF_SIZE+1);
}

/* return the socket file descriptor */
int init_client(Client *client, char ip[]) {
  std::string s = ip;
  client->port = std::stoi(s.substr(s.find(":") + 1, s.length() - s.find(":") - 1));
  strcpy(client->ip, s.substr(0, s.find(":")).c_str());
  client->sadr.sin_family = AF_INET;
  client->sadr.sin_addr.s_addr = inet_addr(client->ip);
  client->sadr.sin_port = htons(client->port);
  client->sadr_len = sizeof(client->sadr);
  if ((client->conn_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr, "create socket failed...\n");
    return -1;
  }
  if (connect(client->conn_fd, (sockaddr*)&client->sadr, sizeof(client->sadr)) < 0) {
    fprintf(stderr, "connect failed...\n");
    return -1;
  }
  DIR *dir = opendir("client_dir");
  if (dir) {
    closedir(dir);
  }
  if (errno == ENOENT) {
    mkdir("client_dir", 0744);
  }
  return 0;
}

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

int main(int argc, char *argv[]) {
  Client client;
  std::string s;
  assert(argc > 1);
  if (init_client(&client, argv[1]) < 0)
    return -1;

  while (true) {
    clear_buf();
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
 
}
