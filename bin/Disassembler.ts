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

import { NodeStorageAccess } from "./NodeStorageAccess";
import * as fs from "fs";
import * as path from "path";

import { Volume, Info, Platform, ResourceType, PictureDecoder, PictureRasterizer, PictureWidth, PictureHeight, SoundDecoder, SoundSequencer } from "../lib";

import { PicturePrinter } from "../lib/PicturePrinter";
import { PNGFile, ChunkHeader, ColorType, ChunkPalette, PNGImageIndexed, ChunkPhysicalPixels, PNGUnit, ChunkText } from "spu-png";
import { EGA16 } from "../lib/EGA16";

import { SoundPrinter, SoundSequencePrinter, SoundWaveEncoder } from "../lib/SoundPrinter";
import { SoundMixer, WaveformType } from "../lib/SoundMixer";

function writeInfo(outDir: string, info: Info) {
    function platformToString(platform: Platform): string {
        switch (platform) {
        case Platform.PCXT:
            return "PC XT";
        }
    
        return `Platform ${platform}`;
    }
    
    function engineVersionToString(n: number): string {
        return `${(n >>> 12) & 0xf}.${(n >>> 8) & 0xf}.${(n & 0xff).toString(16)}`;
    }

    let infoText = "";

    infoText += `Code: ${info.code}\n`;
    infoText += `Description: ${info.description}\n`;
    infoText += `Version: ${info.gameVersion}\n`;
    infoText += `Platform: ${platformToString(info.platform)}\n`;
    infoText += `Engine Version: ${engineVersionToString(info.engineVersion)}\n`;

    fs.writeFile(path.join(outDir, "info.txt"), infoText, function(){});
}

function writeEGAPicture(fileName: string, title: string, data: Uint8Array) {
    const file  = PNGFile.create(new ChunkHeader(PictureWidth, PictureHeight, 4, ColorType.Palette), new ChunkPalette(EGA16));
    const image = <PNGImageIndexed>file.renderImage();

    for (let y = 0; y < PictureHeight; y++) {
        for (let x = 0; x < PictureWidth; x++) {
            image.setIndex(x, y, data[(y * PictureWidth) + x]);
        }
    }

    /**
     * Most modern screen today, since the VGA standard, only have square pixels. But. In
     * ancient times, EGA pixels were not square, their ratio is 1:1.2. It was essentially
     * made liek this to make the characters on screen encodable on 8 bytes (8x8) while
     * trying to match the aspect ratio expected by humans. And they felt like 10x8.
     * 
     * EGA Screens are 12 inches wide (0.3048 meters), 9 inches tall (0.2286 meters), with a 15 inches diagnonal (0.381 meters).
     * EGA Screens (as used by AGI) are rendered at 320 pixels wide, 200 pixels tall.
     * 
     * That made 97.526 pixels per meter wide and 45.72 pixels per meter tall.
     * That made 1049.8688 pixels per meter wide and 874.89 pixels per meter tall.
     * 
     * But to add to the complexity. AGI, to safe space, stored pictures as 160x168.
     * 
     * The bottom 32 pixels where left black for the command prompt. But every pixels
     * of the pictures were doubled on the X-axis.
     */

    file.chunks.splice(1, 0,
        new ChunkPhysicalPixels(525, 875, PNGUnit.Meter),
        new ChunkText("tEXt", "Title", title));
    file.updateImage(image);

    fs.writeFileSync(fileName, Buffer.from(file.toArrayBuffer()));
}

function writePictures(outDir: string, volume: Volume) {
    const picDir = path.join(outDir, "pictures");
    if (!fs.existsSync(picDir))
        fs.mkdirSync(picDir);

    const gameDesc = volume.info.description;

    for (let i = 0; i < 256; i++) {
        if (!volume.exists(ResourceType.Picture, i))
            continue;
        
        const raw     = volume.load(ResourceType.Picture, i);
        const printer = new PicturePrinter();

        PictureDecoder.process(raw, printer);
        fs.writeFileSync(path.join(picDir, `${i}.bin`), Buffer.from(raw));
        fs.writeFileSync(path.join(picDir, `${i}.js`), printer.toString());

        const [screenData, priorityAndControlData] = PictureRasterizer.process(volume.info, raw);
        const [priorityData, controlData] = PictureRasterizer.splitPriorityAndControlData(priorityAndControlData);

        writeEGAPicture(path.join(picDir, `${i}.png`),                      `${gameDesc}: Picture ${i}: Visual`, screenData);
        writeEGAPicture(path.join(picDir, `${i} Priority and Control.png`), `${gameDesc}: Picture ${i}: Priority and Control`, priorityAndControlData);
        writeEGAPicture(path.join(picDir, `${i} Priority.png`),             `${gameDesc}: Picture ${i}: Priority`, priorityData);
        writeEGAPicture(path.join(picDir, `${i} Control.png`),              `${gameDesc}: Picture ${i}: Control`, controlData);
    }
}

function writeSounds(outDir: string, volume: Volume) {
    const soundDir = path.join(outDir, "sounds");
    if (!fs.existsSync(soundDir))
        fs.mkdirSync(soundDir);

    for (let i = 0; i < 256; i++) {
        if (!volume.exists(ResourceType.Sound, i))
            continue;

        try {
            const raw = volume.load(ResourceType.Sound, i);
            if (raw.byteLength < 2)
                continue;

            {
                const printer = new SoundPrinter();
                SoundDecoder.process(raw, printer);
                fs.writeFileSync(path.join(soundDir, `${i}.bin`), Buffer.from(raw));
                fs.writeFileSync(path.join(soundDir, `${i}.js`), printer.toString());
            }

            {
                const sequencePrinter = new SoundSequencePrinter();
                const sequencer = SoundSequencer.create(volume.info, raw, sequencePrinter);

                while (sequencer.next()) {
                }

                fs.writeFileSync(path.join(soundDir, `${i}.sequence.js`), sequencePrinter.toString());
            }

            {
                const waveEncoder = new SoundWaveEncoder(path.join(soundDir, `${i}.wav`));
                const mixer       = new SoundMixer(WaveformType.Ramp, waveEncoder);
                const sequencer   = SoundSequencer.create(volume.info, raw, mixer);

                while (sequencer.next()) {
                }

                waveEncoder.close();
            }
        }
        catch (e) {
            console.warn(`Failed to load/process sound ${i}:\n${e["stack"] || e}`)
        }
    }
}

function main(args: string[]): number {
    for (const arg of args) {
        const storageAccess = new NodeStorageAccess(arg);
        const outDir = arg + ".disasm";

        if (!fs.existsSync(outDir))
            fs.mkdirSync(outDir);

        const volume = Volume.for(storageAccess);
        if (!volume) {
            console.error(`Can't instanciate volume for ${arg}`);
            continue;
        }

        writeInfo(outDir, volume.info);
        writePictures(outDir, volume);
        writeSounds(outDir, volume);
    }

    return 0;
}

process.exitCode = main(process.argv.slice(2));
