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

import { SoundDecoder } from "./SoundDecoder";
import { Info } from "./Info";

export interface SoundSequencerDelegate {
    note(channel: number, frequency: number, volume: number): void;
    end(channel: number): void;
    flush(): void;
}

interface SoundNote {
    duration:  number;
    frequency: number;
    volume:    number;
}

export class SoundSequencer {
    private readonly channels:  SoundNote[][];
    private readonly delegate:  SoundSequencerDelegate;
    private readonly positions: number[] = [];
    private readonly durations: number[] = [];

    private _position: number = -1;

    length: number = 0;

    constructor(info: Info, channels: SoundNote[][], delegate: SoundSequencerDelegate) {
        this.channels = channels;
        this.delegate = delegate;

        for (const channel of channels) {
            let length = 0;

            for (const note of channel)
                length += note.duration;

            if (this.length < length)
                this.length = length;

            this.positions.push(-1);
            this.durations.push(0);
        }
    }

    get channelCount(): number {
        return this.channels.length;
    }

    get position(): number {
        return this.position;
    }

    set position(value: number) {
        let index = 0;

        for (const _ of this.channels) {
            this.positions[index] = -1;
            this.durations[index] = 1;
            index++;
        }

        this._next(value, true);
    }

    next(): boolean {
        return this._next(1, true);
    }

    private _next(count: number, notifyDelegate: boolean): boolean {
        let   channelIndex = 0;
        let   position     = this._position;
        const positions    = this.positions;
        const durations    = this.durations;
        const delegate     = this.delegate;

        for (let index = 0; index < count; index++) {
            channelIndex   = 0;
            this._position = ++position;

            for (const channel of this.channels) {
                const duration = --(durations[channelIndex]);

                if (duration <= 0) {
                    const note = channel[++(positions[channelIndex])];

                    if (note) {
                        durations[channelIndex] = note.duration;
                        delegate.note(channelIndex, note.frequency, note.volume);
                    }
                    else {
                        durations[channelIndex] = 0x7fffffff;
                        delegate.end(channelIndex);
                    }
                }

                channelIndex++;
            }
        }

        delegate.flush();
        return position < this.length;
    }

    static create(info: Info, buffer: ArrayBuffer, delegate: SoundSequencerDelegate): SoundSequencer {
        const channels: SoundNote[][] = [];

        SoundDecoder.process(buffer, {
            note: (channel: number, duration: number, frequency: number, volume: number) => {
                while (channel >= channels.length)
                    channels.push([]);

                channels[channel].push({ duration: duration, frequency: frequency, volume: volume });
            }
        });

        return new SoundSequencer(info, channels, delegate);
    }
}
