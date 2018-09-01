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

import { PictureTracer, PictureTracerDelegate, PictureWidth, PictureHeight } from "./PictureTracer";
import { Info } from "./Info";

export class PictureRasterizer implements PictureTracerDelegate {
    screen   = new Uint8Array(PictureWidth * PictureHeight);
    priority = new Uint8Array(PictureWidth * PictureHeight);

    constructor() {
        this.screen.fill(0x0f);
        this.priority.fill(0x04);
    }

    pixelScreen(x: number, y: number): number {
        return this.screen[(y * PictureWidth) + x];
    }
    
    setPixelScreen(x: number, y: number, color: number): void {
        this.screen[(y * PictureWidth) + x] = color;
    }
    
    pixelPriority(x: number, y: number): number {
        return this.priority[(y * PictureWidth) + x];
    }
    
    setPixelPriority(x: number, y: number, priority: number): void {
        this.priority[(y * PictureWidth) + x] = priority;
    }

    static process(info: Info, buffer: ArrayBuffer): [Uint8Array, Uint8Array] {
        const rasterizer = new PictureRasterizer();
        PictureTracer.process(info, buffer, rasterizer);
        return [rasterizer.screen, rasterizer.priority];
    }

    static splitPriorityAndControlData(data: Uint8Array): [Uint8Array, Uint8Array] {
        const priorityData = new Uint8Array(PictureWidth * PictureHeight);
        const controlData  = new Uint8Array(PictureWidth * PictureHeight);

        for (let y = 0; y < PictureHeight; y++) {
            for (let x = 0; x < PictureWidth; x++) {
                const pixel = data[(y * PictureWidth) + x];

                if (pixel < 4) {
                    let effectivePriority = 4;

                    for (let y2 = y + 1; y2 < PictureHeight; y2++) {
                        const lowerPixel = data[(y2 * PictureWidth) + x];

                        if (lowerPixel >= 4) {
                            effectivePriority = lowerPixel;
                            break;
                        }
                    }

                    controlData [(y * PictureWidth) + x] = pixel;
                    priorityData[(y * PictureWidth) + x] = effectivePriority;
                }
                else {
                    controlData [(y * PictureWidth) + x] = 15;
                    priorityData[(y * PictureWidth) + x] = pixel;
                }
            }
        }

        return [priorityData, controlData];
    }
}
