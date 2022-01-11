#include <string>
#include <istream>
#include <sstream>
#include <fstream>
#include <vector>
#include <streambuf>
#include <iterator>
#include "WebServer.hpp"

// Handler for when a message is received from the client
int WebServer::on_message_received(int client_socket, const char *msg, int length) {

    std::cerr << msg << std::endl;
    // Write the document back to the client
    std::ostringstream oss;

    oss << "HTTP/1.1 200 OK\n";
    oss << "Cache-Control: no-cache, private\n";
    oss << "Content-Type: text/plain\n";
    oss << "Content-Length: 7\n";
    oss << "Access-Control-Allow-Origin: *\n";
    oss << "Access-Control-Allow-Methods: *\n";
    oss << "Access-Control-Allow-Headers: *\n";
    oss << "\n";
    oss << "Success\n";

    std::string output = oss.str();
    int size = output.size() + 1;

    send_to_client(client_socket, output.c_str(), size);

    return 0;

    // Parse out the document requested (gives all strings separated by spaces)
    // std::istringstream iss(msg);
    // std::vector<std::string> parsed((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());

    // std::string content = "<h1>404 Not Found</h1>";
    // std::string html_file = "/index.html";
    // int statusCode = 404;

    // if (parsed.size() >= 3 && parsed[0] == "GET") {
    //     html_file = parsed[1];

    //     if(html_file == "/")
    //         html_file = "/index.html";
    // }

    // // Open the document in the local file system
    // std::ifstream f("./wwwroot" + html_file);

    // if (f.good()) {
    //     std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    //     content = str;
    //     statusCode = 200;
    // }

    // f.close();

    // // Write the document back to the client
    // std::ostringstream oss;
    // oss << "HTTP/1.1 " << statusCode << " OK\n";
    // oss << "Cache-Control: no-cache, private\n";
    // oss << "Content-Type: text/html\n";
    // oss << "Content-Length: " << content.size() << "\n";
    // oss << "\n";
    // oss << content;

    // std::string output = oss.str();
    // int size = output.size() + 1;

    // send_to_client(client_socket, output.c_str(), size);

    // return 0;
}

// Handler for client connections
int WebServer::on_client_connected(int client_socket) {

    return 0;
}

// Handler for client disconnections
int WebServer::on_client_disconnected(int client_socket) {

    return 0;
}