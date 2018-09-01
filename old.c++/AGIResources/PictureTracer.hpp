//
//  PictureTracer.hpp
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#ifndef __AGIResources__PictureTracer_hpp__
#define __AGIResources__PictureTracer_hpp__

#include "PictureDecoder.hpp"

namespace AGI { namespace Resources {

    class GameInfo;

    /**
     * This class transform a picture instructions in a pixel instructinos.
     */
    class PictureTracer : public PictureCallback {
    private:
        bool _screen;
        uint8_t _screenColor;
        bool _priority;
        uint8_t _priorityColor;
        uint8_t _patternCode;
        uint8_t _patternNumber;
        bool _version3;

    public:
        PictureTracer(const GameInfo& info);

    public:
        virtual uint8_t pixelScreen(uint8_t x, uint8_t y) = 0;
        virtual void setPixelScreen(uint8_t x, uint8_t y, uint8_t color) = 0;
        virtual uint8_t pixelPriority(uint8_t x, uint8_t y) = 0;
        virtual void setPixelPriority(uint8_t x, uint8_t y, uint8_t priority) = 0;

    public:
        virtual void setColor(uint8_t color) override;
        virtual void setScreen(bool) override;
        virtual void setPriority(uint8_t priority) override;
        virtual void setPriority(bool) override;
        virtual void drawYCorner(uint8_t* coordinates, size_t count) override;
        virtual void drawXCorner(uint8_t* coordinates, size_t count) override;
        virtual void drawLineAbsolute(uint8_t* coordinates, size_t count) override;
        virtual void drawLineShort(uint8_t* coordinates, size_t count) override;
        virtual void drawFill(uint8_t x, uint8_t y) override;
        virtual void setPattern(uint8_t code, uint8_t number) override;
        virtual void plotPattern(uint8_t x, uint8_t y) override;
        virtual void end() override;

    private:
        void putPixel(uint8_t x, uint8_t y);
        void drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
        bool drawFillCheck(uint8_t x, uint8_t y);
    };

}}

#endif /* __AGIResources__PictureTracer_hpp__ */
