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

 export interface PictureDecoderDelegate {
    setColor(color: number): void;
    setColorScreen(screen: boolean): void;
    setPriority(priority: number): void;
    setPriorityScreen(priority: boolean): void;
    drawYCorner(coordinates: Uint8Array): void;
    drawXCorner(coordinates: Uint8Array): void;
    drawLineAbsolute(coordinates: Uint8Array): void;
    drawFill(x: number, y: number): void;
    setPattern(code: number, n: number): void;
    plotPattern(x: number, y: number): void;
    end(): void;

    hex?(data: Uint8Array): void;
};

function convertLineShortToLineAbsolute(input: Uint8Array): Uint8Array {
    const end = input.length;
    if (end < 2)
        return input;

    let disp: number;
    let dx:   number;
    let dy:   number;
    let x1 = input[0];
    let y1 = input[1];

    let it = 2;
    let ot = 2;

    const output = new Uint8Array(2 + ((input.length - 2) * 2));
    output[0] = x1;
    output[1] = y1;

    while (it !== end) {
        disp = input[it++];

        dx = ((disp & 0xf0) >> 4) & 0x0f;
        dy = (disp & 0x0f);

        if (dx & 0x08)
            dx = -(dx & 0x07);
        if (dy & 0x08)
            dy = -(dy & 0x07);

        x1 += dx;
        y1 += dy;
        output[ot++] = x1;
        output[ot++] = y1;
    }
    
    return output;
}

export class PictureDecoder {
    static process(buffer: ArrayBuffer, delegate: PictureDecoderDelegate): void {
        const shallowSlice = function(start: number, end: number): Uint8Array {
            return new Uint8Array(buffer, start, end - start);
        }

        let buffer8       = new Uint8Array(buffer);
        let it            = 0;
        let end           = buffer.byteLength;
        let patternCode   = 0;
        let patternNumber = 0;

        while (it < end) {
            const commandStart = it;
            const command = buffer8[it++];

            switch (command) {
            case 0xf0:
                if (delegate.hex)
                    delegate.hex(shallowSlice(commandStart, it + 1));

                delegate.setColor(buffer8[it++]);
                delegate.setColorScreen(true);
                continue;
            case 0xf1:
                if (delegate.hex)
                    delegate.hex(shallowSlice(commandStart, it));
                
                 delegate.setColorScreen(false);
                continue;
            case 0xf2:
                if (delegate.hex)
                    delegate.hex(shallowSlice(commandStart, it + 1));

                delegate.setPriority(buffer8[it++]);
                delegate.setPriorityScreen(true);
                continue;
            case 0xf3:
                if (delegate.hex)
                    delegate.hex(shallowSlice(commandStart, it));

                delegate.setPriorityScreen(false);
                continue;
            case 0xf9:
                if (delegate.hex)
                    delegate.hex(shallowSlice(commandStart, it + 1));
                
                patternCode = buffer8[it++];
                delegate.setPattern(patternCode, patternNumber);
                continue;
            case 0xfc:
                if (delegate.hex)
                    delegate.hex(shallowSlice(commandStart, it + 2));
                
                delegate.setColor(buffer8[it++]);
                delegate.setPriority(buffer8[it++]);
                break;
            case 0xff:
                if (delegate.hex)
                    delegate.hex(shallowSlice(commandStart, it));

                delegate.end();
                return;
            }

            let start = it;
            while (buffer8[it] < 0xf0 && it < end)
                it++;

            if (delegate.hex)
                delegate.hex(shallowSlice(commandStart, it));

            switch (command) {
            case 0xf4:
                delegate.drawYCorner(shallowSlice(start, it));
                break;
            case 0xf5:
                delegate.drawXCorner(shallowSlice(start, it));
                break;
            case 0xf6:
                delegate.drawLineAbsolute(shallowSlice(start, it));
                break;
            case 0xf7:
                delegate.drawLineAbsolute(convertLineShortToLineAbsolute(shallowSlice(start, it)));
                break;
            case 0xf8:
            case 0xfc:
                for (; (it - start) >= 2; start += 2)
                    delegate.drawFill(buffer8[start], buffer8[start + 1]);
                break;
            case 0xfa:
                while (start != it) {
                    if (patternCode & 0x20) {
                        patternNumber = buffer8[start++];
                        patternNumber >>= 1;
                        delegate.setPattern(patternCode, patternNumber);
                    }
    
                    if ((it - start) < 2)
                        break;
    
                    delegate.plotPattern(buffer8[start], buffer8[start + 1]);
                    start += 2;
                }
                break;
            }
        }
    }
};
