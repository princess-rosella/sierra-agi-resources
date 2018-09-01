//
//  PictureRasterizer.hpp
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#ifndef __AGIResources__PictureRasterizer_hpp__
#define __AGIResources__PictureRasterizer_hpp__

#include "PictureTracer.hpp"

namespace AGI { namespace Resources {

    class GameInfo;

    /**
     * This class transform pixels instructions into a bitmap.
     */
    class PictureRasterizer : public PictureTracer {
    private:
        uint8_t* _screen;
        uint8_t* _priority;

    public:
        PictureRasterizer(const GameInfo& info, uint8_t* screen, uint8_t* priority, bool clear = true);

    public:
        virtual uint8_t pixelScreen(uint8_t x, uint8_t y) override;
        virtual void setPixelScreen(uint8_t x, uint8_t y, uint8_t color) override;
        virtual uint8_t pixelPriority(uint8_t x, uint8_t y) override;
        virtual void setPixelPriority(uint8_t x, uint8_t y, uint8_t priority) override;
    };

}}

#endif /* __AGIResources__PictureRasterizer_hpp__ */
