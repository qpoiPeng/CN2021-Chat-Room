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


    // Parse out the document requested (gives all strings separated by spaces)
    std::istringstream iss(msg);
    std::vector<std::string> parsed((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());

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
        }
    }

    // Catch all mechanism
    std::vector<std::string> reserved{"manifest.json", "favicon.ico", "asset-manifest.json", "static"};
    std::vector<std::string> paths = split_string(path, "/");

    bool flag = true;
    for(auto resv : reserved)
        if(paths[1] == resv)
            flag = false;

    if(flag) {
        for(auto path : paths)
            std::cerr << path << " ";
        std::cerr << std::endl;
        path = "/index.html";
    }

    // Open the document in the local file system
    std::ifstream f("./Frontend/build" + path);

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

    return 0;
}

// Handler for client connections
int WebServer::on_client_connected(int client_socket) {

    return 0;
}

// Handler for client disconnections
int WebServer::on_client_disconnected(int client_socket) {

    return 0;
}


std::vector<std::string> WebServer::split_string(std::string s, std::string delim) {
    std::vector<std::string> ret;
    int cur = 0, pos = 0;
    while ((pos = s.find(delim, cur)) != -1) {
        ret.push_back(s.substr(cur, pos - cur));
        cur = pos + delim.size();
    }
    if (s.substr(cur) != "")
        ret.push_back(s.substr(cur));
    return ret;
}
