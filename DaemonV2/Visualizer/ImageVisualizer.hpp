//
// Created by Tomek on 10/18/2020.
//

#ifndef UNTITLED_IMAGEVISUALIZER_HPP
#define UNTITLED_IMAGEVISUALIZER_HPP

#include "../ImageReconstructor.hpp"

class ImageVisualizer {
    double minimum, maximum;
    bool window_created;
    int zoom;

public:
    ImageVisualizer(void);
    ~ ImageVisualizer(void);

    void ShowReconstruction(const ImageReconstructor& reconstructor);

    void DoZoomIn(void);
    void DoZoomOut(void);
    void DoResetNormalization(void);
};


#endif //UNTITLED_IMAGEVISUALIZER_HPP
