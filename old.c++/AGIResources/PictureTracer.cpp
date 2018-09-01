//
//  PictureRasterizer.cpp
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#include "PictureTracer.hpp"

#include "AGIResources.hpp"
#include "GameInfo.hpp"

using namespace AGI::Resources;

PictureTracer::PictureTracer(const GameInfo& info) : _screen(false), _screenColor(0), _priority(false), _priorityColor(0), _patternCode(0), _patternNumber(0) {
    _version3 = info.version() >= 0x3000;
}

void PictureTracer::setScreen(bool screen) {
    _screen = screen;
}

void PictureTracer::setColor(uint8_t color) {
    _screenColor = color;
}

void PictureTracer::setPriority(bool priority) {
    _priority = priority;
}

void PictureTracer::setPriority(uint8_t priority) {
    _priorityColor = priority;
}

void PictureTracer::putPixel(uint8_t x, uint8_t y) {
    if (x >= PictureWidth || y >= PictureHeight)
        return;

    if (_screen)
        setPixelScreen(x, y, _screenColor);
    if (_priority)
        setPixelPriority(x, y, _priorityColor);
}

template <typename T>
inline T clip(T value, T min, T max) {
    if (value < min)
        return min;
    else if (value > max)
        return max;
    return value;
}

template <typename T>
inline void swap(T& v1, T& v2) {
    T temp = v2;
    v2 = v1;
    v1 = temp;
}

void PictureTracer::drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    x1 = clip<uint8_t>(x1, 0, PictureWidth - 1);
    x2 = clip<uint8_t>(x2, 0, PictureWidth - 1);
    y1 = clip<uint8_t>(y1, 0, PictureHeight - 1);
    y2 = clip<uint8_t>(y2, 0, PictureHeight - 1);

    int i, x, y, deltaX, deltaY, stepX, stepY, errorX, errorY, detdelta;

    // Vertical line

    if (x1 == x2) {
        if (y1 > y2)
            swap<uint8_t>(y1, y2);

        for (; y1 <= y2; y1++)
            putPixel(x1, y1);

        return;
    }

    // Horizontal line

    if (y1 == y2) {
        if (x1 > x2)
            swap<uint8_t>(x1, x2);

        for (; x1 <= x2; x1++)
            putPixel(x1, y1);

        return;
    }

    y = y1;
    x = x1;

    stepY = 1;
    deltaY = y2 - y1;
    if (deltaY < 0) {
        stepY = -1;
        deltaY = -deltaY;
    }

    stepX = 1;
    deltaX = x2 - x1;
    if (deltaX < 0) {
        stepX = -1;
        deltaX = -deltaX;
    }

    if (deltaY > deltaX) {
        i = deltaY;
        detdelta = deltaY;
        errorX = deltaY / 2;
        errorY = 0;
    }
    else {
        i = deltaX;
        detdelta = deltaX;
        errorX = 0;
        errorY = deltaX / 2;
    }

    putPixel(x, y);

    do {
        errorY += deltaY;
        if (errorY >= detdelta) {
            errorY -= detdelta;
            y += stepY;
        }

        errorX += deltaX;
        if (errorX >= detdelta) {
            errorX -= detdelta;
            x += stepX;
        }

        putPixel(x, y);
        i--;
    } while (i > 0);
}

void PictureTracer::drawYCorner(uint8_t* coordinates, size_t count) {
    if (count < 2)
        return;

    uint8_t x1, x2, y1, y2;
    x1 = coordinates[0];
    y1 = coordinates[1];
    putPixel(x1, y1);

    uint8_t* end = coordinates + count;
    coordinates += 2;

    while (coordinates != end) {
        y2 = *coordinates++;
        drawLine(x1, y1, x1, y2);

        if (coordinates == end)
            break;

        y1 = y2;
        x2 = *coordinates++;
        drawLine(x1, y1, x2, y1);
        x1 = x2;
    }
}

void PictureTracer::drawXCorner(uint8_t* coordinates, size_t count) {
    if (count < 2)
        return;

    uint8_t x1, x2, y1, y2;
    x1 = coordinates[0];
    y1 = coordinates[1];
    putPixel(x1, y1);

    uint8_t* end = coordinates + count;
    coordinates += 2;

    while (coordinates != end) {
        x2 = *coordinates++;
        drawLine(x1, y1, x2, y1);

        if (coordinates == end)
            break;

        x1 = x2;
        y2 = *coordinates++;
        drawLine(x1, y1, x1, y2);
        y1 = y2;
    }
}

void PictureTracer::drawLineAbsolute(uint8_t* coordinates, size_t count) {
    if (count < 2)
        return;

    uint8_t x1, x2, y1, y2;
    x1 = coordinates[0];
    y1 = coordinates[1];
    putPixel(x1, y1);

    uint8_t* end = coordinates + count;
    coordinates += 2;

    while (coordinates != end) {
        x2 = *coordinates++;
        if (coordinates == end)
            break;

        y2 = *coordinates++;
        drawLine(x1, y1, x2, y2);
        x1 = x2;
        y1 = y2;
    }
}

void PictureTracer::drawLineShort(uint8_t* coordinates, size_t count) {
    if (count < 2)
        return;

    uint8_t x1, y1, disp;
    int8_t dx, dy;
    x1 = coordinates[0];
    y1 = coordinates[1];
    putPixel(x1, y1);

    uint8_t* end = coordinates + count;
    coordinates += 2;

    while (coordinates != end) {
        disp = *coordinates++;

        dx = ((disp & 0xf0) >> 4) & 0x0f;
        dy = (disp & 0x0f);

        if (dx & 0x08)
            dx = -(dx & 0x07);
        if (dy & 0x08)
            dy = -(dy & 0x07);

        drawLine(x1, y1, x1 + dx, y1 + dy);
        x1 += dx;
        y1 += dy;
    }
}

bool PictureTracer::drawFillCheck(uint8_t x, uint8_t y) {
    if (x >= PictureWidth || y >= PictureHeight)
        return false;

    uint8_t screenColor    = pixelScreen(x, y);
    uint8_t screenPriority = pixelPriority(x, y);

    if (!_priority && _screen && _screenColor != 15)
        return (screenColor == 15);

    if (_priority && !_screen && _priorityColor != 4)
        return screenPriority == 4;

    return (_screen && screenColor == 15 && _screenColor != 15);
}

void PictureTracer::drawFill(uint8_t x, uint8_t y) {
    if (!_screen && !_priority)
        return;

    std::vector<std::pair<uint8_t, uint8_t>> stack;
    stack.emplace_back(x, y);

    while (stack.size()) {
        std::pair<uint8_t, uint8_t> p = *(stack.begin());
        stack.erase(stack.begin());

        if (!drawFillCheck(p.first, p.second))
            continue;

        unsigned int c;
        bool newspanUp, newspanDown;

        for (c = p.first - 1; drawFillCheck(c, p.second); c--)
            ;

        newspanUp = newspanDown = true;
        for (c++; drawFillCheck(c, p.second); c++) {
            putPixel(c, p.second);
            if (drawFillCheck(c, p.second - 1)) {
                if (newspanUp) {
                    stack.emplace_back(c, p.second - 1);
                    newspanUp = false;
                }
            }
            else {
                newspanUp = true;
            }

            if (drawFillCheck(c, p.second + 1)) {
                if (newspanDown) {
                    stack.emplace_back(c, p.second + 1);
                    newspanDown = false;
                }
            }
            else {
                newspanDown = true;
            }
        }
    }
}

void PictureTracer::setPattern(uint8_t code, uint8_t number) {
    _patternCode = code;
    _patternNumber = number;
}

void PictureTracer::plotPattern(uint8_t x, uint8_t y) {
    static const uint16_t binaryList[] = {
        0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100,
        0x0080, 0x0040, 0x0020, 0x0010, 0x008, 0x004, 0x002, 0x001
    };

    static const uint8_t circleList[] = {
        0, 1, 4, 9, 16, 25, 37, 50
    };

    static const uint16_t circleData[] = {
        0x8000,
        0xE000, 0xE000, 0x0E000,
        0x7000, 0xF800, 0x0F800, 0x0F800, 0x07000,
        0x3800, 0x7C00, 0x0FE00, 0x0FE00, 0x0FE00, 0x07C00, 0x03800,
        0x1C00, 0x7F00, 0x0FF80, 0x0FF80, 0x0FF80, 0x0FF80, 0x0FF80, 0x07F00, 0x01C00,
        0x0E00, 0x3F80, 0x07FC0, 0x07FC0, 0x0FFE0, 0x0FFE0, 0x0FFE0, 0x07FC0, 0x07FC0, 0x03F80, 0x1F00, 0x0E00,
        0x0F80, 0x3FE0, 0x07FF0, 0x07FF0, 0x0FFF8, 0x0FFF8, 0x0FFF8, 0x0FFF8, 0x0FFF8, 0x07FF0, 0x7FF0, 0x3FE0, 0x0F80,
        0x07C0, 0x1FF0, 0x03FF8, 0x07FFC, 0x07FFC, 0x0FFFE, 0x0FFFE, 0x0FFFE, 0x0FFFE, 0x0FFFE, 0x7FFC, 0x7FFC, 0x3FF8, 0x1FF0, 0x07C0
    };

    static const uint16_t circleDataForVersion3PenSize1[] = {
        0x0000, 0xE000, 0x00000,
    };

    uint16_t        circleWord;
    const uint16_t* circlePointer;
    uint16_t        counter;
    uint16_t        penWidth  = 0;
    int             penFinalX = 0;
    int             penFinalY = 0;

    uint8_t  t = 0;
    uint8_t  temp8;
    uint16_t temp16;

    uint16_t textureNumber = 0;
    int      penX          = x;
    int      penY          = y;
    uint16_t penSize       = _patternCode & 0x07;

    circlePointer = &circleData[circleList[penSize]];

    if (_version3 && penSize == 1)
        circlePointer = circleDataForVersion3PenSize1;

    // setup the X position
    // = penX - pen.size/2

    penX = (penX * 2) - penSize;
    if (penX < 0)
        penX = 0;

    temp16 = (PictureWidth * 2) - (2 * penSize);
    if (penX >= temp16)
        penX = temp16;

    penX     /= 2;
    penFinalX = penX; // original starting point?? -> used in plotrelated

    // Setup the Y Position
    // = penY - pen.size
    penY = penY - penSize;
    if (penY < 0)
        penY = 0;

    temp16 = (PictureHeight - 1) - (2 * penSize);
    if (penY >= temp16)
        penY = temp16;

    penFinalY = penY;    // used in plotrelated

    t = (uint8_t)(textureNumber | 0x01);        // even

    penWidth   = (penSize << 1) + 1; // pen size
    penFinalY += penWidth;           // the last row of this shape
    temp16     = penWidth << 1;

    bool circleCond;
    int  counterStep;
    int  ditherCond;

    circleCond  = ((_patternCode & 0x10) != 0);
    counterStep = 4;
    ditherCond  = 0x01;

    for (; penY < penFinalY; penY++) {
        circleWord = *circlePointer++;

        for (counter = 0; counter <= penWidth; counter += counterStep) {
            if (circleCond || ((binaryList[counter >> 1] & circleWord) != 0)) {
                if ((_patternCode & 0x20) != 0) {
                    temp8 = t % 2;
                    t = t >> 1;
                    if (temp8 != 0)
                        t = t ^ 0xB8;
                }

                // == box plot, != circle plot
                if ((_patternCode & 0x20) == 0 || (t & 0x03) == ditherCond)
                    putPixel(penX, penY);
            }
            penX++;
        }

        penX = penFinalX;
    }
}

void PictureTracer::end() {
}
