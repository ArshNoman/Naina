#include "pixhawk_ingest.hpp"

#if NAINA_ENABLE_PIXHAWK // isolates Oak and DepthAI dependencies from
                         // everything else

#include "time_utils.hpp"
#include <atomic>
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <thread>

namespace naina {

struct PixhawkIngest::Impl {
  std::atomic<bool> running{
      false}; // indicates whether ingestion is active, safe across threads
  std::unique_ptr<mavsdk::Mavsdk>
      sdk; // owns the MAVSDK instance managing connections
  std::unique_ptr<mavsdk::Telemetry>
      tel; // owns the telemetry plugin bound to a system
};

PixhawkIngest::PixhawkIngest()
    : impl_(new Impl) {} // constructor: allocate hidden implementation
PixhawkIngest::~PixhawkIngest() {
  stop();
  delete impl_;
} // destructor: ensure clean shutdown, then free impl

bool PixhawkIngest::start_serial(const std::string &serial_path, int baudrate,
                                 const ImuCallback &cb) {
  if (impl_->running.load())
    return true;              // do nothing if it is already running
  impl_->running.store(true); // mark ingestion as active

  mavsdk::Mavsdk::Configuration config{mavsdk::ComponentType::GroundStation};
  impl_->sdk = std::make_unique<mavsdk::Mavsdk>(config); // create MAVSDK context
  const std::string url = "serial://" + serial_path + ":" +
                          std::to_string(baudrate); // MAVSDK serial URL

  const auto conn =
      impl_->sdk->add_any_connection(url); // attempt to open serial connection
  if (conn != mavsdk::ConnectionResult::Success) {
    impl_->running.store(false); // rollback running state on failure
    return false;
  }

  auto sys = impl_->sdk->first_autopilot(
      10.0); // wait up to 10s for the Pixhawk to appear
  if (!sys) {
    impl_->running.store(false); // rollback if no Pixhawk is discovered
    return false;
  }

  impl_->tel = std::make_unique<mavsdk::Telemetry>(
      *sys);                       // bind telemetry plugin to the system
  impl_->tel->set_rate_imu(100.0); // request IMU updates at 100 Hz

  impl_->tel->subscribe_imu([cb](mavsdk::Telemetry::Imu
                                     imu) { // register IMU callback
    ImuSample s{};                          // zero-initialized output sample
    s.host_t_ns = now_monotonic_ns();       // capture host-side receipt time
    s.imu_time_us = 0; // IMU timestamp not provided by MAVSDK here

    s.gyro_rad_s[0] = imu.angular_velocity_frd
                          .forward_rad_s; // body-frame forward angular rate
    s.gyro_rad_s[1] =
        imu.angular_velocity_frd.right_rad_s; // body-frame right angular rate
    s.gyro_rad_s[2] =
        imu.angular_velocity_frd.down_rad_s; // body-frame down angular rate

    s.accel_m_s2[0] =
        imu.acceleration_frd.forward_m_s2; // body-frame forward acceleration
    s.accel_m_s2[1] =
        imu.acceleration_frd.right_m_s2; // body-frame right acceleration
    s.accel_m_s2[2] =
        imu.acceleration_frd.down_m_s2; // body-frame down acceleration

    cb(s); // forward the converted sample to user-provided callback
  });

  return true; // ingestion successfully started
}

void PixhawkIngest::stop() {
  if (!impl_->running.load())
    return;                    // do nothing if it is already running
  impl_->running.store(false); // signal shutdown
  if (impl_->tel) {
    impl_->tel->subscribe_imu(nullptr); // unregister IMU callback
  }
  impl_->tel.reset(); // destroy telemetry plugin
  impl_->sdk.reset(); // destroy MAVSDK instance and close connections
}

} // namespace naina

#else // fallback build when Pixhawk support is disabled

namespace naina {
struct PixhawkIngest::Impl {}; // empty implementation placeholder
PixhawkIngest::PixhawkIngest() : impl_(new Impl) {} // construct stub impl
PixhawkIngest::~PixhawkIngest() { delete impl_; }   // destroy stub impl
bool PixhawkIngest::start_serial(const std::string &, int,
                                 const ImuCallback &) {
  return false;
} // always fail
void PixhawkIngest::stop() {} // no-op
} // namespace naina

#endif
