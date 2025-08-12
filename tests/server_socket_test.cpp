#include "../networking/server_socket.hpp"
#include "../networking/exceptions.hpp"
#include <iostream>
#include <thread>

using namespace networking;
using namespace exceptions;
using namespace std;

int main() {
    cout << "Binding ServerSocket to ip 127.0.0.1 and port 45678, attaching callback and checking for errors.\n";

    ServerSocket server;
    try {
        server.bind("127.0.0.1", 45678);
        server.on_data([&server] (socket_t client_socket, unsigned char* packet, unsigned long size) {
            cout << string((char*) packet, (size_t) size) << '\n';
            server.send(client_socket, packet, size);
        });
        server.listen(1);
    } catch (const exception& e) {
        cout << "Got unexpected error:\n" << e.what() << '\n';
        server.close();
        return 1;
    }

    server.start();
    cout << "Press enter to stop server...\n";
    cin.get();

    cout << "Stopping server";
    server.stop();
}