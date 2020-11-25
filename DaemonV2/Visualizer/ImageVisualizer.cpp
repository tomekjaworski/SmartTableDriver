//
// Created by Tomek on 10/18/2020.
//

#include "ImageVisualizer.hpp"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>  // Video write
#include <filesystem>
ImageVisualizer::ImageVisualizer(void) {
    this->minimum = 0;
    this->maximum = 1;
    this->window_created = false;
    this->zoom = 8;
    this->frame_counter = 0;

    std::error_code ec;
    std::filesystem::remove_all("data", ec);
    std::filesystem::create_directories("data");
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


void ImageVisualizer::ShowReconstruction(const ImageReconstructor &reconstructor) {

    if (!this->window_created) {
        cv::namedWindow("ImageVisualizer", cv::WINDOW_AUTOSIZE);
        this->window_created = true;
    }

    cv::Mat temp(reconstructor.GetHeight(), reconstructor.GetWidth(), CV_16UC1,
                 const_cast<uint16_t *>(reconstructor.GetData()));

    cv::Mat source_image = temp.clone();

    double min, max;
    cv::minMaxLoc(temp, &min, &max);
    this->minimum = std::min(min, this->minimum);
    this->maximum = std::max(max, this->maximum);

    source_image = 65535.0 * (temp - this->minimum) / (this->maximum - this->minimum);

    cv::Mat zoomed;
    cv::resize(source_image, zoomed, cv::Size(), this->zoom, this->zoom, CV_INTER_NN);
    cv::imshow("ImageVisualizer", zoomed);
    cv::waitKey(1);

//    std::vector<int> compression_params;
//    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
//    compression_params.push_back(95);

    char fname[PATH_MAX];
    snprintf(fname, PATH_MAX, "data/frame_%05d.png", this->frame_counter++);
    cv::imwrite(fname, zoomed);
    //
//
//    if (this->video_writer == nullptr) {
//        cv::resize(source_image, zoomed, cv::Size(), 16, 16, CV_INTER_NN);
//        cv::Size S(zoomed.size[0], zoomed.size[1]);
//
//        this->video_writer = std::make_shared<cv::VideoWriter>();
//        this->video_writer->open("aaa",
//                                 -1,//cv::VideoWriter::fourcc('M', 'P', '1', 'V'),
//                                 30, S, false);  //30 for 30 fps
//
//        if (!this->video_writer->isOpened())
//            throw std::runtime_error("Unable to open OpenCV Video Writer");
//
//        this->video_writer->write(zoomed);
//    }


}

