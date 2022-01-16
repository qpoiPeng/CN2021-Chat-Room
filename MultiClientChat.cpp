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

  std::cerr << hr.path.substr(0, 10) << std::endl;

  HttpResponse resp;
  json j;

  // Handle CORS
  resp.set_header("Access-Control-Allow-Origin", hr.header["Origin"]);
  resp.set_header("Access-Control-Allow-Methods", "*");
  resp.set_header("Access-Control-Allow-Headers", "content-type");
  resp.set_header("Access-Control-Allow-Credentials", "true");

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
    std::cerr << "HERE\n";
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
    std::cerr << resp.dump();
    send_to_client(client_socket, resp.dump().c_str(), resp.dump().size());
  }
  // * Get friend list of user
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
  // * Send new message
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
  else if (hr.path.substr(0, 9) == "/sendFile" && hr.method == "POST") {   // "/file/$filename/$friend_name

    CHECK_LOGIN;
    std::string user = get_user(hr);
    CHECK_USER;
    std::vector<std::string> _tmp = db::split_string(hr.path, "/");
    std::string frd = _tmp[2], filetoken;

    std::cerr << "Sending file to " << frd << std::endl;

    if (db_manager.is_friend(user, frd) != db::status::OK) {
      j["status"] = "Not friend";
    }
    else {
      if (hr.download(filetoken, db_manager)) {
        if (db_manager.send_file_link(user, frd, filetoken) == db::status::OK)
          j["status"] = "Success";
        else
          j["status"] = "Db_manager send file link failed";
      }
      else
        j["status"] = "Download failed";
    }
    resp.set_content(j.dump());
    send_to_client(client_socket, resp.dump().c_str(), resp.dump().size());
  }
  else if (hr.path.substr(0, 6) == "/file/" && hr.method == "GET") {   // "/file/$fileid
    CHECK_LOGIN;
    std::string user = get_user(hr);
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
  // * Get all existing users
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
  else {
    // Parse out the document requested (gives all strings separated by spaces)
    std::istringstream iss(msg);
    std::vector<std::string> parsed((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());

    if(parsed[0] != "GET") {
        std::cerr << msg << std::endl;
        return 0;
    }

    std::string content = "<h1>404 Not Found</h1>";
    std::string path = "/index.html";
    std::string content_type = "text/html";
    int statusCode = 404;

    std::cerr << parsed[1] << std::endl;

    if (parsed.size() >= 3 && parsed[0] == "GET") {
        path = parsed[1];

        if(path == "/")
            path = "/index.html";
        else {
            std::string ext = path.substr(path.find_last_of(".") + 1);
            if(ext == "css")
                content_type = "text/css";
            else if(ext == "js")
                content_type = "text/javascript";
            else if(ext == "jpeg" || ext == "jpg" || ext == "JPG" || ext == "JPEG")
                content_type = "image/jpeg";
            else if(ext == "png" || ext == "PNG")
                content_type = "image/png";
            else if(ext == "html")
                content_type = "text/html";
            else
                content_type = "text/plain";
        }
    }

    // Catch all mechanism
    std::vector<std::string> paths = db::split_string(path, "/");

    // Open the document in the local file system
    std::ifstream f("server_dir/" + paths.back());

    if (f.good()) {
        std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        content = str;
        statusCode = 200;
    }
    f.close();

    // Write the document back to the client
    std::ostringstream oss;
    oss << "HTTP/1.1 " << statusCode << " OK\n";
    oss << "Cache-Control: no-cache, private\n";
    oss << "Content-Type: " << content_type << "\n";
    oss << "Content-Length: " << content.size() << "\n";
    oss << "\n";
    oss << content;

    std::string output = oss.str();
    int size = output.size() + 1;

    send_to_client(client_socket, output.c_str(), size);
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
