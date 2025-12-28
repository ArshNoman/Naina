#pragma once // ensure this header is only included once

#include <opencv2/core.hpp> 
#include <string> 

namespace naina {

class Display {
public:
    Display(); // sets up display-related state

    void show_left_and_disparity(const cv::Mat& left_gray_u8, const cv::Mat& disparity_u16, const std::string& overlay);
    // displays the left grayscale image and the disparity image side by side
    // overlay is a text string drawn on top (e.g., status or debug info)

    bool should_quit() const; // returns true if the user requested to exit

private:
    mutable int last_key_ = 0; // stores last key pressed, mutable to allow updates in const methods
};

}
