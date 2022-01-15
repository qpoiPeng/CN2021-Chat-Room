#include "MultiClientChat.hpp"
#include <string>
#include <istream>
#include <sstream>
#include <fstream>
#include <vector>
#include <streambuf>
#include <iterator>
#include <climits>
#include "HttpParser/HttpResponse.hpp"

using json = nlohmann::json;

#define CHECK_LOGIN if (hr.header.find("Cookie") == hr.header.end()) {	\
    j["status"] = "NOT LOGIN";						\
    resp.set_content(j.dump());						\
    send_to_client(client_socket, resp.dump().c_str(), resp.dump().size()); \
    return 0;								\
  }
#define CHECK_USER if (user == "") {					\
    j["status"] = "NOT LOGIN";						\
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
    std::string token = hr.header["Cookie"].substr(pos+3, endpos-pos);
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
  if (hr.path == "/register") {
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
  else if (hr.path == "/friends" && hr.method == "GET") {
    CHECK_LOGIN;
    std::string user = get_user(hr);
    CHECK_USER;
    if (user == "") {
      resp.set_content(j.dump());
      send_to_client(client_socket, resp.dump().c_str(), resp.dump().size());
      return 0;
    }
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
  else if (hr.path.substr(0, 5) == "/chat" && hr.method == "GET") {
    CHECK_LOGIN;
    std::string user = get_user(hr), frd = hr.path.substr(6);
    CHECK_USER;
    std::vector<db::Message> chat;
    db::status res = db_manager.get_chat(user, frd, chat);
    if (res == db::status::OK) {
      sort(chat.begin(), chat.end(), timecmp);
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
  else if (hr.path.substr(0, 5) == "/file" && hr.method == "POST") {
    if (hr.download(msg, client_socket) == 0)
      j["status"] = "Success";
    else
      j["status"] = "Failed";
    resp.set_content(j.dump());
    send_to_client(client_socket, resp.dump().c_str(), resp.dump().size());
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
