#pragma once // ensure this header is only included once
#include "packet_defs.hpp"
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <mutex>
#include <string>
#include <unordered_map>

namespace naina {

    class BlackBoxLogger {
    public:
        explicit BlackBoxLogger(const std::string& session_dir); // create a logger to write into session directory
        ~BlackBoxLogger(); // flush and close files

        BlackBoxLogger(const BlackBoxLogger&) = delete; // create a rule that says this object is non-copyable
        BlackBoxLogger& operator=(const BlackBoxLogger&) = delete; // another rule saying this object is non-reassignable

        void write_text_line(const std::string& line); // append a human-readable line

        void write_packet(PacketType type, // append a packet
                        const void* payload,
                        uint32_t payload_bytes,
                        uint64_t t_ns);

        const std::string& session_dir() const { return session_dir_; } // access session directory path

    private:
        uint64_t next_seq(PacketType type);

        std::string session_dir_;
        std::ofstream bin_;
        std::ofstream txt_;
        std::mutex mtx_;
        std::unordered_map<uint16_t, uint64_t> seq_;
    };

} 
