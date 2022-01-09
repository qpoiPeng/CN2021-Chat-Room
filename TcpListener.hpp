#pragma once
#include "TcpClient.hpp"
#include "mysig.hpp"
#include "const.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <set>
#include <algorithm>
#include <iostream>

class TcpListener {
    public:

        TcpListener(const char* ipAddress, int port) :
            m_ipAddress(ipAddress), m_port(port) { }

        // Initialize the listener
        int init();

        // Run the listener
        int run();


    protected:
        // Lock & Unlock mutex before switching threads
        void lock() { m_mtx.lock(); }
        void unlock() { m_mtx.unlock(); }

        // Thread function that runs to serve a specific client
        void _serve(int client_fd, int client_id);

        // Cleans up thread after client disconnects
        void cleanup(int id);

        // Send a message to a client
        void send_to_client(int client_socket, const char* msg, int length);

        // Broadcast a message from a client
        void broadcast_to_clients(int sending_client, const char* msg, int length);

        // Receive a message from a client
        char* recv_from_client(int client_socket);

        // Handler for when a message is received from the client
        virtual int on_message_received(int client_socket, const char *msg, int length);

        // Handler for client connections
        virtual int on_client_connected(int client_socket);

        // Handler for client disconnections
        virtual int on_client_disconnected(int client_socket);

    private:
        const char* m_ipAddress;    // IP Address server will run on
        int m_port;                 // Port # for the server
        int m_socket;               // Internal socket FD for the listening

        TcpClient* m_clients;       // Records the info of all clients referenced by thread id
        int m_id2fd[MAX_THREAD];    // Get client fd by client id
        int m_fd2id[MAX_THREAD];    // Get client id by client fd
        std::thread m_thr[MAX_THREAD] = {}; // Records the status of all threads
        std::mutex m_mtx;           // Mutex to acquite before running threads
        std::set<int> m_freeThread; // Free threads that have not been used
        std::set<int> m_usedThread; // Complement of free_thread
        bool running;               // Records the state of the listener
};