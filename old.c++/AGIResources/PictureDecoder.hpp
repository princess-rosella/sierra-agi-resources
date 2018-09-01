//
//  PictureDecoder.hpp
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#ifndef __AGIResources__PictureDecoder_hpp__
#define __AGIResources__PictureDecoder_hpp__

#include <stdint.h>
#include <stdlib.h>

#include <vector>

namespace AGI { namespace Resources {

    class PictureCallback {
    public:
        virtual ~PictureCallback() {}

    public:
        virtual void setColor(uint8_t color) = 0;
        virtual void setScreen(bool) = 0;
        virtual void setPriority(uint8_t priority) = 0;
        virtual void setPriority(bool) = 0;
        virtual void drawYCorner(uint8_t* coordinates, size_t count) = 0;
        virtual void drawXCorner(uint8_t* coordinates, size_t count) = 0;
        virtual void drawLineAbsolute(uint8_t* coordinates, size_t count) = 0;
        virtual void drawLineShort(uint8_t* coordinates, size_t count) = 0;
        virtual void drawFill(uint8_t x, uint8_t y) = 0;
        virtual void setPattern(uint8_t code, uint8_t number) = 0;
        virtual void plotPattern(uint8_t x, uint8_t y) = 0;
        virtual void end() = 0;
    };

    /**
     * This class transform a picture resource in a series of instructions that can be used
     * by a picture tracer.
     */
    class PictureDecoder {
    private:
        std::vector<uint8_t> _buffer;

    public:
        PictureDecoder(std::vector<uint8_t>&& buffer);
        PictureDecoder(const std::vector<uint8_t>& buffer);

    public:
        void decode(PictureCallback& callback);
    };

}}

#endif /* __AGIResources__PictureDecoder_hpp__ */
