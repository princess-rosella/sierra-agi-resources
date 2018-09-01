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

import { SoundSequencerDelegate } from "./SoundSequencer";

const WAVEFORM_RAMP = new Int16Array([
    0,    8,   16,   24,   32,   40,   48,   56,
   64,   72,   80,   88,   96,  104,  112,  120,
  128,  136,  144,  152,  160,  168,  176,  184,
  192,  200,  208,  216,  224,  232,  240,  255,
    0, -248, -240, -232, -224, -216, -208, -200,
 -192, -184, -176, -168, -160, -152, -144, -136,
 -128, -120, -112, -104,  -96,  -88,  -80,  -72,
  -64,  -56,  -48,  -40,  -32,  -24,  -16,   -8
]);

const WAVEFORM_SQUARE = new Int16Array([
  255,  230,  220,  220,  220,  220,  220,  220,
  220,  220,  220,  220,  220,  220,  220,  220,
  220,  220,  220,  220,  220,  220,  220,  220,
  220,  220,  220,  220,  220,  220,  220,  110,
 -255, -230, -220, -220, -220, -220, -220, -220,
 -220, -220, -220, -220, -220, -220, -220, -220,
 -220, -220, -220, -220, -220, -220, -220, -220,
 -220, -220, -220, -110,    0,    0,    0,    0
]);

const WAVEFORM_MAC = new Int16Array([
   45,  110,  135,  161,  167,  173,  175,  176,
  156,  137,  123,  110,   91,   72,   35,   -2,
  -60, -118, -142, -165, -170, -176, -177, -179,
 -177, -176, -164, -152, -117,  -82,  -17,   47,
   92,  137,  151,  166,  170,  173,  171,  169,
  151,  133,  116,  100,   72,   43,   -7,  -57,
  -99, -141, -156, -170, -174, -177, -178, -179,
 -175, -172, -165, -159, -137, -114,  -67,  -19
]);

const ENV_DECAY   = 1000;
const ENV_SUSTAIN = 100;
const ENV_RELEASE = 7500;

export const enum WaveformType {
    Ramp,
    Square,
    Mac,
}

function waveformTypeToArray(type: WaveformType): Int16Array {
    switch (type) {
    case WaveformType.Ramp:   return WAVEFORM_RAMP;
    case WaveformType.Square: return WAVEFORM_SQUARE;
    case WaveformType.Mac:    return WAVEFORM_MAC;
    }

    throw new Error("Invalid waveform type");
}

const enum NoteMixerChannelFlags {
	Attack  = 3,
	Decay   = 2,
	Sustain = 1,
	Release = 0
}

class NoteMixerChannel {
    frequency: number                = 0;
    volume:    number                = 0;
    phase:     number                = 0;
    env:       number                = 0;
    envFlags:  NoteMixerChannelFlags = NoteMixerChannelFlags.Attack;
    end:       boolean               = false;

    constructor() {
    }

    note(frequency: number, volume: number): void {
        if (frequency > 0) {
            this.frequency = frequency;
            this.volume    = volume;
            this.phase     = 0;
            this.env       = 0x10000;
            this.envFlags  = NoteMixerChannelFlags.Attack;
        }
        else {
            this.envFlags = NoteMixerChannelFlags.Release;
        }
    }

    noteEnd(): void {
        this.frequency = 0;
        this.volume    = 0;
    }
}

export interface SoundMixerDelegate {
    sample(buffer: Int16Array): void;
}

export class SoundMixer implements SoundSequencerDelegate {
    private buffer:   Int16Array         = new Int16Array(410);
    private channels: NoteMixerChannel[] = [ new NoteMixerChannel(), new NoteMixerChannel(), new NoteMixerChannel(), new NoteMixerChannel() ];
    private waveform: Int16Array;
    private delegate: SoundMixerDelegate;

    constructor(waveformType: WaveformType, delegate: SoundMixerDelegate) {
        this.waveform = waveformTypeToArray(waveformType);
        this.delegate = delegate;
    }

    note(channel: number, frequency: number, volume: number): void {
        this.channels[channel].note(frequency, volume);
    }

    end(channel: number): void {
        this.channels[channel].noteEnd();
    }

    flush(): void {
        const buffer         = this.buffer;
        const waveform       = this.waveform;
        const waveformLength = waveform.length;

        buffer.fill(0);

        let b: number;
        let phase: number;

        for (const channel of this.channels) {
            const volume = channel.volume * channel.env >>> 16;
            if (volume <= 0)
                continue;

            phase = channel.phase;

            for (let j = 0; j < 410; j++) {
                b  = waveform[phase >>> 8];
                b += ((waveform[((phase >>> 8) + 1) % waveformLength] - waveform[phase >> 8]) * (phase & 0xff)) >> 8;

                buffer[j] += (b * volume) >> 4;
                phase     += (11860 * 4 / channel.frequency)|0;
                phase     %= waveformLength << 8;
            }

            switch (channel.envFlags) {
            case NoteMixerChannelFlags.Attack:
                channel.envFlags = NoteMixerChannelFlags.Decay;
                break;
            case NoteMixerChannelFlags.Decay:
                if (channel.env  > (channel.volume * ENV_SUSTAIN + ENV_DECAY)) {
                    channel.env -= ENV_DECAY;
                }
                else {
                    channel.env      = channel.volume * ENV_SUSTAIN;
                    channel.envFlags = NoteMixerChannelFlags.Sustain;
                }
                break;
            case NoteMixerChannelFlags.Sustain:
                break;
            case NoteMixerChannelFlags.Release:
                if (channel.env >= ENV_RELEASE)
                    channel.env -= ENV_RELEASE;
                else
                    channel.env = 0;
                break;
            }
            
            channel.phase = phase;
        }

        this.delegate.sample(buffer);
    }
}
