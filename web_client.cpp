#include "WebServer.hpp"
#include <cassert>

int main(int argc, char *argv[]) {

    assert(argc > 1);
    int port = atoi(argv[1]);

    // Run web server on local (Can be connected with browser)
    WebServer webserver("0.0.0.0", port);
    if(webserver.init() != 0)
        return -1;
    webserver.run();

    return 0;
}