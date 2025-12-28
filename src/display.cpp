#include "display.hpp" 
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp> 

namespace naina {

Display::Display() {
    cv::namedWindow("Naina Left", cv::WINDOW_NORMAL); // create resizable window for left image
    cv::namedWindow("Naina Disparity", cv::WINDOW_NORMAL); // create resizable window for disparity view
}

void Display::show_left_and_disparity(const cv::Mat& left_gray_u8, const cv::Mat& disparity_u16, const std::string& overlay) {
    if (!left_gray_u8.empty()) { // only render if a left image is provided
        cv::Mat left_vis; // visualization image
        cv::cvtColor(left_gray_u8, left_vis, cv::COLOR_GRAY2BGR); // convert grayscale to color for drawing
        cv::putText(left_vis, overlay, {10, 25}, cv::FONT_HERSHEY_SIMPLEX, 0.6, {255, 255, 255}, 2); // draw overlay text
        cv::imshow("Naina Left", left_vis); // show left image window
    }

    if (!disparity_u16.empty()) { // only render if a disparity image is provided
        cv::Mat disp8, dispColor; // temporary images for visualization
        double minv = 0.0, maxv = 0.0; // track disparity range
        cv::minMaxLoc(disparity_u16, &minv, &maxv); // find min and max disparity values
        const double scale = (maxv > 0.0) ? (255.0 / maxv) : 1.0; // scale to 8-bit range
        disparity_u16.convertTo(disp8, CV_8UC1, scale); // convert 16-bit disparity to 8-bit
        cv::applyColorMap(disp8, dispColor, cv::COLORMAP_TURBO); // apply color map for easier viewing
        cv::imshow("Naina Disparity", dispColor); // show disparity window
    }

    last_key_ = cv::waitKey(1); // poll keyboard input without blocking
}

bool Display::should_quit() const {
    return last_key_ == 27 || last_key_ == 'q' || last_key_ == 'Q'; // exit on ESC or Q or q
}

} 
