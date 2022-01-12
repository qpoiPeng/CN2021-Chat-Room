#include "MultiClientChat.hpp"
#include <string>
#include <istream>
#include <sstream>
#include <fstream>
#include <vector>
#include <streambuf>
#include <iterator>
#include "HttpParser/HttpParser.hpp"

// Handler for when a message is received from the client
int MultiClientChat::on_message_received(int client_socket, const char *msg, int length) {

    // Parse out the request (gives all strings separated by spaces)
    std::istringstream iss(msg);
    std::vector<std::string> parsed((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());

    if (parsed[0] == "1") { // login
        // int sign_in() {
        // if (send(sig::sign_in) < 0)
        //     return -1;
        // char buf[BUF_SIZE+1] = {};
        // std::string username, password;

        // if (recv(buf) <= 0)
        //     return -1;
        // username = buf;

        // memset(buf, 0, BUF_SIZE+1);
        // if (recv(buf) <= 0)
        //     return -1;
        // password = buf;

        // db::status res = server.db_manager.sign_in(username, password);
        // if (res == db::status::OK) {
        //     if (send(sig::sign_in_success) < 0)
        //     return -1;
        //     this->name = username;
        //     return 0;
        // }
        // // else
        // if (send(sig::sign_in_fail) < 0)
        //     return -1;
        // return 1;
        // }
    }
    else if (parsed[0] == "2") { // signup
        // std::string username, password;

        // char buf[BUF_SIZE+1] = {};
        // if (send(sig::sign_up) < 0)
        //     return -1;

        // if (recv(buf) <= 0)
        //     return -1;
        // username = buf;

        // memset(buf, 0, BUF_SIZE+1);
        // if (recv(buf) <= 0)
        //     return -1;
        // password = buf;

        // memset(buf, 0, BUF_SIZE+1);
        // if (recv(buf) <= 0)
        //     return -1;
        // password2 = buf;

        // if (password != password2) {
        // constexpr char password_prompt2[] = "password error.\n";
        // if (send(password_prompt2) < 0)
        //     return -1;
        // }
        // else {
        //     server.db_mtx.lock();
        //     db::status res = server.db_manager.sign_up(username, password);
        //     server.db_mtx.unlock();
        // if (res == db::status::OK) {

        //     constexpr char success[] = "Create account successfully!\n";

        //     if (send(success) < 0)
        //         return -1;
        // }
        // else {
        //     constexpr char failed[] = "The username is used. Please try again.\n";
        //     if (send(failed) < 0)
        //         return -1;
        // }

    }
    else if (parsed[0] == "3") { // quit

    }
    else {

    }

    // while (true) {

    //     if (s == "1") { // login
    //         int res = client->sign_in();

    //         if (res < 0)  // disconnect
    //             break;
    //         else if (res == 0) {  // success
    //             client->home();
    //             break;
    //         }
    //         // fail and try again.
    //     }
    //     else if (s == "2") {  // register
    //     if (client->sign_up() < 0)
    //     break;
    //     }
    //     else if (s == "3") {  // quit
    //     if (client->send(sig::quit) < 0)
    //         goto THREAD_END;
    //     break;
    //     }
    //     else {  // wrong input
    //     if (client->send(wrong_format) < 0)
    //         goto THREAD_END;
    //     }
    // }
    //   void home() {
//     char buf[BUF_SIZE+1] = {};
//     std::string s;

//     while (true) {
//       if (recv(buf) <= 0)
// 	      break;
//       s = buf;
//       if (s == "1") {  // list friend
//         std::cerr << "List friend" << std::endl;
//         send(sig::ok);
//       }
//       else if (s == "2") {  // send request
//         std::cerr << "Send request" << std::endl;
//         send(sig::ok);
//         // if (friend_request() < 0)
//           // break;
//       }
//       else if (s == "3") {  // confirm request
//         std::cerr << "Confirm request" << std::endl;
//         send(sig::ok);
//       }
//       else if (s == "4") {  // delete friend
//         std::cerr << "Delete friend" << std::endl;
//         send(sig::ok);
//       }
//       else if (s == "5") {  // direct message
//         std::cerr << "Direct message" << std::endl;
//         send(sig::ok);
//       }
//       else if (s == "6") {  // quit
//         std::cerr << "Quit" << std::endl;
//         send(sig::quit);
//         break;
//       }
//       else {
// 	      if (send(wrong_format) < 0)
// 	      break;
//       }
//     }
//   }

//   int friend_request() {
//     if (send(sig::create_friend_request) < 0)
//       return -1;
//     char buf[BUF_SIZE+1] = {};
//     std::string destination;
//     if (recv(buf) <= 0)
//       return -1;
//     destination = buf;
//     if (destination == name) {
//       if (send(sig::request_self) < 0)
// 	return -1;
//     }
//     else if (server.db_manager.if_user_exists(destination) == db::status::USER_NOT_EXISTS) {
//       if (send(sig::user_not_exists) < 0)
// 	return -1;
//     }
//     else {
//       if (server.db_manager.create_friend_request(name, destination) == db::status::DUPLICATED_REQUEST) {
// 	if (send(sig::duplicated_request) < 0)
// 	  return -1;
//       }
//       if (send(sig::ok) < 0)
// 	return -1;
//     }
//     return 0;
//   }

//   int confirm_friend_request() {
//     if (send(sig::confirm_friend_request) < 0)
//       return -1;
//     std::vector<std::string> request_list;
//     server.db_manager.get_friend_request_list(name, request_list);    // no check yet!
//     sort(request_list.begin(), request_list.end());
//     int size = request_list.size();
//     std::string s;
//     char buf[BUF_SIZE+1] = {};
//     sprintf(buf, "%d", size);
//     send(buf);
//     for (int i = 0; i < size; ++i) {
//       sprintf(buf, "%s", request_list[i].c_str());
//       send(buf);
//     }
//     int idx = 0;
//     if (size > 0) {
//       memset(buf, 0, BUF_SIZE+1);
//       recv(buf);
//       try {
// 	s = buf;
// 	idx = stoi(s);
//       }
//       catch (std::exception e) {
// 	      send(sig::wrong_format);
// 	      return 0;
//       }
//     }
//     if (idx > size || idx < -size) {
//       send(sig::wrong_format);
//       return 0;
//     }

//     return 0;
//   }

// };
    HttpRequest hr(msg);
    hr.show();
    
    // std::cerr << msg;

    return 0;
}

// Handler for client connections
int MultiClientChat::on_client_connected(int client_socket) {

    constexpr char welcome[] = "(1) Login (2) Register (3) Quit\n";
    send_to_client(client_socket, welcome, strlen(welcome) + 1);

    return 0;
}

// Handler for client disconnections
int MultiClientChat::on_client_disconnected(int client_socket) {
    return 0;
}
