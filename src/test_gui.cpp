#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    std::cout << "OpenCV version: " << CV_VERSION << std::endl;
    cv::Mat img(400, 600, CV_8UC3, cv::Scalar(100, 150, 200));
    cv::putText(img, "Press any key to close", cv::Point(150, 200), 
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255,255,255), 2);
    cv::imshow("Test Window", img);
    std::cout << "Window should be visible now. Press any key in the window." << std::endl;
    int key = cv::waitKey(0);
    std::cout << "Key pressed: " << key << std::endl;
    return 0;
}