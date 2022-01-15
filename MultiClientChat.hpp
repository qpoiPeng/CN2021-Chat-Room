#pragma once

#include "Db_manager/Db_manager.hpp"
#include "TcpListener.hpp"
#include "nlohmann/json.hpp"
#include "HttpParser/HttpParser.hpp"
class MultiClientChat : public TcpListener {
    public:
        MultiClientChat(const char* ip_address, int port) :
            TcpListener(ip_address, port) { }

    protected:
        // Handler for when a message is received from the client
        virtual int on_message_received(int client_socket, const char *msg, int length);

        // Handler for client connections
        virtual int on_client_connected(int client_socket);

        // Handler for client disconnections
        virtual int on_client_disconnected(int client_socket);

    private:
        std::mutex db_mutex;
        db::Db_manager db_manager;
        std::string get_user(HttpRequest& hr);
};
