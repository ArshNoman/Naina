#pragma once // ensure this header is only included once
#include <cstdint>
#include <string>

namespace naina {

    uint64_t now_monotonic_ns(); // returns an 8-bit monotonic timestamp in nanoseconds
    std::string wall_time_compact(); // formats the current time into a compact string for logging (YYYYMMDD_HHMMSS)

}
