#include <opencv2/opencv.hpp>
#include <iostream>

int main()
{
    cv::VideoCapture cap("../data/sequence.mp4");

    if (!cap.isOpened())
    {
        std::cerr << "Failed to open video" << std::endl;
        return 1;
    }

    cv::Mat frame;
    cv::Mat gray;

    while (true)
    {
        if (!cap.read(frame))
        {
            break;
        }

        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        cv::imshow("Frame Stream", gray);

        char key = static_cast<char>(cv::waitKey(30));
        if (key == 27)
        {
            break;
        }
    }

    return 0;
}
