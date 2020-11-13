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
    this->window_created = false;
    this->zoom = 8;
}

ImageVisualizer::~ImageVisualizer(void) {

}

void ImageVisualizer::DoZoomIn(void) {
    this->zoom = (this->zoom == 15) ? 15 : this->zoom + 1;
}

void ImageVisualizer::DoZoomOut(void) {
    this->zoom = (this->zoom == 1) ? 1 : this->zoom - 1;
}

void ImageVisualizer::DoResetNormalization(void) {
    this->minimum = 0;
    this->maximum = 1;
}


void ImageVisualizer::ShowReconstruction(const ImageReconstructor& reconstructor) {

    if (!this->window_created) {
        cv::namedWindow("ImageVisualizer", cv::WINDOW_AUTOSIZE);
        this->window_created = true;
    }


    cv::Mat temp(reconstructor.GetHeight(), reconstructor.GetWidth(), CV_16UC1,
                         const_cast<uint16_t*>(reconstructor.GetData()));

    cv::Mat source_image = temp.clone();

    double min, max;
    cv::minMaxLoc(source_image, &min, &max);
    this->minimum = std::min(min, this->minimum);
    this->maximum = std::max(max, this->maximum);

    source_image = 65535.0 * (source_image - this->minimum) / (this->maximum - this->minimum);

    cv::Mat zoomed;
    cv::resize(source_image, zoomed, cv::Size(), this->zoom, this->zoom, CV_INTER_NN);
    cv::imshow("ImageVisualizer", zoomed);
    cv::waitKey(1);
}

