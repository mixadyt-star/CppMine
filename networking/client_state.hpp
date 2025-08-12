#pragma once
#include <vector>

namespace networking {
    struct ClientState {
        std::vector<unsigned char> buffer {};
        size_t packet_length = 0;
        size_t temp_varint = 0;
        int varint_bytes_read = 0;
    };
}