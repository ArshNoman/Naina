#pragma once
#include <cstdint>
#include <functional>
#include <opencv2/core.hpp>

namespace naina {

struct OakFrame { // data container for an Oak Frame
    uint64_t host_t_ns; // host-side timestamp in nanoseconds for when the frame was taken
    cv::Mat left_gray_u8;     // left camera image in 8-bit grey-scale
    cv::Mat disparity_u16;    // 16-bit stereo disparity image
    float depth_valid_ratio; // fraction of pixels in disparity_u16 that contain valid depth
    float mean_disparity; // average disaprty value from disparity_u16 
    float exposure_time_us; // exposure time in microseconds used by the camera when this frame was captured
    float analog_gain; // analog sensor gain applied for this frame. ^ gain means ^ low-light conditions and noise
};

// callback type alias
using OakCallback = std::function<void(const OakFrame&)>; // structure of any function taking in OakFrame and outputting void
// the callback function determines the handling of each frame (logging, visual odometry, telemetry)
// having a callback functione essentially separates the implementations of frame acquisitions and frame processing 

class OakIngest {
public:
    OakIngest(); // constructor
    ~OakIngest(); // destructor, shuts down the ingest if running

    // starts the camera pipeline and stores the callback function passed as an argument
    void start(const OakCallback& cb, int left_width, int left_height, int fps);

    // stops the camera pipeline
    void stop();

private: // PIMPL pattern
    struct Impl;
    Impl* impl_;
};

}
