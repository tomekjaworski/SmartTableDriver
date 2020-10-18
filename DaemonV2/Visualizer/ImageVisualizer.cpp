//
// Created by Tomek on 10/18/2020.
//

#include "ImageVisualizer.hpp"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

ImageVisualizer::ImageVisualizer(void) {
    this->minimum = 0;
    this->maximum = 1;
}

ImageVisualizer::~ImageVisualizer(void) {

}

void ImageVisualizer::ShowReconstruction(const ImageReconstructor& reconstructor) {

    cv::Mat temp(reconstructor.GetHeight(), reconstructor.GetWidth(), CV_16UC1,
                         const_cast<uint16_t*>(reconstructor.GetData()));

    cv::Mat source_image = temp.clone();

    double min, max;
    cv::minMaxLoc(source_image, &min, &max);
    this->minimum = std::min(min, this->minimum);
    this->maximum = std::max(max, this->maximum);

    source_image = 65535.0 * (source_image - this->minimum) / (this->maximum - this->minimum);

    cv::Mat zoomed;
    cv::resize(source_image, zoomed, cv::Size(), 4, 4, CV_INTER_NN);
    cv::imshow("Test", zoomed);
    cv::waitKey(100);
}

