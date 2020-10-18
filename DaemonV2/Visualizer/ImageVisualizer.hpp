//
// Created by Tomek on 10/18/2020.
//

#ifndef UNTITLED_IMAGEVISUALIZER_HPP
#define UNTITLED_IMAGEVISUALIZER_HPP

#include "../ImageReconstructor.hpp"

class ImageVisualizer {

public:
    ImageVisualizer(void);
    ~ ImageVisualizer(void);

    void ShowReconstruction(const ImageReconstructor& reconstructor);
};


#endif //UNTITLED_IMAGEVISUALIZER_HPP
