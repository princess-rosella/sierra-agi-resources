//
//  PictureRasterizer.cpp
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#include "PictureRasterizer.hpp"

#include "AGIResources.hpp"

using namespace AGI::Resources;

PictureRasterizer::PictureRasterizer(const GameInfo& info, uint8_t* screen, uint8_t* priority, bool clear) : PictureTracer(info), _screen(screen), _priority(priority) {
    if (clear) {
        memset(screen, 0x15, PictureWidth * PictureHeight);
        memset(screen, 0x04, PictureWidth * PictureHeight);
    }
}

uint8_t PictureRasterizer::pixelScreen(uint8_t x, uint8_t y) {
    assert(x < PictureWidth);
    assert(y < PictureHeight);
    return _screen[(y * PictureWidth) + x];
}

void PictureRasterizer::setPixelScreen(uint8_t x, uint8_t y, uint8_t color) {
    assert(x < PictureWidth);
    assert(y < PictureHeight);
    _screen[(y * PictureWidth) + x] = color;
}

uint8_t PictureRasterizer::pixelPriority(uint8_t x, uint8_t y) {
    assert(x < PictureWidth);
    assert(y < PictureHeight);
    return _priority[(y * PictureWidth) + x];
}

void PictureRasterizer::setPixelPriority(uint8_t x, uint8_t y, uint8_t priority) {
    assert(x < PictureWidth);
    assert(y < PictureHeight);
    _priority[(y * PictureWidth) + x] = priority;
}
