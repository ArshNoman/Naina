#include "blackbox_logger.hpp"
#include "display.hpp"
#include "packet_defs.hpp"
#include "time_utils.hpp"
#include "oak_ingest.hpp"
#include "pixhawk_ingest.hpp"

#include <atomic>
#include <csignal>
#include <iostream>
#include <mutex>
#include <optional>
#include <string>

using namespace naina;

static std::atomic<bool> g_run{true};

static void on_sigint(int) {
    g_run.store(false);
}

int main(int argc, char** argv) {
    std::signal(SIGINT, on_sigint);

    const std::string session = "logs/session_" + wall_time_compact();
    BlackBoxLogger logger(session);
    logger.write_text_line("Naina Phase 1 started");
    logger.write_text_line("Session dir: " + session);

    Display display;

    std::mutex oak_mtx;
    std::optional<OakFrame> last_oak;

    std::mutex imu_mtx;
#if NAINA_ENABLE_PIXHAWK
    uint64_t imu_count = 0;
    uint64_t imu_last_print_ns = now_monotonic_ns();
#endif

#if NAINA_ENABLE_OAK
    OakIngest oak;
    oak.start([&](const OakFrame& f) {
        {
            std::lock_guard<std::mutex> lock(oak_mtx);
            last_oak = f;
        }

        // Log left image
        if (!f.left_gray_u8.empty()) {
            logger.write_packet(PacketType::OakLeftGrayU8,
                                f.left_gray_u8.data,
                                (uint32_t)(f.left_gray_u8.total() * f.left_gray_u8.elemSize()),
                                f.host_t_ns);
        }

        // Log disparity
        if (!f.disparity_u16.empty()) {
            logger.write_packet(PacketType::OakDisparityU16,
                                f.disparity_u16.data,
                                (uint32_t)(f.disparity_u16.total() * f.disparity_u16.elemSize()),
                                f.host_t_ns);
        }

        // Log quality
        OakQualityPayload q{};
        q.depth_valid_ratio = f.depth_valid_ratio;
        q.mean_disparity = f.mean_disparity;
        q.exposure_time_us = f.exposure_time_us;
        q.analog_gain = f.analog_gain;
        q.width = (uint32_t)f.left_gray_u8.cols;
        q.height = (uint32_t)f.left_gray_u8.rows;

        logger.write_packet(PacketType::OakQuality, &q, (uint32_t)sizeof(q), f.host_t_ns);
    }, 640, 400, 30);
#else
    logger.write_text_line("Oak ingest disabled");
#endif

#if NAINA_ENABLE_PIXHAWK
    std::string serial = "/dev/tty.usbmodem01";
    int baud = 921600;
    if (argc >= 2) serial = argv[1];
    if (argc >= 3) baud = std::stoi(argv[2]);

    PixhawkIngest px;
    const bool ok = px.start_serial(serial, baud, [&](const ImuSample& s) {
        PixhawkImuPayload p{};
        p.gyro_rad_s[0] = s.gyro_rad_s[0];
        p.gyro_rad_s[1] = s.gyro_rad_s[1];
        p.gyro_rad_s[2] = s.gyro_rad_s[2];
        p.accel_m_s2[0] = s.accel_m_s2[0];
        p.accel_m_s2[1] = s.accel_m_s2[1];
        p.accel_m_s2[2] = s.accel_m_s2[2];
        p.imu_time_us = s.imu_time_us;

        logger.write_packet(PacketType::PixhawkImu, &p, (uint32_t)sizeof(p), s.host_t_ns);

        imu_count += 1;
    });

    if (!ok) {
        logger.write_text_line("Pixhawk connect failed. Check serial path and permissions.");
    } else {
        logger.write_text_line("Pixhawk connected on " + serial);
    }
#else
    logger.write_text_line("Pixhawk ingest disabled");
#endif

    while (g_run.load()) {
        OakFrame f{};
        bool have_oak = false;

#if NAINA_ENABLE_OAK
        {
            std::lock_guard<std::mutex> lock(oak_mtx);
            if (last_oak.has_value()) {
                f = *last_oak;
                have_oak = true;
            }
        }
#endif

        std::string overlay = "Naina Phase 1";
        if (have_oak) {
            overlay += "  valid=" + std::to_string(f.depth_valid_ratio);
            overlay += "  meanDisp=" + std::to_string(f.mean_disparity);
        }

#if NAINA_ENABLE_PIXHAWK
        const uint64_t now_ns = now_monotonic_ns();
        if (now_ns - imu_last_print_ns > 1000000000ull) {
            const double hz = (double)imu_count;
            imu_count = 0;
            imu_last_print_ns = now_ns;
            overlay += "  imuHz=" + std::to_string((int)hz);
        }
#endif

        if (have_oak) {
            display.show_left_and_disparity(f.left_gray_u8, f.disparity_u16, overlay);
        } else {
            display.show_left_and_disparity(cv::Mat(), cv::Mat(), overlay);
        }

        if (display.should_quit()) {
            g_run.store(false);
        }
    }

#if NAINA_ENABLE_OAK
    oak.stop();
#endif
#if NAINA_ENABLE_PIXHAWK
    // PixhawkIngest stops in destructor
#endif

    logger.write_text_line("Naina Phase 1 stopped");
    return 0;
}
