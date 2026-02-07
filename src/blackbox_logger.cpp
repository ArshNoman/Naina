#include "blackbox_logger.hpp"
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem; // fs becomes shorthand for std::filesystem

namespace naina {
// this function runs when a BlackBoxLogger object is created (constructor)
BlackBoxLogger::BlackBoxLogger(const std::string &session_dir)
    : // session_dir is a string containing a folder path
      session_dir_(session_dir) { // session_dir_ (a class variable) is set to
                                  // session_dir
  fs::create_directories(
      session_dir_); // create the directory if it does not already exist

  // std::ios::binary : write raw bytes
  // std::ios::out    : open for output
  bin_.open(session_dir_ + "/BlackBox.bin", std::ios::binary | std::ios::out);
  // Open a text file for writing
  txt_.open(session_dir_ + "/BlackBox.txt", std::ios::out);

  // check whether either file failed to open
  if (!bin_.is_open() || !txt_.is_open()) {
    // throw an exception if file opening failed and stop object creation
    throw std::runtime_error("file opening failed");
  }
}

// this function runs automatically when the object is destroyed (destructor)
BlackBoxLogger::~BlackBoxLogger() {
  std::lock_guard<std::mutex> lock(
      mtx_); // lock mutex so no other thread can use the files
  if (bin_.is_open())
    bin_.close(); // close the binary file
  if (txt_.is_open())
    txt_.close(); // close the text file
}

// write one line to the log file
void BlackBoxLogger::write_text_line(const std::string &line) {
  std::lock_guard<std::mutex> lock(
      mtx_);            // lock mutex so no other thread can use the files
  txt_ << line << "\n"; // add line
  txt_.flush();         // force data to be written on disk immediately
}

// returns the next sequence number for a given packet type
uint64_t BlackBoxLogger::next_seq(PacketType type) {
  const uint16_t key = static_cast<uint16_t>(
      type);                // convert the enum PacketType into a 16-bit integer
  auto it = seq_.find(key); // find the key in the sequence map
  if (it == seq_.end()) {   // if this packet type has never been seen before
    seq_[key] = 1;          // sequence order = 1
    return 1;
  }
  it->second += 1;   // else, increment the current sequence number by 1
  return it->second; // return updated value
}

// write a structured binary packet to the binary file
void BlackBoxLogger::write_packet(PacketType type, const void *payload,
                                  uint32_t payload_bytes, uint64_t t_ns) {
  std::lock_guard<std::mutex> lock(
      mtx_); // lock mutex so no other thread can use the file

  PacketHeader
      hdr{}; // create a PakcetHeader object and zero-initialize all fields
  hdr.magic = kMagic;
  hdr.version = kVersion;
  hdr.type = type;
  hdr.payload_bytes = payload_bytes;
  hdr.t_ns = t_ns;
  hdr.seq = next_seq(type);

  bin_.write(
      reinterpret_cast<const char *>(&hdr),
      sizeof(PacketHeader)); // write the header as raw bytes to the binary file
  if (payload_bytes > 0 && payload != nullptr) { // if there is payload data
    bin_.write(reinterpret_cast<const char *>(payload),
               payload_bytes); // write that as well
  }
}

} // namespace naina
