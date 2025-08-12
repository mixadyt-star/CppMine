#include "server_socket.hpp"
#include "client_state.hpp"
#include "exceptions.hpp"
#include <functional>
#include <future>
#include <thread>
#include <string>
#include <vector>
#include <mutex>

#ifdef _WIN32
    #include <winsock2.h>

    #pragma comment(lib, "Ws2_32.lib")
#else

#endif

using namespace networking;
using namespace exceptions;
using namespace std;

// Public
ServerSocket::ServerSocket(): running(false) {
#ifdef _WIN32
    init_networking();

    _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_socket == INVALID_SOCKET) {
        cleanup();
        throw SocketOpenError(WSAGetLastError());
    }

    set_non_blocking(_socket);
#else

#endif
}

void ServerSocket::bind(string ip, unsigned short port) {
#ifdef _WIN32
    sockaddr_in service{};
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr(ip.c_str());
    service.sin_port = htons(port);

    int result = ::bind(_socket, (sockaddr* ) &service, sizeof (service));
    if (result == SOCKET_ERROR) {
        throw SocketBindError(WSAGetLastError());
    }
#else

#endif
}

void ServerSocket::on_data(function<void (socket_t, unsigned char*, unsigned long)> callback) {
    data_callback = callback;
}

void ServerSocket::listen(int backlog = SOMAXCONN) {
#ifdef _WIN32
    int result = ::listen(_socket, backlog);
    if (result == SOCKET_ERROR) {
        throw SocketListenError(WSAGetLastError());
    }
#else

#endif
}

void ServerSocket::send(socket_t client_socket, unsigned char* packet, size_t size) {
    vector<unsigned char> header;
    size_t len = size;

    while (len != 0) {
        unsigned char byte = len & 0x7F;
        len >>= 7;
        if (len != 0) {
            byte |= 0x80;
        }
        header.push_back(byte);
    }
    
    vector<unsigned char> full_packet;
    full_packet.reserve(header.size() + size);
    full_packet.insert(full_packet.end(), header.begin(), header.end());
    full_packet.insert(full_packet.end(), packet, packet + size);
    
    lock_guard<mutex> lock(send_queues_mutex);
    queue<vector<unsigned char>> &queue = send_queues[client_socket];
    queue.push(move(full_packet));
}

void ServerSocket::start(int event_timeout /*= 100*/, int buffer_size /*= 1024*/) {
#ifdef _WIN32
    add_pfd(_socket, POLLIN);

    _event_timeout = event_timeout;
    _buffer_size = buffer_size;
    running = true;

    server_thread = thread(&ServerSocket::run, this);
#else

#endif
}

void ServerSocket::close() {
#ifdef _WIN32
    closesocket(_socket);
    cleanup();
#else

#endif
}

void ServerSocket::stop() {
    if (running) {
        running = false;
        if (server_thread.joinable()) server_thread.join();
        close();
    }
}

// Private
void ServerSocket::init_networking() {
#ifdef _WIN32
    WSADATA wsa_data;

    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        throw NetworkSetupError(result);
    }
#endif
}

void ServerSocket::set_non_blocking(socket_t sock) {
#ifdef _WIN32
    unsigned long nonblocking = 1;

    int result = ioctlsocket(sock, FIONBIO, &nonblocking);
    if (result != 0) {
        throw SetSocketNonBlockingError(result);
    }
#else

#endif
}

void ServerSocket::run() {
#ifdef _WIN32
    while (running) {
        {
            lock_guard<mutex> lock(send_queues_mutex);
            for (auto &pair : send_queues) {
                if (!pair.second.empty()) {
                    for (auto &pfd : fds) {
                        if (pfd.fd == pair.first) {
                            pfd.events |= POLLOUT;
                            break;
                        }
                    }
                }
            }
        }

        int count = WSAPoll(fds.data(), fds.size(), _event_timeout);

        if (count == SOCKET_ERROR) {
            throw SocketPollError(WSAGetLastError());
        }

        if (count == 0) continue;

        process_events();
    }
#else

#endif
}

void ServerSocket::add_pfd(int fd, short events) {
#ifdef _WIN32
    pollfd pfd {};
    pfd.fd = fd;
    pfd.events = POLLIN;

    lock_guard<mutex> lock(fds_mutex);
    fds.push_back(pfd);
#else

#endif
}

void ServerSocket::process_events() {
#ifdef _WIN32
    for (pollfd &pfd : fds) {
        if (pfd.revents == 0) continue;

        if (pfd.fd == _socket) {
            accept();
            pfd.revents = 0;
            continue;
        }

        if (pfd.revents & POLLIN) {
            process_client_data(pfd.fd);
            pfd.revents &= ~POLLIN;
        }

        if (pfd.revents & POLLOUT) {
            process_client_send(pfd.fd);
            pfd.revents &= ~POLLOUT;
        }
    }
#else

#endif
}

void ServerSocket::accept() {
#ifdef _WIN32
    while (true) {
        socket_t client_socket = ::accept(_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            int error = WSAGetLastError();

            if (error == WSAEWOULDBLOCK) break;
            throw SocketAcceptError(error);
        }

        set_non_blocking(client_socket);
        add_pfd(client_socket, POLLIN);
        {
            lock_guard<mutex> lock(clients_states_mutex);
            clients_states[client_socket] = ClientState();
        }
        {
            lock_guard<mutex> lock(send_queues_mutex);
            send_queues[client_socket] = queue<vector<unsigned char>> ();
        }
    }
#else

#endif
}

void ServerSocket::process_client_data(socket_t client_socket) {
#ifdef _WIN32
    char* buffer {new char[_buffer_size]};

    while (running) {
        int count = recv(client_socket, buffer, _buffer_size, 0);

        if (count > 0) {
            if (clients_states.find(client_socket) == clients_states.end()) {
                break;
            }

            {
                lock_guard<mutex> lock(clients_states_mutex);
                ClientState &client_state = clients_states[client_socket];
                client_state.buffer.insert(client_state.buffer.end(), buffer, buffer + count);
            }

            future handle = async(launch::async, parse_packets, this, client_socket);
        } else if (count < 0) {
            int error = WSAGetLastError();

            if (error == WSAEWOULDBLOCK) break;
            throw SocketReceiveError(error);
        }
    }
#else

#endif
}

void ServerSocket::parse_packets(socket_t client_socket) {
    lock_guard<mutex> lock(clients_states_mutex);
    ClientState &client_state = clients_states[client_socket];
    size_t offset = 0;

    while (offset < client_state.buffer.size()) {
        if (client_state.packet_length == 0) {
            while (offset < client_state.buffer.size() && client_state.varint_bytes_read <= 5) {
                size_t byte = client_state.buffer[offset++];
                client_state.temp_varint |= (byte & 0x7F) << (7 * client_state.varint_bytes_read);
                client_state.varint_bytes_read++;

                if ((byte & 0x80) == 0) {
                    client_state.packet_length = client_state.temp_varint;

                    client_state.temp_varint = 0;
                    client_state.varint_bytes_read = 0;
                    break;
                }
            }

            if (client_state.packet_length == 0) break;
        } else {
            size_t available = client_state.buffer.size() - offset;

            if (available >= client_state.packet_length) {
                if (data_callback) data_callback(client_socket, client_state.buffer.data() + offset, client_state.packet_length);

                offset += client_state.packet_length;
                client_state.packet_length = 0;
            } else break;
        }
    }

    if (offset > 0) client_state.buffer.erase(client_state.buffer.begin(), client_state.buffer.begin() + offset);
}

void ServerSocket::process_client_send(socket_t client_socket) {
    lock_guard<mutex> lock(send_queues_mutex);
    unordered_map<socket_t, queue<vector<unsigned char>>>::iterator it = send_queues.find(client_socket);
    if (it == send_queues.end()) return;
    
    queue<vector<unsigned char>> &queue = it->second;
    while (!queue.empty()) {
        vector<unsigned char> &packet = queue.front();
        size_t bytes_sent = 0;
        
        while (bytes_sent < packet.size()) {
            int result = ::send(client_socket, reinterpret_cast<const char*> (packet.data() + bytes_sent), packet.size() - bytes_sent, 0);
            
            if (result == SOCKET_ERROR) {
                int error = WSAGetLastError();
                if (error == WSAEWOULDBLOCK) {
                    if (bytes_sent > 0) packet.erase(packet.begin(), packet.begin() + bytes_sent);

                    return;
                } else {
                    throw SocketSendError(error);
                }
            }
            
            bytes_sent += result;
        }
        
        queue.pop();
    }
}

void ServerSocket::cleanup() {
#ifdef _WIN32
    WSACleanup();
#endif
}