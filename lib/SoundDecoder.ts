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

export const enum SoundType {
    Sample       = 1,
    MIDI         = 2,
    FourChannels = 8
}

export interface SoundDecoderDelegate {
    note(channel: number, duration: number, frequency: number, volume: number): void;
    hex?(data: Uint8Array): void;
};

export class SoundDecoder {
    private static processFourChannels(view: DataView, offset: number, delegate: SoundDecoderDelegate): void {
        let channel = 0;

        while (offset < view.byteLength) {
            const duration = view.getUint16(offset, true);

            if (duration === 0xffff) {
                channel++;
                offset += 2;
                continue;
            }

            const freq0 = view.getUint8(offset + 2);
            const freq1 = view.getUint8(offset + 3);
            const vol   = view.getUint8(offset + 4) & 0x0f;
            const freq  = ((freq0 & 0x3f) << 4) | (freq1 & 0x0f);

            if (delegate.hex)
                delegate.hex(new Uint8Array(view.buffer, view.byteOffset + offset, 5));

            delegate.note(channel, duration, freq, (vol === 0x0f)? 0: 0xff - (vol << 1));
            offset += 5;
        }
    }

    static process(buffer: ArrayBuffer, delegate: SoundDecoderDelegate): void {
        if (buffer.byteLength < 2) {
            return;
        }

        const view = new DataView(buffer);
        const type = view.getUint16(0, true);

        switch (type) {
        case SoundType.Sample:
        case SoundType.MIDI:
            throw new Error(`Unsupported sound resource: ${type}`);
        case SoundType.FourChannels:
            this.processFourChannels(view, 8, delegate);
            break;
        }
    }
};
