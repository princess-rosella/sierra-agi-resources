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

import { RGB } from "spu-png";

const c55 =  85.0 / 255.0;
const caa = 170.0 / 255.0;

export const EGA16: RGB[] = [
    { r: 0,   g: 0,   b: 0   }, // 0 000000
    { r: 0,   g: 0,   b: caa }, // 1 0000AA
    { r: 0,   g: caa, b: 0   }, // 2 00AA00
    { r: 0,   g: caa, b: caa }, // 3 00AAAA
    { r: caa, g: 0,   b: 0   }, // 4 AA0000
    { r: caa, g: 0,   b: caa }, // 5 AA00AA
    { r: caa, g: c55, b: 0   }, // 6 AA5500
    { r: caa, g: caa, b: caa }, // 7 AAAAAA
    { r: c55, g: c55, b: c55 }, // 8 555555
    { r: c55, g: c55, b: 1   }, // 9 5555FF
    { r: c55, g: 1,   b: c55 }, // a 55FF55
    { r: c55, g: 1,   b: 1   }, // b 55FFFF
    { r: 1,   g: c55, b: c55 }, // c FF5555
    { r: 1,   g: c55, b: 1   }, // d FF55FF
    { r: 1,   g: 1,   b: c55 }, // e FFFF55
    { r: 1,   g: 1,   b: 1   }, // f FFFFFF
];
