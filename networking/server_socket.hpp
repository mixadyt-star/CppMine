#pragma once
#include "client_state.hpp"
#include <unordered_map>
#include <functional>
#include <thread>
#include <string>
#include <vector>
#include <mutex>
#include <queue>

#ifdef _WIN32
    #include <winsock2.h>

    #pragma comment(lib, "Ws2_32.lib")
    typedef SOCKET socket_t;
#else
    typedef int socket_t;
#endif

namespace networking {
    class ServerSocket {
        std::unordered_map<socket_t, std::queue<std::vector<unsigned char>>> send_queues {};
        std::mutex send_queues_mutex;
        std::function<void (socket_t, unsigned char*, size_t)> data_callback;
        std::unordered_map<socket_t, networking::ClientState> clients_states {};
        std::mutex clients_states_mutex;
        std::vector<pollfd> fds {};
        std::mutex fds_mutex;
        std::thread server_thread;
        int _event_timeout;
        int _buffer_size;
        socket_t _socket;
        bool running;

        public:
            ServerSocket();
            void bind(std::string ip, unsigned short port);
            void on_data(std::function<void (socket_t, unsigned char*, unsigned long)> callback);
            void listen(int backlog);
            void send(socket_t client_socket, unsigned char* packet, size_t size);
            void start(int event_timeout = 100, int buffer_size = 1024);
            void close();
            void stop();
            
        private:
            void init_networking();
            void set_non_blocking(socket_t sock);
            void run();
            void add_pfd(int fd, short events);
            void process_events();
            void accept();
            void process_client_data(socket_t client_socket);
            void parse_packets(socket_t client_socket);
            void process_client_send(socket_t client_socket);
            void cleanup();
    };
}