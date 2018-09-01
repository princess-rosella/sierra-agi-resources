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

type u8  = number;
type i32 = number;
type u32 = number;

const MAXBITS    = 12;
const TABLE_SIZE = 18041;
const START_BITS = 9;

class Expander {
    private BITS:      i32 = 0;
    private MAX_VALUE: i32 = 0;
    private MAX_CODE:  i32 = 0;

    private inputBitCount:   i32         = 0; // Number of bits in input bit buffer
    private inputBitBuffer:  u32         = 0;
    private prefixCode:      Uint32Array = new Uint32Array(TABLE_SIZE);
    private appendCharacter: Uint8Array  = new Uint8Array(TABLE_SIZE);
    private decodeStack:     Uint8Array  = new Uint8Array(8192);

    /**
     * Adjust the number of bits used to store codes to the value passed in.
     */
    private setBits(value: i32): boolean {
        if (value === MAXBITS)
            return true;

        this.BITS      = value;
        this.MAX_VALUE = (1 << this.BITS) - 1;
        this.MAX_CODE  = this.MAX_VALUE - 1;
        return false;
    }

    /**
     * Return the string that the code taken from the input buffer
     * represents. The string is returned as a stack, i.e. the characters are
     * in reverse order.
     */
    private decodeString(buffer: Uint8Array, ptr: number, code: u32): number {
        for (let i = 0; code > 255;) {
            buffer[ptr++] = this.appendCharacter[code];
            code          = this.prefixCode[code];
            if (i++ >= 4000)
                throw new Error("lzw: error in code expansion");
        }
    
        buffer[ptr] = code;
        return ptr;
    }

    /**
     * Uncompress the data contained in the input buffer and store
     * the result in the output buffer. The fileLength parameter says how
     * many bytes to uncompress. The compression itself is a form of LZW that
     * adjusts the number of bits that it represents its codes in as it fills
     * up the available codes. Two codes have special meaning:
     *
     *  code 256 = start over
     *  code 257 = end of data
     */
    public expand(input: Uint8Array, output: Uint8Array): boolean {
        let c:       i32;
        let lzwnext: i32;
        let lzwnew:  i32;
        let lzwold:  i32;

        let s:         number;
        let inputPtr:  number = 0;
        let outputPtr: number = 0;
        let outputEnd: number = output.length;

        /**
         * Return the next code from the input buffer.
         */
        const inputCode = () => {
            while (this.inputBitCount <= 24) {
                this.inputBitBuffer |= input[inputPtr++] << this.inputBitCount;
                this.inputBitCount  += 8;
            }
    
            const r = (this.inputBitBuffer & 0x7fff) % (1 << this.BITS);
            this.inputBitBuffer >>>= this.BITS;
            this.inputBitCount    -= this.BITS;
            return r;
        };

        this.setBits(START_BITS);  // Starts at 9-bits
        lzwnext = 257;             // Next available code to define
        lzwold  = c = inputCode(); // Read in the first code
        lzwnew  = inputCode();
    
        while ((outputPtr < outputEnd) && (lzwnew != 0x101)) {
            if (lzwnew == 0x100) {
                // Code to "start over"
                lzwnext = 258;
                this.setBits(START_BITS);
                lzwold = inputCode();
                lzwnew = inputCode();
                output[outputPtr++] = lzwold & 0xff;
                c = lzwold;
            }
            else {
                if (lzwnew >= lzwnext) {
                    // Handles special LZW scenario
                    this.decodeStack[0] = c;
                    s = this.decodeString(this.decodeStack, 1, lzwold);
                }
                else {
                    s = this.decodeString(this.decodeStack, 0, lzwnew);
                }
    
                // Reverse order of decoded string and store in out buffer
                c = this.decodeStack[s];
                while (s >= 0)
                    output[outputPtr++] = this.decodeStack[s--];
    
                if (lzwnext > this.MAX_CODE)
                    this.setBits(this.BITS + 1);
    
                this.prefixCode     [lzwnext] = lzwold;
                this.appendCharacter[lzwnext] = c;
                lzwnext++;
                lzwold = lzwnew;
                lzwnew = inputCode();
            }
        }
    
        return outputPtr === outputEnd;        
    }
};

export function expand(input: ArrayBuffer, outputSize: number): ArrayBuffer {
    const expander    = new Expander();
    const output      = new ArrayBuffer(outputSize);
    const inputPadded = new Uint8Array(input.byteLength + 32);

    inputPadded.set(new Uint8Array(input));
    expander.expand(new Uint8Array(input), new Uint8Array(output));
    return output;
}

export function picExpand(inputBuffer: ArrayBuffer, outputSize: number): ArrayBuffer {
    const input          = new Uint8Array(inputBuffer);
    const inputLength    = input.byteLength;
    let   inputPtr       = 0;
    let   inputPtrNibble = false;

    function readByte(): u8 {
        if (!inputPtrNibble) {
            return input[inputPtr++];
        }
        else {
            let curByte = (input[inputPtr++] << 4) & 0xff;
            return ((input[inputPtr] >>> 4) | curByte);
        }
    }

    function readNibble(): u8 {
        if (!inputPtrNibble) {
            inputPtrNibble = true;
            return input[inputPtr] >>> 4;
        }
        else {
            inputPtrNibble = false;
            return input[inputPtr++] & 0x0F;
        }
    }

    const output    = new Uint8Array(outputSize);
    let   outputPtr = 0;

    while (inputPtr < inputLength) {
        let curByte = readByte();

        output[outputPtr++] = curByte;

        switch (curByte) {
        case 0xf0:
            output[outputPtr++] = readNibble();
            break;
        case 0xf2:
            output[outputPtr++] = readNibble();
            break;
        case 0xff:
            inputPtr = inputLength;
            break;
        }
    }

    return output.buffer;
}
