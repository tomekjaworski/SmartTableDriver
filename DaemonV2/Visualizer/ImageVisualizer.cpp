//
// Created by Tomek on 10/18/2020.
//

#include "ImageVisualizer.hpp"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

ImageVisualizer::ImageVisualizer(void) {

}

ImageVisualizer::~ImageVisualizer(void) {

}

void ImageVisualizer::ShowReconstruction(const ImageReconstructor& reconstructor) {

    cv::Mat source_image(reconstructor.GetHeight(), reconstructor.GetWidth(), CV_16UC1,
                         const_cast<uint16_t*>(reconstructor.GetData()));

    source_image *= 64;
    double min, max;
    cv::minMaxLoc(source_image, &min, &max);
    cv::Mat x()

    cv::Mat zoomed;
    cv::resize(source_image, zoomed, cv::Size(), 4, 4, CV_INTER_NN);
    cv::imshow("Test", zoomed);
    cv::waitKey(100);
}

