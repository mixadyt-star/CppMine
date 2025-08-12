#pragma once
#include <stdexcept>

namespace exceptions {
    class NetworkSetupError : public std::runtime_error {
        public:
            NetworkSetupError(int err_code);
    };

    class SocketOpenError : public std::runtime_error {
        public:
            SocketOpenError(int err_code);
    };

    class SocketBindError : public std::runtime_error {
        public:
            SocketBindError(int err_code);
    };

    class SetSocketNonBlockingError : public std::runtime_error {
        public:
            SetSocketNonBlockingError(int err_code);
    };

    class SocketListenError : public std::runtime_error {
        public:
            SocketListenError(int err_code);
    };

    class SocketPollError : public std::runtime_error {
        public:
            SocketPollError(int err_code);
    };

    class SocketAcceptError : public std::runtime_error {
        public:
            SocketAcceptError(int err_code);
    };

    class SocketReceiveError : public std::runtime_error {
        public:
            SocketReceiveError(int err_code);
    };

    class SocketSendError : public std::runtime_error {
        public:
            SocketSendError(int err_code);
    };
}