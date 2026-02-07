#include "oak_ingest.hpp"
#include <depthai/pipeline/node/Camera.hpp>
#include <depthai/pipeline/node/MonoCamera.hpp>

#if NAINA_ENABLE_OAK // isolates Oak and DepthAI dependencies from everything
                     // else

// Check DepathAI C++ docs for a better understanding of the syntax
// https://docs.luxonis.com/software-v3/depthai/api/cpp

#include "time_utils.hpp"
#include <atomic>
#include <depthai/depthai.hpp>
#include <opencv2/opencv.hpp>
#include <thread>
#define DEPTHAI_OPENCV_SUPPORT

namespace naina {

// this struct lets one dedicated background thread handle all OAK camera work
struct OakIngest::Impl {
  std::atomic<bool> running{
      false};         // check for if the camera processing is running
  std::thread worker; // owning the thread worker as an inherent object
};

// constructor
OakIngest::OakIngest() : impl_(new Impl) {}

// destructor
OakIngest::~OakIngest() {
  stop();
  delete impl_;
}

void OakIngest::start(const OakCallback &cb, int left_width, int left_height,
                      int fps) {
  if (impl_->running.load())
    return; // check if worker thread is running. If yes, return;
  impl_->running.store(true); // if not, set it to true

  impl_->worker = std::thread([=]() { // start a new thread
    dai::Pipeline p;                  // create a DepthAI pipeline

    auto monoL = p.create<dai::node::MonoCamera>();      // a mono lens pipeline
    auto monoR = p.create<dai::node::MonoCamera>();      // a mono lens pipeline
    monoL->setBoardSocket(dai::CameraBoardSocket::LEFT); // specify left lens
    monoR->setBoardSocket(dai::CameraBoardSocket::RIGHT); // specify right lens

    monoL->setResolution(dai::MonoCameraProperties::SensorResolution::
                             THE_400_P); // set left resolution to 640×400
    monoR->setResolution(dai::MonoCameraProperties::SensorResolution::
                             THE_400_P); // set right resolution to 640×400
    monoL->setFps((float)fps);           // set left FPS to fps
    monoR->setFps((float)fps);           // set right FPS to fps

    auto stereo =
        p.create<dai::node::StereoDepth>(); // create a stereo lens pipeline
    stereo->setDefaultProfilePreset(
        dai::node::StereoDepth::PresetMode::ROBOTICS); // set the profile to
                                                       // ROBOTICS
    stereo->setLeftRightCheck(true); // turn ON compute and combines disparities
                                     // in both L-R and R-L directions
    stereo->setSubpixel(
        false); // turn OFF computes disparity with sub-pixel interpolation
    stereo->setExtendedDisparity(
        false); // turn OFF disparity range increase from 95 to 190

    monoL->out.link(
        stereo->left); // define data flow so that synchronized left frames are
                       // fed into the stereo matching algorithm
    monoR->out.link(
        stereo->right); // define data flow so that synchronized right frames
                        // are fed into the stereo matching algorithm

    auto qLeft = stereo->rectifiedLeft.createOutputQueue(
        4, false); // receives rectified left images from the OAK
    auto qDisp = stereo->disparity.createOutputQueue(
        4, false); // receives rectified disparity images from the OAK

    dai::Device dev;      // initiate a DepthAI object
    dev.startPipeline(p); // uploads the pipeline to the device and starts
                          // execution on the OAK hardware

    while (impl_->running.load()) { // while camera processing is running
      auto left = qLeft->tryGet<dai::ImgFrame>(); // get left frame
      auto disp = qDisp->tryGet<dai::ImgFrame>(); // get disparity frame

      if (!left || !disp) { // if one or both frames not received
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        continue; // wait and continue for a millisecond
      }

      OakFrame f{}; // OakFrame data container that is zero-initialized
      f.host_t_ns = now_monotonic_ns(); // append current timestamp

      cv::Mat leftMat = left->getCvFrame(); // convert DepthAI frame to OpenCV
                                            // matrix (IGNORE RED LINE)
      if (leftMat.channels() == 3) {        // if colored image
        cv::cvtColor(leftMat, f.left_gray_u8,
                     cv::COLOR_BGR2GRAY); // convert to grayscale
      } else {
        f.left_gray_u8 = leftMat;
      }

      // disparity from DepthAI is usually 8 bit, but we store as 16 bit for
      // consistency
      cv::Mat dispMat = disp->getFrame(); // retrieve raw disparity frame
      if (dispMat.type() == CV_8UC1) {    // common 8-bit disparity format
        dispMat.convertTo(f.disparity_u16, CV_16UC1,
                          256.0);              // scale into 16-bit range
      } else if (dispMat.type() == CV_16UC1) { // if already 16-bit
        f.disparity_u16 = dispMat;
      } else {
        dispMat.convertTo(f.disparity_u16, CV_16UC1); // fallback conversion
      }

      const int total =
          f.disparity_u16.rows * f.disparity_u16.cols; // total disparity pixels
      int valid = 0;    // count of non-zero disparity pixels
      double sum = 0.0; // sum of valid disparity values
      for (int y = 0; y < f.disparity_u16.rows; ++y) { // iterate rows
        const uint16_t *row =
            f.disparity_u16.ptr<uint16_t>(y); // pointer to the current row
        for (int x = 0; x < f.disparity_u16.cols; ++x) { // iterate columns
          const uint16_t d = row[x]; // disparity value at pixel
          if (d > 0) {               // consider only valid disparity
            valid += 1;              // increment valid pixel count
            sum += (double)d;        // accumulate disparity value
          }
        }
      }
      f.depth_valid_ratio = total > 0 ? (float)valid / (float)total
                                      : 0.0f; // ratio of valid depth pixels
      f.mean_disparity = valid > 0 ? (float)(sum / (double)valid)
                                   : 0.0f; // mean disparity over valid pixels

      // Exposure and gain are not always available directly per frame in this
      // API path
      f.exposure_time_us = 0.0f; // placeholder exposure
      f.analog_gain = 0.0f;      // placeholder gain

      cb(f); // deliver populated frame to user callback
    }
  });
}

void OakIngest::stop() {
  if (!impl_->running.load())
    return;
  impl_->running.store(false);
  if (impl_->worker.joinable())
    impl_->worker.join();
}

} // namespace naina

#else

namespace naina {
struct OakIngest::Impl {};
OakIngest::OakIngest() : impl_(new Impl) {}
OakIngest::~OakIngest() { delete impl_; }
void OakIngest::start(const OakCallback &, int, int, int) {}
void OakIngest::stop() {}
} // namespace naina

#endif
