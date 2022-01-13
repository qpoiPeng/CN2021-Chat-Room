#include "MultiClientChat.hpp"
#include <string>
#include <istream>
#include <sstream>
#include <fstream>
#include <vector>
#include <streambuf>
#include <iterator>
#include "HttpParser/HttpParser.hpp"
#include "HttpParser/HttpResponse.hpp"

// Handler for when a message is received from the client
int MultiClientChat::on_message_received(int client_socket, const char *msg, int length) {

  HttpRequest hr(msg, client_socket);
  hr.show();
  if (hr.path == "/register") {
    db::status res = db_manager.sign_up(hr.j_content["name"], hr.j_content["password"]);
    if (res == db::status::OK) {
      HttpResponse resp("Success");
      std::cerr << resp.dump().c_str() << resp.dump().size();
      send_to_client(client_socket, resp.dump().c_str(), resp.dump().size()+1);
    }
    else {
      HttpResponse resp("Failed");
      send_to_client(client_socket, resp.dump().c_str(), resp.dump().size());
    }
  }
  else if (hr.path == "/login") {
    db::status res = db_manager.sign_in(hr.j_content["name"], hr.j_content["password"]);
    if (res == db::status::OK) {
      HttpResponse resp("Success");
      resp.set_header("Set-Cookie", "123");  // TODO
      send_to_client(client_socket, resp.dump().c_str(), resp.dump().size());
    }
    else {
      HttpResponse resp("Failed");
      send_to_client(client_socket, resp.dump().c_str(), resp.dump().size());
    }
  }
  else if (hr.path == "/friends" && hr.method == "GET") {
    std::string user = "qpoi";     // TODO: get user by cookie
    std::vector<std::string> friends;

    db::status res = db_manager.get_friend_list(user, friends);
    if (res == db::status::OK) {
      std::string content;
      for (std::string& f : friends)
	content += f + ",";
      HttpResponse resp;
      resp.set_content(content);
      std::string output = resp.dump();
      int size = output.size() + 1;
      std::cerr << output.c_str();
      send_to_client(client_socket, output.c_str(), size);
    }
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
