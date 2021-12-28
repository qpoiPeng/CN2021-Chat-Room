#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<netinet/in.h>
#include<unistd.h>
#include<assert.h>
#include<dirent.h>
#include<errno.h>
#include<cstdio>
#include<cstring>
#include<string>
#include<vector>
#include<thread>
#include<mutex>
#include<set>
#include<algorithm>
#include "Db_manager/Db_manager.cpp"
#include "mysig.hpp"
constexpr int MAX_THREAD = 512;
constexpr int BUF_SIZE = 2048;
constexpr int NAMELEN = 64;
constexpr int MAX_FILE_COUNT = 1024;

constexpr char wrong_format[] = "Wrong format. Please try again.\n";

void _serve(int client_fd, int client_id);
struct Server {
  unsigned short port;
  sockaddr_in sadr;
  socklen_t sadr_len;
  int listen_fd;
  std::thread thr[MAX_THREAD] = {};
  std::mutex mtx, db_mtx;
  std::set<int> free_thread;
  std::set<int> used_thread;  // complement of free_thread.
  db::Db_manager db_manager;
  void lock() {
    mtx.lock();
  }
  void unlock() {
    mtx.unlock();
  }
  /* you should acquire mutex lock before using ALL the methods below !!! */
  void serve(int fd) {
    int id = *(free_thread.begin());
    free_thread.erase(id);
    used_thread.insert(id);
    thr[id] = std::thread(_serve, fd, id);
    thr[id].detach();
  }
};
Server server;

struct Client {
  std::string name;
  int fd;
  Client(int fd) {
    this->fd = fd;
  }
  int send(const char *buf) {
    int ret = ::send(fd, buf, BUF_SIZE, MSG_NOSIGNAL);
    if (ret < 0 && errno == EPIPE)
      return -1;
    return ret;
  }
  int recv(char *buf) {
    return ::recv(fd, buf, BUF_SIZE, MSG_WAITALL);
  }
  int sign_up() {
    std::string username, password, password2;
    char buf[BUF_SIZE+1] = {};
    if (send(sig::sign_up) < 0) 
      return -1;

    if (recv(buf) <= 0)
      return -1;
    username = buf;

    memset(buf, 0, BUF_SIZE+1);
    if (recv(buf) <= 0)
      return -1;
    password = buf;

    memset(buf, 0, BUF_SIZE+1);
    if (recv(buf) <= 0)
      return -1;
    password2 = buf;
  
    if (password != password2) {
      constexpr char password_prompt2[] = "password error.\n";
      if (send(password_prompt2) < 0)
	return -1;
    }
    else {
      server.db_mtx.lock();
      db::status res = server.db_manager.sign_up(username, password);
      server.db_mtx.unlock();
      if (res == db::status::OK) {
	constexpr char success[] = "Create account successfully!\n";
	if (send(success) < 0)
	  return -1;
      }
      else {
	constexpr char failed[] = "The username is used. Please try again.\n";
	if (send(failed) < 0)
	  return -1;
      }
    }
    return 0;
  }
  
  int sign_in() {
    if (send(sig::sign_in) < 0)
      return -1;
    char buf[BUF_SIZE+1] = {};
    std::string username, password;

    if (recv(buf) <= 0)
      return -1;
    username = buf;

    memset(buf, 0, BUF_SIZE+1);
    if (recv(buf) <= 0)
      return -1;
    password = buf;

    db::status res = server.db_manager.sign_in(username, password);
    if (res == db::status::OK) {
      if (send(sig::sign_in_success) < 0)
	return -1;
      this->name = username;
      return 0;
    }
    // else
    if (send(sig::sign_in_fail) < 0)
      return -1;
    return 1;
  }

  void home() {
    char buf[BUF_SIZE+1] = {};
    std::string s;

    while (true) {
      if (recv(buf) <= 0)
	break;
      s = buf;
      if (s == "1") {  // list friend
      
      }
      else if (s == "2") {  // send request
	if (friend_request() < 0)
	  break;
      }
      else if (s == "3") {  // confirm request
	
      }
      else if (s == "4") {  // delete friend

      }
      else if (s == "5") {  // direct message

      }
      else if (s == "6") {  // quit
	send(sig::quit);
	break;
      }
      else {
	if (send(wrong_format) < 0)
	  break;
      }
    }
  }

  int friend_request() {
    if (send(sig::create_friend_request) < 0)
      return -1;
    char buf[BUF_SIZE+1] = {};
    std::string destination;
    if (recv(buf) <= 0)
      return -1;
    destination = buf;
    if (destination == name) {
      if (send(sig::request_self) < 0)
	return -1;
    }
    else if (server.db_manager.if_user_exists(destination) == db::status::USER_NOT_EXISTS) {
      if (send(sig::user_not_exists) < 0)
	return -1;
    }
    else {
      if (server.db_manager.create_friend_request(name, destination) == db::status::DUPLICATED_REQUEST) {
	if (send(sig::duplicated_request) < 0)
	  return -1;
      }
      if (send(sig::ok) < 0)
	return -1;
    }
    return 0;
  }

  int confirm_friend_request() {
    if (send(sig::confirm_friend_request) < 0)
      return -1;
    std::vector<std::string> request_list;
    server.db_manager.get_friend_request_list(name, request_list);    // no check yet!
    sort(request_list.begin(), request_list.end());
    int size = request_list.size();
    std::string s;
    char buf[BUF_SIZE+1] = {};
    sprintf(buf, "%d", size);
    send(buf);
    for (int i = 0; i < size; ++i) {
      sprintf(buf, "%s", request_list[i].c_str());
      send(buf);
    }
    int idx = 0;
    if (size > 0) {
      memset(buf, 0, BUF_SIZE+1);
      recv(buf);
      try {
	s = buf;
	idx = stoi(s);
      }
      catch (std::exception e) {
	send(sig::wrong_format);
	return 0;
      }
    }
    if (idx > size || idx < -size) {
      send(sig::wrong_format);
      return 0;
    }
    
    return 0;
  }

};

int init_server(Server *server) {
  if ((server->listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr, "create socket failed...\n");
    return -1;
  }
  bzero(&server->sadr, sizeof(server->sadr));
  server->sadr.sin_family = AF_INET;
  server->sadr.sin_addr.s_addr = htonl(INADDR_ANY);
  server->sadr.sin_port = htons(server->port);
  server->sadr_len = sizeof(server->sadr);
  for (int i = 0; i < MAX_THREAD; ++i)
    server->free_thread.insert(i);
  if (bind(server->listen_fd, (sockaddr*)&(server->sadr), sizeof(server->sadr)) < 0) {
    fprintf(stderr, "bind failed...\n");
    return -1;
  }
  if (listen(server->listen_fd, 1024) < 0) {
    fprintf(stderr, "listen failed...\n");
    return -1;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  assert(argc > 1);
  server.port = atoi(argv[1]);
  if (init_server(&server) < 0) // error
    return -1;
  int client_fd;
  while (1) {
    client_fd = accept(server.listen_fd, (sockaddr*) &(server.sadr), &(server.sadr_len));
    server.lock();
    server.serve(client_fd);
    server.unlock();
    fprintf(stderr, "new connection %d\n", client_fd);
  }
}

void cleanup(int id) {
  server.lock();
  server.used_thread.erase(id);
  server.free_thread.insert(id);
  server.unlock();
  fprintf(stderr, "%d disconnected\n", id);
}

std::vector<std::string> split_string(std::string s, std::string delim = " ") {
  std::vector<std::string> ret;
  int cur = 0, pos = 0;
  while ((pos = s.find(delim, cur)) != -1) {
    ret.push_back(s.substr(cur, pos - cur));
    cur = pos + 1;
  }
  if (s.substr(cur) != "")
    ret.push_back(s.substr(cur));
  return ret;
}

std::string merge_string(std::vector<std::string> v, std::string delim = ",") {
  std::string ret;
  for (auto s : v)
    ret += s + delim;
  ret = ret.substr(0, std::max((int) ret.length() - (int)delim.length(), 0));
  return ret;
}

void _serve(int client_fd, int client_id) {
  fprintf(stderr, "new client served, client_fd : %d, client_id : %d\n", client_fd, client_id);
  constexpr char welcome[] = "(1) Login (2) Register (3) Quit\n";
  char buf[BUF_SIZE+1] = {};
  std::string s;
  Client *client = new Client(client_fd);
  while (true) {
    if (client->send(welcome) < 0)
      goto THREAD_END;
    
    memset(buf, 0, BUF_SIZE+1);
    if (client->recv(buf) <= 0)
      goto THREAD_END;
    s = buf;
    if (s == "1") { // login
      int res = client->sign_in();
      if (res < 0)  // disconnect
	break;
      else if (res == 0) {  // success
	client->home();
	break;
      }
      // fail and try again.
    }
    else if (s == "2") {  // register
      if (client->sign_up() < 0)
	break;
    }
    else if (s == "3") {  // quit
      if (client->send(sig::quit) < 0)
	goto THREAD_END;
      break;
    }
    else {  // wrong input
      if (client->send(wrong_format) < 0)
	goto THREAD_END;
    }
  }
  delete client;
 THREAD_END:
  cleanup(client_id);
  close(client_fd);
}

