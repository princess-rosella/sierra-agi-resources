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

import { PictureDecoderDelegate } from "./PictureDecoder";
import { HexFormatter } from "spu-hex-dump";

export class PicturePrinter implements PictureDecoderDelegate {
    output: string[] = [];
    formatter = new HexFormatter();

    constructor() {
        this.formatter.aligned = false;
    }

    setColor(color: number): void {
        this.output.push(`ctx.setColor(${color});`);
    }
    
    setColorScreen(screen: boolean): void {
        this.output.push(`ctx.setColorScreen(${screen});`);
    }
    
    setPriority(priority: number): void {
        this.output.push(`ctx.setPriority(${priority});`);
    }
    
    setPriorityScreen(priority: boolean): void {
        this.output.push(`ctx.setPriorityScreen(${priority});`);
    }
    
    drawYCorner(coordinates: Uint8Array): void {
        this.output.push(`ctx.drawYCorner([${coordinates}]);`);
    }
    
    drawXCorner(coordinates: Uint8Array): void {
        this.output.push(`ctx.drawXCorner([${coordinates}]);`);
    }
    
    drawLineAbsolute(coordinates: Uint8Array): void {
        this.output.push(`ctx.drawLineAbsolute([${coordinates}]);`);
    }
    
    drawLineShort(coordinates: Uint8Array): void {
        this.output.push(`ctx.drawLineShort([${coordinates}]);`);
    }
    
    drawFill(x: number, y: number): void {
        this.output.push(`ctx.drawFill(${x}, ${y});`);
    }
    
    setPattern(code: number, n: number): void {
        this.output.push(`ctx.setPattern(${code}, ${n});`);
    }
    
    plotPattern(x: number, y: number): void {
        this.output.push(`ctx.plotPattern(${x}, ${y});`);
    }
    
    end(): void {
        this.output.push(`ctx.end();`);
    }

    hex(data: Uint8Array) {
        const lines = this.formatter.format(data).map(function(s) { return "// " + s });
        this.output.push(...lines);
    }

    toString(): string {
        return `function draw(ctx) {
  ${this.output.join("\n  ")}
}`;
    }
}
