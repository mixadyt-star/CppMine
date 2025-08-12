#include "exceptions.hpp"
#include <stdexcept>
#include <string>

using namespace exceptions;
using namespace std;

NetworkSetupError::NetworkSetupError(int err_code): runtime_error("Can't setup network because of error with code: " + to_string(err_code)) {}
SocketOpenError::SocketOpenError(int err_code): runtime_error("Can't open socket because of error with code: " + to_string(err_code)) {}
SocketBindError::SocketBindError(int err_code): runtime_error("Can't bind socket because of error with code: " + to_string(err_code)) {}
SetSocketNonBlockingError::SetSocketNonBlockingError(int err_code): runtime_error("Can't set socket to nonblocking mode because of error with code: " + to_string(err_code)) {}
SocketListenError::SocketListenError(int err_code): runtime_error("Can't lsiten socket because of error with code: " + to_string(err_code)) {}
SocketPollError::SocketPollError(int err_code): runtime_error("Can't poll socket event because of error with code: " + to_string(err_code)) {}
SocketAcceptError::SocketAcceptError(int err_code): runtime_error("Can't accept client socket because of error with code: " + to_string(err_code)) {}
SocketReceiveError::SocketReceiveError(int err_code): runtime_error("Can't receive data because of error with code: " + to_string(err_code)) {}
SocketSendError::SocketSendError(int err_code): runtime_error("Can't send data because of error with code: " + to_string(err_code)) {}