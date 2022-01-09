class TcpClient {

    public:
        TcpClient() { }
        TcpClient(int fd, int id) {
            m_fd = fd;
            m_id = id;
        }

        int set_fd(int fd) { m_fd = fd; return 0; }
        int get_fd() { return m_fd; }

        // enum st {STATE_NEW, STATE_LOGIN, STATE_REGISTER, STATE_NORMAL};

    protected:

    private:

        int m_fd; // socket fd for client
        int m_id; // id for client
        int state; // current state of the client
};