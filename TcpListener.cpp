#include "TcpListener.hpp"

int TcpListener::init() {

    if ((m_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "create socket failed...\n");
        return -1;
    }

    // Initialize listener to the specified ip address and port
    sockaddr_in hint;
    bzero(&hint, sizeof(hint));
    hint.sin_family = AF_INET;
    hint.sin_addr.s_addr = htonl(INADDR_ANY);
    hint.sin_port = htons(m_port);
    // hint_len = sizeof(hint);

    m_clients = new TcpClient[MAX_THREAD];

    // Initialize threads
    for (int i = 0; i < MAX_THREAD; ++i) {
        m_freeThread.insert(i);
        m_clients[i] = TcpClient(0, i);
        m_fd2id[i] = m_id2fd[i] = -1;
    }

    // Bind socket to port
    if (bind(m_socket, (sockaddr*)&(hint), sizeof(hint)) < 0) {
        fprintf(stderr, "bind failed...\n");
        return -1;
    }

    // Tell that the system that the socket is for listening
    if (listen(m_socket, 1024) < 0) {
        fprintf(stderr, "listen failed...\n");
        return -1;
    }

    DIR* dir = opendir("server_dir");
    if (dir) {
      closedir(dir);
    }
    else if (errno == ENOENT) {
      mkdir("server_dir", 0744);
    }
    
    return 0;
}

void TcpListener::_serve(int client_fd, int client_id) {
    std::cerr << "new client served, client_fd : " << client_fd << ", client_id : " << client_id << std::endl;

    while(true) {
        char buf[BUF_SIZE+1];
        memset(buf, 0, BUF_SIZE+1);
        int bytes_in = recv(client_fd, buf, BUF_SIZE, 0);
        if (bytes_in <= 0) {
            std::cerr << client_fd << " disconnected" << std::endl;
            // Drop the client
            on_client_disconnected(client_fd);
            break;
        }
        else {
            if (on_message_received(client_fd, buf, bytes_in) < 0) {
                std::cerr << client_fd << " disconnected" << std::endl;
                // Drop the client
                on_client_disconnected(client_fd);
                break;
            }
        }
    }

    cleanup(client_id);
    close(client_fd);
}

// Cleans up all the information about this specific client for the multi client chat server
void TcpListener::cleanup(int id) {
    int fd = m_id2fd[id];
    m_id2fd[id] = m_fd2id[fd] = -1;
    lock();
    m_usedThread.erase(id);
    m_freeThread.insert(id);
    unlock();
}

int TcpListener::run() {
    int client_fd;
    running = true;
    while (running) {
        // Accept a new connection
        client_fd = accept(m_socket, nullptr, nullptr);

        // Acquire thread mutex
        lock();

        // Run new client on new thread and detach the thread
        int id = *(m_freeThread.begin());
        m_freeThread.erase(id);
        m_usedThread.insert(id);
        m_clients[id].set_fd(client_fd);
        // Initialize these conversion tables for further use
        m_id2fd[id] = client_fd;
        m_fd2id[client_fd] = id;

        m_thr[id] = std::thread(&TcpListener::_serve, this, client_fd, id);
        m_thr[id].detach();

        // Release thread mutex
        unlock();

        on_client_connected(client_fd);
        std::cerr << "new connection: " << client_fd << std::endl;
    }
    return 0;
}

// Send a message to a client
void TcpListener::send_to_client(int client_socket, const char* msg, int length) {
    send(client_socket, msg, length, 0);
}

// Broadcast a message from a client
void TcpListener::broadcast_to_clients(int sending_client_socket, const char* msg, int length) {
    lock();
    for (auto id : m_usedThread) {
        int out_socket = m_clients[id].get_fd();
        if (out_socket != sending_client_socket)
            send_to_client(out_socket, msg, length);
    }
    unlock();
}

// Handler for when a message is received from the client
int TcpListener::on_message_received(int client_socket, const char *msg, int length) {
    return 0;
}

// Handler for client connections
int TcpListener::on_client_connected(int client_socket) {
    return 0;
}

// Handler for client disconnections
int TcpListener::on_client_disconnected(int client_socket) {
    return 0;
}
