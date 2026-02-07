#pragma once // ensure this header is only included once
#include <cstdint>

namespace naina {

// 1-9 : OAK related
// 10-19: Pixhawk related

enum class PacketType : uint8_t {
  OakLeftGrayU8 = 1,   // left gray'ed image buffer, 8-bit grayscale
  OakDisparityu16 = 2, // disparty map image buffer, 16-bit values
  OakQuality = 3,      // health metrics from DepthAI pipeline
  PixhawkIMU = 10,     // raw IMU sensor data (linear acceleration, angular
                       // velocity, timestamps)
  PixhawkStatus = 11 // system status information from Pixhawk (arm state, error
                     // codes, health flags)
};

#pragma pack(push, 1) // no padding bytes are inserted between fields

struct PacketHeader {
  uint32_t magic;         // 0x41525348 = "ARSH"
  uint16_t version;       // 1
  PacketType type;        // PacketType
  uint32_t payload_bytes; // how many bytes that follow the header
  uint64_t t_ns;          // timestamp of when the packet was created
  uint64_t seq;           // sequence per type
};

struct OAKpayload {
  float depth_valid_ratio; // ratio (0-1) of valid depth measurements to total
                           // depth pixels
  float
      mean_disparity; // average disparty value measured over the disparity map
  float exposure_time_us; // sensor exposure duration in microseconds
  float analog_gain; // how much the camera amplified the signal to make the
                     // image brighter
  uint32_t width;
  uint32_t height;
};

struct IMUpayload {
  float gyro_rad_s[3];  // 3 gyro values (x, y, z)
  float accel_m_s2[3];  // 3 acceleration values (x, y, z)
  uint64_t imu_time_us; // timestamp from the IMU itself
};

#pragma pack(                                                                  \
    pop) // restore normal memory alignment rules (reverts line 17's command)

static constexpr uint32_t kMagic = 0x41525348u; // u = unsigned
static constexpr uint16_t kVersion = 1;

} // namespace naina
