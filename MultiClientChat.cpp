#include "MultiClientChat.hpp"
#include <string>
#include <istream>
#include <sstream>
#include <fstream>
#include <vector>
#include <streambuf>
#include <iterator>
#include <climits>
#include <sys/stat.h>
#include "HttpParser/HttpResponse.hpp"

using json = nlohmann::json;

#define CHECK_LOGIN if (hr.header.find("Cookie") == hr.header.end()) {	\
    j["status"] = "NOT LOGIN";						\
    resp.set_content(j.dump());						\
    send_to_client(client_socket, resp.dump().c_str(), resp.dump().size()); \
    return 0;								\
  }
#define CHECK_USER if (user == "") {					\
    j["status"] = "NO USER";						\
    resp.set_content(j.dump());						\
    send_to_client(client_socket, resp.dump().c_str(), resp.dump().size()); \
    return 0;								\
  }

bool timecmp(db::Message& a, db::Message& b) {
  return a.timestamp < b.timestamp;
}

std::string MultiClientChat::get_user(HttpRequest& hr) {
  int pos = hr.header["Cookie"].find("id=");
  std::string user;
  if (pos != -1) {
    int endpos = hr.header["Cookie"].find(";", pos+1);
    endpos = endpos == -1? INT_MAX : endpos;
    std::string token = hr.header["Cookie"].substr(pos+3, endpos-pos-3);
    db_manager.token2name(token, user);
  }
  return user;
}

// Handler for when a message is received from the client
int MultiClientChat::on_message_received(int client_socket, const char *msg, int length) {

  HttpRequest hr(msg, client_socket);
  hr.show();
  HttpResponse resp;
  json j;

  // Handle CORS
  // if(hr.header.find("Sec-Fetch-Mode") != hr.header.end() && hr.header["Sec-Fetch-Mode"] == "cors") {
    resp.set_header("Access-Control-Allow-Origin", hr.header["Origin"]);
    resp.set_header("Access-Control-Allow-Methods", "*");
    resp.set_header("Access-Control-Allow-Headers", "content-type");
    resp.set_header("Access-Control-Allow-Credentials", "true");

  // }

  if(hr.method == "OPTIONS")
    send_to_client(client_socket, resp.dump().c_str(), resp.dump().size());
  else if (hr.path == "/register") {
    db::status res = db_manager.sign_up(hr.j_content["name"], hr.j_content["password"]);
    if (res == db::status::OK) {
      j["status"] = "Success";
    }
    else {
      j["status"] = "Username exists";
    }
    resp.set_content(j.dump());
    send_to_client(client_socket, resp.dump().c_str(), resp.dump().size());
  }
  else if (hr.path == "/login") {
    db::status res = db_manager.sign_in(hr.j_content["name"], hr.j_content["password"]);
    if (res == db::status::OK) {
      std::string name = hr.j_content["name"], token;
      db_manager.create_token(name, token);
      token = "id=" + token + ";";
      resp.set_header("Set-Cookie", token);
      j["status"] = "Success";
    }
    else {
      j["status"] = "Failed";
    }
    resp.set_content(j.dump());
    send_to_client(client_socket, resp.dump().c_str(), resp.dump().size());
  }
  // * Get friend list of user
  else if (hr.path == "/friends" && hr.method == "GET") {
    CHECK_LOGIN;
    std::string user = get_user(hr);
    CHECK_USER;
    std::vector<std::string> friends;
    db::status res = db_manager.get_friend_list(user, friends);
    if (res == db::status::OK) {
      j["status"] = "Success";
      j["friends"] = friends;
    }
    else {
      j["status"] = "Failed";
    }
    resp.set_content(j.dump());
    send_to_client(client_socket, resp.dump().c_str(), resp.dump().size());
  }
  // * POST: add friends (send friend requests) or delete friends
  else if (hr.path == "/friends" && hr.method == "POST") {
    CHECK_LOGIN;
    std::string user = get_user(hr);
    CHECK_USER;
    if (hr.j_content["action"] == "add") {
      db::status res = db_manager.create_friend_request(user, hr.j_content["friend_name"]);
      if (res == db::status::OK) {
	j["status"] = "Success";
      }
      else if (res == db::status::DUPLICATED_REQUEST) {
	j["status"] = "Duplicated request";
      }
      else if (res == db::status::IS_FRIEND_ALREADY) {
	j["status"] = "Is friend already";
      }
    }
    else if (hr.j_content["action"] == "delete") {
      db::status res = db_manager.delete_friend(user, hr.j_content["friend_name"]);
      if (res == db::status::OK) {
	j["status"] = "Success";
      }
      else {
	j["status"] = "Failed";
      }
    }
    else {
      j["status"] = "??";
    }
    resp.set_content(j.dump());
    send_to_client(client_socket, resp.dump().c_str(), resp.dump().size());
  }
  // * GET friend request list
  else if (hr.path == "/friends/requests" && hr.method == "GET") {
    CHECK_LOGIN;
    std::string user = get_user(hr);
    CHECK_USER;
    std::vector<std::string> reqlist;
    db::status res = db_manager.get_friend_request_list(user, reqlist);
    if (res == db::status::OK) {
      j["status"] = "Success";
      j["request list"] = reqlist;
    }
    else
      j["status"] = "Failed";
    resp.set_content(j.dump());
    send_to_client(client_socket, resp.dump().c_str(), resp.dump().size());
  }
  // * Accept or reject friend requests
  else if (hr.path == "/friends/requests" && hr.method == "POST") {
    CHECK_LOGIN;
    std::string user = get_user(hr);
    CHECK_USER;
    db::status res;
    if (hr.j_content["action"] == "accept") {
      res = db_manager.confirm_friend_request(user, hr.j_content["friend_name"]);
    }
    else if (hr.j_content["action"] == "reject") {
      // no check if to-be-deleted person exists
      res = db_manager.reject_friend_request(user, hr.j_content["friend_name"]);
    }
    if (res == db::status::OK)
      j["status"] = "Success";
    else
      j["status"] = "Failed";
    resp.set_content(j.dump());
    send_to_client(client_socket, resp.dump().c_str(), resp.dump().size());
  }
  // * Get chat history
  else if (hr.path.substr(0, 5) == "/chat" && hr.method == "GET") {
    CHECK_LOGIN;
    std::string user = get_user(hr), frd = hr.path.substr(6);
    CHECK_USER;
    std::vector<db::Message> chat;
    db::status res = db_manager.get_chat(user, frd, chat);
    if (res == db::status::OK) {
      sort(chat.begin(), chat.end(), timecmp);
      j["status"] = "Success";
      for (int i = 0; i < chat.size(); ++i) {
	if (chat[i].type == "user") continue;
	json tmp;
	tmp["type"] = chat[i].type;
	tmp["timestamp"] = chat[i].timestamp;
	tmp["from"] = chat[i].from;
	tmp["to"] = chat[i].to;
	tmp["content"] = chat[i].content;
	j["messages"].push_back(tmp);
      }
    }
    resp.set_content(j.dump());
    send_to_client(client_socket, resp.dump().c_str(), resp.dump().size());
  }
  else if (hr.path.substr(0, 7) == "/unread") {
    CHECK_LOGIN;
    std::string user = get_user(hr), frd = hr.path.substr(8);
    CHECK_USER;
    std::vector<db::Message> chat;
    db::status res = db_manager.get_unread(user, frd, chat);
    if (res == db::status::OK) {
      sort(chat.begin(), chat.end(), timecmp);
      j["status"] = "Success";
      for (int i = 0; i < chat.size(); ++i) {
	if (chat[i].type == "user") continue;
	json tmp;
	tmp["type"] = chat[i].type;
	tmp["timestamp"] = chat[i].timestamp;
	tmp["from"] = chat[i].from;
	tmp["to"] = chat[i].to;
	tmp["content"] = chat[i].content;
	j["messages"].push_back(tmp);
      }
    }
    resp.set_content(j.dump());
    send_to_client(client_socket, resp.dump().c_str(), resp.dump().size());
  }
  else if (hr.path.substr(0, 5) == "/chat" && hr.method == "POST") {
    CHECK_LOGIN;
    std::string user = get_user(hr), frd = hr.path.substr(6);
    CHECK_USER;
    db::status res = db_manager.write_message(user, frd, hr.j_content["message"]);
    if (res == db::status::OK) {
      j["status"] = "Success";
    }
    else {
      j["status"] = "Failed";
    }
    resp.set_content(j.dump());
    send_to_client(client_socket, resp.dump().c_str(), resp.dump().size());
  }
  else if (hr.path.substr(0, 5) == "/file" && hr.method == "POST") {   // "/file/$filename/$friend_name
    CHECK_LOGIN;
    std::string user = get_user(hr);
    CHECK_USER;
    int pos = hr.path.substr(6).find("/") + 6;
    std::string frd = hr.path.substr(pos+1), filename = hr.path.substr(6, pos - 6), filetoken;
    if (db_manager.is_friend(user, frd) != db::status::OK) {
      j["status"] = "Not friend";
    }
    else {
      if (hr.download(msg, client_socket, filename, filetoken, db_manager) == 0) {
      if (db_manager.send_file_link(user, frd, filetoken) == db::status::OK)
        j["status"] = "Success";
      else
        j["status"] = "Failed";
      }
      else
  j["status"] = "Failed";
    }
    resp.set_content(j.dump());
    send_to_client(client_socket, resp.dump().c_str(), resp.dump().size());
  }
  else if (hr.path.substr(0, 6) == "/file/" && hr.method == "GET") {   // "/file/$fileid
    //    CHECK_LOGIN;
    //    std::string user = get_user(hr);
    std::string filetoken = hr.path.substr(6), filename;
    db_manager.filetoken2name(filetoken, filename);
    std::string filepath = "server_dir/" + filetoken;
    struct stat st;
    stat(filepath.c_str(), &st);
    int size = st.st_size, bytes = 0, tmp = 0;
    char buf[BUF_SIZE+1];
    std::string hdr = "HTTP/1.1 200 OK\r\n"
      "Cache-Control: no-cache, private\r\n"
      "Content-Type: text/html\r\n"
      "Content-Length: ";
    hdr += std::to_string(size) + "\r\n\r\n";
    int offset = hdr.size();
    memset(buf, 0, BUF_SIZE+1);
    strncpy(buf, hdr.c_str(), offset);
    FILE *fp = fopen(filepath.c_str(), "rb");
    while (true) {
      if (offset == BUF_SIZE || bytes == size) {
	offset = 0;
	if (send(client_socket, buf, BUF_SIZE, 0) < 0)
	  return -1;
      }
      tmp = fread(buf+offset, 1, std::min(size-bytes, BUF_SIZE-offset), fp);
      bytes += tmp;
      offset += tmp;
    }
    fclose(fp);
    return 0;
  }
  else if (hr.path.substr(0, 9) == "/filename" && hr.method == "GET") {   // "/filename/$fileid
    CHECK_LOGIN;
    std::string user = get_user(hr);
    CHECK_USER;
    std::string filetoken = hr.path.substr(10), filename;
    db::status res = db_manager.filetoken2name(filetoken, filename);
    if (res == db::status::OK) {
      j["status"] = "Success";
      j["filename"] = filename;
    }
    else {
      j["status"] = "Failed";
    }
    resp.set_content(j.dump());
    send_to_client(client_socket, resp.dump().c_str(), resp.dump().size());
  }
  else if (hr.path == "/console/upload") {
    CHECK_LOGIN;
    std::string user = get_user(hr);
    CHECK_USER;
    std::string filename = hr.j_content["filename"], frd = hr.j_content["friend_name"], filetoken;
    j["status"] = "Ready";
    resp.set_content(j.dump());
    send_to_client(client_socket, resp.dump().c_str(), resp.dump().size());
    int size = hr.j_content["size"], bytes = 0, tmp;
    db_manager.add_file(filename, filetoken);
    std::string filepath = "server_dir/" + filetoken;
    FILE *fp = fopen(filepath.c_str(), "wb");
    char buf[BUF_SIZE+1];
    while (bytes < size) {
      memset(buf, 0, BUF_SIZE+1);
      recv(client_socket, buf, BUF_SIZE, MSG_WAITALL);
      std::cerr << buf[0] << '\n';
      tmp = fwrite(buf, 1, std::min(size-bytes, BUF_SIZE), fp);
      bytes += tmp;
    }
    db_manager.send_file_link(user, frd, filetoken);
    fclose(fp);
  }
  else if (hr.path == "/console/download") {
    CHECK_LOGIN;
    std::string user = get_user(hr);
    CHECK_USER;
    std::string filetoken = hr.j_content["filetoken"], filename;
    db::status res = db_manager.filetoken2name(filetoken, filename);
    std::string filepath = "server_dir/" + filetoken;
    FILE* fp = fopen(filepath.c_str(), "rb");
    struct stat st;
    stat(filepath.c_str(), &st);
    int size = st.st_size, bytes = 0, tmp = 0;
    if (res == db::status::OK) {
      j["status"] = "Success";
      j["size"] = size;
      j["filename"] = filename;
    }
    else {
      j["status"] = "Failed";
    }
    resp.set_content(j.dump());
    send_to_client(client_socket, resp.dump().c_str(), resp.dump().size());
    
    char buf[BUF_SIZE+1];
    while (bytes < size) {
      memset(buf, 0, BUF_SIZE+1);
      bytes += fread(buf, 1, std::min(size - bytes, BUF_SIZE), fp);
      if (send(client_socket, buf, BUF_SIZE, MSG_NOSIGNAL) < 0)
        return -1;
    }
  }
  else if (hr.path == "/api/users" && hr.method == "GET") {
    std::vector<std::string> userlist;
    db::status res = db_manager.get_all_user(userlist);
    if (res == db::status::OK) {
      j["status"] = "Success";
      j["userlist"] = userlist;
    }
    else
      j["status"] = "Failed";
    resp.set_content(j.dump());
    send_to_client(client_socket, resp.dump().c_str(), resp.dump().size());
  }
  return 0;
}

// Handler for client connections
int MultiClientChat::on_client_connected(int client_socket) {
    return 0;
}

// Handler for client disconnections
int MultiClientChat::on_client_disconnected(int client_socket) {
    return 0;
}
