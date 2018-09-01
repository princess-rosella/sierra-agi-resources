//
//  PictureDecoder.cpp
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#include "PictureDecoder.hpp"

using namespace AGI::Resources;

PictureDecoder::PictureDecoder(std::vector<uint8_t>&& buffer) : _buffer(std::move(buffer)) {
}

PictureDecoder::PictureDecoder(const std::vector<uint8_t>& buffer) : _buffer(buffer) {
}

void PictureDecoder::decode(PictureCallback& callback) {
    uint8_t* it           = _buffer.data();
    uint8_t* end          = it + _buffer.size();
    uint8_t patternCode   = 0;
    uint8_t patternNumber = 0;

    while (it != end) {
        uint8_t command = *it++;

        switch (command) {
        case 0xf0:
            callback.setColor(*it++);
            callback.setScreen(true);
            continue;
        case 0xf1:
            callback.setScreen(false);
            continue;
        case 0xf2:
            callback.setPriority(*it++);
            callback.setPriority(true);
            continue;
        case 0xf3:
            callback.setPriority(false);
            continue;
        case 0xf9:
            patternCode = *it++;
            callback.setPattern(patternCode, patternNumber);
            continue;
        case 0xfc:
            callback.setColor(*it++);
            callback.setPriority(*it++);
            break;
        case 0xff:
            callback.end();
            return;
        }

        uint8_t* start = it;
        while (*it < 0xf0 && it != end)
            ++it;

        switch (command) {
        case 0xf4:
            callback.drawYCorner(start, it - start);
            break;
        case 0xf5:
            callback.drawXCorner(start, it - start);
            break;
        case 0xf6:
            callback.drawLineAbsolute(start, it - start);
            break;
        case 0xf7:
            callback.drawLineShort(start, it - start);
            break;
        case 0xf8:
        case 0xfc:
            for (; (it - start) >= 2; start += 2)
                callback.drawFill(start[0], start[1]);
            break;
        case 0xfa:
            while (start != it) {
                if (patternCode & 0x20) {
                    patternNumber = *start++;
                    patternNumber >>= 1;
                    callback.setPattern(patternCode, patternNumber);
                }

                if ((it - start) < 2)
                    break;

                callback.plotPattern(start[0], start[1]);
                start += 2;
            }
            break;
        }
    }
}
