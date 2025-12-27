#include "time_utils.hpp"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace naina {

uint64_t now_monotonic_ns() {
    const auto t = std::chrono::steady_clock::now().time_since_epoch(); // get current time point
    return (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(t).count(); // converts t to nanoseconds
}

std::string wall_time_compact() {
    std::time_t tt = std::time(nullptr); // get current system time in seconds
    std::tm tm{}; // break down to calendar time
#if defined(_WIN32) // platform-specific thread safe variables are used
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    std::ostringstream oss; // string stream used to build the formatted timestamp
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S"); // format
    return oss.str(); // return
}

} 
