//
// Created by Tomek on 10/18/2020.
//

#ifndef UNTITLED_IMAGEVISUALIZER_HPP
#define UNTITLED_IMAGEVISUALIZER_HPP

#include "../ImageReconstructor.hpp"
#include <memory>

// OpenCV forward
namespace cv {
    class VideoWriter;
}

class ImageVisualizer {
    double minimum, maximum;
    bool window_created;
    int zoom;
    std::shared_ptr<cv::VideoWriter> video_writer;
    int frame_counter;

public:
    ImageVisualizer(void);
    ~ ImageVisualizer(void);

    void ShowReconstruction(const ImageReconstructor& reconstructor);

    void DoZoomIn(void);
    void DoZoomOut(void);
    void DoResetNormalization(void);
};


#endif //UNTITLED_IMAGEVISUALIZER_HPP
