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

import { HexFormatter } from "spu-hex-dump";
import { SoundDecoderDelegate } from "./SoundDecoder";
import { SoundMixerDelegate } from "./SoundMixer";
import { SoundSequencerDelegate } from "./SoundSequencer";
import { openSync, writeSync, closeSync } from "fs";

export class SoundPrinter implements SoundDecoderDelegate {
    output: string[] = [];
    formatter = new HexFormatter();

    constructor() {
        this.formatter.aligned = false;
        this.formatter.bytesPerLine = 5;
    }

    note(channel: number, duration: number, frequency: number, volume: number): void {
        this.output.push(`ctx.play(${channel}, ${duration}, ${frequency}, ${volume});`);
    }
    
    hex(data: Uint8Array) {
        const lines = this.formatter.format(data).map(function(s) { return "// " + s });
        this.output.push(...lines);
    }

    toString(): string {
        return `function decode(ctx) {
  ${this.output.join("\n  ")}
}`;
    }
};

export class SoundSequencePrinter implements SoundSequencerDelegate {
    formatter = new HexFormatter();
    output:    string[] = [];
    frequency: number[] = [];
    volume:    number[] = [];

    constructor() {
        this.formatter.aligned = false;
    }

    note(channel: number, frequency: number, volume: number): void {
        this.frequency[channel] = frequency;
        this.volume[channel]    = volume;
    }
    
    end(channel: number): void {
        this.frequency[channel] = 0;
        this.volume[channel]    = 0;
    }

    flush(): void {
        const notes: number[] = [];
        let   index = 0;

        for (const frequency of this.frequency)
            notes.push(frequency, this.volume[index++]);

        this.output.push(`ctx.play(${notes.join(", ")});`);
    }

    toString(): string {
        return `function sequence(ctx) {
  ${this.output.join("\n  ")}
}`;
    }
};

export class SoundWaveEncoder implements SoundMixerDelegate {
    private fd: number;
    private length: number = 0;

    constructor(fileName: string) {
        this.fd = openSync(fileName, "w", 0o0644);
    }

    sample(buffer: Int16Array): void {
        if (this.length === 0) {
            const dataView = new DataView(new ArrayBuffer(44 + buffer.byteLength));

            dataView.setUint32(0x00, 0x52494646, false);  // 'RIFF'
            dataView.setUint32(0x04, 36 + buffer.byteLength, true);
            dataView.setUint32(0x08, 0x57415645, false);  // 'WAVE'
            dataView.setUint32(0x0c, 0x666d7420, false);  // 'fmt '
            dataView.setUint32(0x10, 16, true);  // Format length
            dataView.setUint16(0x14, 1, true);   // PCM
            dataView.setUint16(0x16, 1, true);   // Mono
            dataView.setUint32(0x18, 22050, true); // Sample Rate
            dataView.setUint32(0x1c, 44100, true); // Bytes Rate (SampleRate * NumChannel * Bits/8)
            dataView.setUint16(0x20, 2, true);   // Bytes per Sample (NumChannel * Bits/8)
            dataView.setUint16(0x22, 16, true);  // 8 bits
            dataView.setUint32(0x24, 0x64617461, false); // 'data'
            dataView.setUint32(0x28, buffer.byteLength, true);

            const dest = new Uint16Array(dataView.buffer, 44, buffer.length);
            dest.set(buffer);

            writeSync(this.fd, dataView);
        }
        else {
            writeSync(this.fd, buffer);
        }

        this.length += buffer.byteLength;
    }

    close() {
        const dataView = new DataView(new ArrayBuffer(4));

        dataView.setUint32(0, 36 + this.length, true);
        writeSync(this.fd, dataView, 0, 4, 0x04);
        dataView.setUint32(0, this.length, true);
        writeSync(this.fd, dataView, 0, 4, 0x28);

        closeSync(this.fd);
        this.fd = -1;
    }
}
