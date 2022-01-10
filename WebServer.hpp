#pragma once
#include "TcpListener.hpp"

class WebServer : public TcpListener {
    public:
        WebServer(const char* ip_address, int port) :
            TcpListener(ip_address, port) { }

    protected:
        // Handler for when a message is received from the client
        virtual int on_message_received(int client_socket, const char *msg, int length);

        // Handler for client connections
        virtual int on_client_connected(int client_socket);

        // Handler for client disconnections
        virtual int on_client_disconnected(int client_socket);

    private:

};