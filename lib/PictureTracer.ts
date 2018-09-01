/*
 * Copyright (c) 2018 Princess Rosella. All rights reserved.
 *
 * @LICENSE_HEADER_START@
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @LICENSE_HEADER_END@
 */

import { Info } from "./Info";
import { PictureDecoderDelegate, PictureDecoder } from "./PictureDecoder";

export const PictureWidth  = 160;
export const PictureHeight = 168;

function clip(value: number, min: number, max: number): number {
    if (value < min)
        return min;
    else if (value > max)
        return max;
    return value;
}

const min = Math.min;
const max = Math.max;

export interface PictureTracerDelegate {
    pixelScreen(x: number, y: number): number;
    setPixelScreen(x: number, y: number, color: number): void;
    pixelPriority(x: number, y: number): number;
    setPixelPriority(x: number, y: number, priority: number): void;
}

interface Point {
    x: number;
    y: number;
};

const PATTERN_BINARY_LIST = new Uint16Array([
    0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100,
    0x0080, 0x0040, 0x0020, 0x0010, 0x008, 0x004, 0x002, 0x001
]);

const PATTERN_CIRCLE_LIST = new Uint8Array([0, 1, 4, 9, 16, 25, 37, 50]);
const PATTERN_CIRCLE_DATA = new Uint16Array([
    0x8000,
    0xE000, 0xE000, 0x0E000,
    0x7000, 0xF800, 0x0F800, 0x0F800, 0x07000,
    0x3800, 0x7C00, 0x0FE00, 0x0FE00, 0x0FE00, 0x07C00, 0x03800,
    0x1C00, 0x7F00, 0x0FF80, 0x0FF80, 0x0FF80, 0x0FF80, 0x0FF80, 0x07F00, 0x01C00,
    0x0E00, 0x3F80, 0x07FC0, 0x07FC0, 0x0FFE0, 0x0FFE0, 0x0FFE0, 0x07FC0, 0x07FC0, 0x03F80, 0x1F00, 0x0E00,
    0x0F80, 0x3FE0, 0x07FF0, 0x07FF0, 0x0FFF8, 0x0FFF8, 0x0FFF8, 0x0FFF8, 0x0FFF8, 0x07FF0, 0x7FF0, 0x3FE0, 0x0F80,
    0x07C0, 0x1FF0, 0x03FF8, 0x07FFC, 0x07FFC, 0x0FFFE, 0x0FFFE, 0x0FFFE, 0x0FFFE, 0x0FFFE, 0x7FFC, 0x7FFC, 0x3FF8, 0x1FF0, 0x07C0
]);

const PATTERN_CIRCLE_DATA_V3_P1 = new Uint16Array([
    0x0000, 0xE000, 0x00000
]);

export class PictureTracer implements PictureDecoderDelegate {
    screen        = false;
    screenColor   = 0;
    priority      = false;
    priorityColor = 0;
    patternCode   = 0;
    patternNumber = 0;

    private readonly version3: boolean;
    private readonly delegate: PictureTracerDelegate;

    constructor(info: Info, delegate: PictureTracerDelegate) {
        this.version3 = info.engineVersion >= 0x3000;
        this.delegate = delegate;
    }

    setColor(color: number): void {
        this.screenColor = color;
    }
    
    setColorScreen(screen: boolean): void {
        this.screen = screen;
    }

    setPriority(priority: number): void {
        this.priorityColor = priority;
    }

    setPriorityScreen(priority: boolean): void {
        this.priority = priority;
    }

    private putPixel(x: number, y: number): void {
        x &= 0xff;
        y &= 0xff;
        if (x >= PictureWidth || y >= PictureHeight)
            return;

        if (this.screen)
            this.delegate.setPixelScreen(x, y, this.screenColor);

        if (this.priority)
            this.delegate.setPixelPriority(x, y, this.priorityColor);
    }

    private drawLine(x1: number, y1: number, x2: number, y2: number) {
        x1 = clip(x1, 0, PictureWidth  - 1);
        x2 = clip(x2, 0, PictureWidth  - 1);
        y1 = clip(y1, 0, PictureHeight - 1);
        y2 = clip(y2, 0, PictureHeight - 1);

        // Vertical line
        if (x1 === x2) {
            let   ymin = min(y1, y2);
            const ymax = max(y1, y2);

            for (; ymin <= ymax; ymin++)
                this.putPixel(x1, ymin);

            return;
        }

        // Horizontal line
        if (y1 === y2) {
            let   xmin = min(x1, x2);
            const xmax = max(x1, x2);

            for (; xmin <= xmax; xmin++)
                this.putPixel(xmin, y1);

            return;
        }

        let y = y1;
        let x = x1;
    
        let stepY  = 1;
        let deltaY = y2 - y1;
        if (deltaY < 0) {
            stepY  = -1;
            deltaY = -deltaY;
        }
    
        let stepX  = 1;
        let deltaX = x2 - x1;
        if (deltaX < 0) {
            stepX  = -1;
            deltaX = -deltaX;
        }
    
        let errorX:   number;
        let errorY:   number;
        let i:        number;
        let detdelta: number;

        if (deltaY > deltaX) {
            i        = deltaY;
            detdelta = deltaY;
            errorX   = (deltaY / 2) | 0;
            errorY   = 0;
        }
        else {
            i        = deltaX;
            detdelta = deltaX;
            errorX   = 0;
            errorY   = (deltaX / 2) | 0;
        }
    
        this.putPixel(x, y);
    
        do {
            errorY += deltaY;
            if (errorY >= detdelta) {
                errorY -= detdelta;
                y      += stepY;
            }
    
            errorX += deltaX;
            if (errorX >= detdelta) {
                errorX -= detdelta;
                x      += stepX;
            }
    
            this.putPixel(x, y);
            i--;
        } while (i > 0);
    }

    drawYCorner(coordinates: Uint8Array): void {
        const end = coordinates.length;
        if (end < 2)
            return;

        let x2: number, y2: number;
        let x1 = coordinates[0];
        let y1 = coordinates[1];
        this.putPixel(x1, y1);

        let it = 2;

        while (it !== end) {
            y2 = coordinates[it++];
            this.drawLine(x1, y1, x1, y2);

            if (it === end)
                break;

            y1 = y2;
            x2 = coordinates[it++];
            this.drawLine(x1, y1, x2, y1);
            x1 = x2;
        }
    }

    drawXCorner(coordinates: Uint8Array): void {
        const end = coordinates.length;
        if (end < 2)
            return;

        let x2: number, y2: number;
        let x1 = coordinates[0];
        let y1 = coordinates[1];
        this.putPixel(x1, y1);
    
        let it = 2;

        while (it !== end) {
            x2 = coordinates[it++];
            this.drawLine(x1, y1, x2, y1);

            if (it === end)
                break;

            x1 = x2;
            y2 = coordinates[it++];
            this.drawLine(x1, y1, x1, y2);
            y1 = y2;
        }
    }

    drawLineAbsolute(coordinates: Uint8Array): void {
        const end = coordinates.length;
        if (end < 2)
            return;

        let x2: number;
        let y2: number;
        let x1 = coordinates[0];
        let y1 = coordinates[1];
        this.putPixel(x1, y1);

        let it = 2;

        while (it !== end) {
            x2 = coordinates[it++];

            if (it === end)
                break;

            y2 = coordinates[it++];
            this.drawLine(x1, y1, x2, y2);
            x1 = x2;
            y1 = y2;
        }
    }

    drawLineShort(coordinates: Uint8Array): void {
        const end = coordinates.length;
        if (end < 2)
            return;

        let disp: number;
        let dx:   number;
        let dy:   number;
        let x1 = coordinates[0];
        let y1 = coordinates[1];
        this.putPixel(x1, y1);
    
        let it = 2;
    
        while (it !== end) {
            disp = coordinates[it++];
    
            dx = ((disp & 0xf0) >> 4) & 0x0f;
            dy = (disp & 0x0f);
    
            if (dx & 0x08)
                dx = -(dx & 0x07);
            if (dy & 0x08)
                dy = -(dy & 0x07);
    
            this.drawLine(x1, y1, x1 + dx, y1 + dy);
            x1 += dx;
            y1 += dy;
        }
    }

    private drawFillCheck(x: number, y: number): boolean {
        if (x >= PictureWidth || y >= PictureHeight)
            return false;
    
        let screenColor    = this.delegate.pixelScreen(x, y);
        let screenPriority = this.delegate.pixelPriority(x, y);
    
        if (!this.priority && this.screen && this.screenColor != 15)
            return (screenColor == 15);
    
        if (this.priority && !this.screen && this.priorityColor != 4)
            return screenPriority == 4;
    
        return (this.screen && screenColor == 15 && this.screenColor != 15);
    }

    drawFill(x: number, y: number): void {
        if (!this.screen && !this.priority)
            return;

        const stack: Point[] = [];

        stack.push({x: x, y: y});

        while (stack.length) {
            const p = stack.shift()!;

            if (!this.drawFillCheck(p.x, p.y))
                continue;

            let c: number;

            for (c = p.x - 1; this.drawFillCheck(c, p.y); c--)
                ;

            let newspanUp:   boolean = true;
            let newspanDown: boolean = true;

            for (c++; this.drawFillCheck(c, p.y); c++) {
                this.putPixel(c, p.y);
                if (this.drawFillCheck(c, p.y - 1)) {
                    if (newspanUp) {
                        stack.push({x: c, y: p.y - 1});
                        newspanUp = false;
                    }
                }
                else {
                    newspanUp = true;
                }
    
                if (this.drawFillCheck(c, p.y + 1)) {
                    if (newspanDown) {
                        stack.push({x: c, y: p.y + 1});
                        newspanDown = false;
                    }
                }
                else {
                    newspanDown = true;
                }
            }
        }
    }

    setPattern(code: number, n: number): void {
        this.patternCode   = code;
        this.patternNumber = n;
    }

    plotPattern(x: number, y: number): void {
        let   penFinalX = 0;
        let   penFinalY = 0;
        const penSize   = this.patternCode & 0x7;

        let circleData    = PATTERN_CIRCLE_DATA;
        let circlePointer = PATTERN_CIRCLE_LIST[penSize];

        if (this.version3 && penSize === 1) {
            circleData    = PATTERN_CIRCLE_DATA_V3_P1;
            circlePointer = 0;
        }

        // setup the X position
        // = penX - pen.size/2

        let penX = (x * 2) - penSize;
        if (penX < 0)
            penX = 0;

        let temp16 = (PictureWidth * 2) - (2 * penSize);
        if (penX >= temp16)
            penX = temp16;

        penX      = (penX / 2) | 0;
        penFinalX = penX; // original starting point?? -> used in plotrelated

        // Setup the Y Position
        // = penY - pen.size
        let penY = y - penSize;
        if (penY < 0)
            penY = 0;

        temp16 = (PictureHeight - 1) - (2 * penSize);
        if (penY >= temp16)
            penY = temp16;

        penFinalY = penY;    // used in plotrelated

        const penWidth = (penSize << 1) + 1; // pen size

        penFinalY += penWidth;           // the last row of this shape

        const circleCond    = ((this.patternCode & 0x10) != 0);
        const counterStep   = 4;
        const ditherCond    = 0x01;
        let   textureNumber = 0x01;

        for (; penY < penFinalY; penY++) {
            const circleWord = circleData[circlePointer++];

            for (let counter = 0; counter <= penWidth; counter += counterStep) {
                if (circleCond || ((PATTERN_BINARY_LIST[counter >> 1] & circleWord) != 0)) {
                    if ((this.patternCode & 0x20) != 0) {
                        const temp8 = textureNumber % 2;
                        textureNumber >>>= 1;
                        if (temp8 != 0)
                            textureNumber = textureNumber ^ 0xB8;
                    }

                    // == box plot, != circle plot
                    if ((this.patternCode & 0x20) == 0 || (textureNumber & 0x03) == ditherCond)
                        this.putPixel(penX, penY);
                }
                penX++;
            }

            penX = penFinalX;
        }
    }

    end(): void {
    }

    static process(info: Info, buffer: ArrayBuffer, delegate: PictureTracerDelegate): void {
        PictureDecoder.process(buffer, new PictureTracer(info, delegate));
    }
}
