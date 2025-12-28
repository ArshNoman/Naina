#pragma once

#include <cstdint>
#include <functional>

namespace naina {

    struct ImuSample {

        uint64_t host_t_ns; // timestamp captured on the host machine, in nanoseconds
        uint64_t imu_time_us; // timestamp reported by the IMU itself, in microseconds
        float gyro_rad_s[3]; // angular velocity around x/y/z axes, stored as a fixed-size C-style array
        float accel_m_s2[3]; // linear acceleration along x/y/z axes, also a fixed-size array
    };

    // callback type alias
    using ImuCallback = std::function<void(const ImuSample&)>; // structure of any function taking in ImuSample and outputting void
    // the callback function (cb) determines the handling of each sample data (logging, odometry, telemetry)
    // having a callback functione essentially separates the implementations of data acquisitions and data processing 

    class PixhawkIngest {

    public:
        PixhawkIngest(); // constructor

        ~PixhawkIngest(); // destructor

        bool start_serial(const std::string& serial_path, int baudrate, const ImuCallback& cb);
        // starts reading IMU data from the Pixhawk
        // returns true/false instead of throwing, indicating success or failure
        // serial_path: names the device (e.g., /dev/ttyUSB0)
        // baudrate: configures serial speed
        // cb: is invoked whenever a new ImuSample is available

        void stop(); // stops ingestion and releases any associated resources

    private:
        struct Impl; // PIMPL
        
        Impl* impl_;
    };

}
