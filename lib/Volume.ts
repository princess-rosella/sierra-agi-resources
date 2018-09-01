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

import { Info } from "./Info";
import { ResourceType } from "./ResourceType";
import { StorageAccess } from "./StorageAccess";
import { decryptLogic, CRYPT_KEY_SIERRA } from "./VolumeDecrypt";
import { expand, picExpand } from "./VolumeExpander";

class Entry {
    public volume: ResourceType;
    public offset: number;
    public length: number;

    constructor(volume: number, offset: number, length: number) {
        this.volume = volume;
        this.offset = offset;
        this.length = length;
    }
};

type VolumeSizes = { [volume: number]: number };

export abstract class Volume {
    readonly storage: StorageAccess;
    readonly info:    Info;

    private readonly entries = new Map<number, Map<number, Entry>>();

    constructor(storage: StorageAccess, info: Info) {
        this.storage = storage;
        this.info    = info;
        this.parseObjectsAndWordsEntries();
    }

    static for(storage: StorageAccess, info?: Info): Volume {
        if (!info)
            info = Info.detect(storage);

        if (info.engineVersion < 0x3000)
            return new VolumeV2(storage, info);

        return new VolumeV3(storage, info);
    }

    public abstract load(type: ResourceType, id: number): ArrayBuffer;

    public exists(type: ResourceType, id: number): boolean {
        const entriesForType = this.entries.get(type);
        if (!entriesForType)
            return false;

        return entriesForType.has(id);
    }

    protected gatherVolumeSizes(): VolumeSizes {
        const files = this.info.files;
        const results: VolumeSizes = {};

        for (let volume = 0; volume < 10; volume++) {
            const volumeName = files[volume];
            if (!volumeName)
                continue;

            results[volume] = this.storage.fileSize(volumeName);
        }

        return results;
    }

    private parseObjectsAndWordsEntries() {
        const files = this.info.files;
        this.entries.set(ResourceType.Objects, new Map<number, Entry>());
        this.entries.set(ResourceType.Words,   new Map<number, Entry>());

        this.entries.get(ResourceType.Objects)!.set(0, new Entry(ResourceType.Objects, 0, this.storage.fileSize(files[ResourceType.Objects])));
        this.entries.get(ResourceType.Words)!  .set(0, new Entry(ResourceType.Words,   0, this.storage.fileSize(files[ResourceType.Words])));
    }

    protected calculateEntryLengths() {
        const sizes            = this.gatherVolumeSizes()
        const ordered: Entry[] = [];

        for (const [type, entriesForType] of this.entries) {
            if (type === ResourceType.Objects || type === ResourceType.Words)
                continue;

            for (const [, entry] of entriesForType) {
                ordered.push(entry);
            }
        }

        ordered.sort((a, b) => {
            if (a.volume > b.volume)
                return 1;
            else if (a.volume < b.volume)
                return -1;

            if (a.offset > b.offset)
                return 1;
            else if (a.offset < b.offset)
                return -1;
            
            return 0;
        });

        for (let i = 0; i < ordered.length; i++) {
            let entry = ordered[i];
            let next  = ordered[i + 1];

            let nextOffset: number;

            if (!next)
                nextOffset = sizes[entry.volume];
            else if (entry.volume !== next.volume)
                nextOffset = sizes[entry.volume];
            else
                nextOffset = next.offset;

            if (nextOffset >= entry.offset)
                entry.length = nextOffset - entry.offset;
            else
                entry.length = 0;
        }
    }

    protected parseDirectoryEntries(type: ResourceType, buffer: ArrayBuffer) {
        let offset  = 0;
        let length  = buffer.byteLength;
        let view    = new Uint8Array(buffer);
        let entries = new Map<number, Entry>();

        for (let i = 0; length; i++, offset += 3, length -= 3) {
            const entryVolume = view[offset] >> 4;
            const entryOffset = ((view[offset] & 0xf) << 16) | (view[offset + 1] << 8) | view[offset + 2];

            if (entryOffset === 0xfffff)
                // Does not exists!
                continue;

            entries.set(i, new Entry(entryVolume, entryOffset, 0));
        }

        this.entries.set(type, entries);
    }

    protected loadRaw(type: ResourceType, id: number): ArrayBuffer {
        const entriesForType = this.entries.get(type);
        if (!entriesForType)
            throw new Error(`Resource type ${type} don't exists`);

        const entry = entriesForType.get(id);
        if (!entry)
            throw new Error(`Resource of type ${type} with ID ${id} does not exists`);

        const volumeFileName = this.info.files[entry.volume];

        try {
            return this.storage.fileRead(volumeFileName, entry.offset, entry.length);
        }
        catch (e) {
            const error = new Error(`Failed to read resource ${type}/${id}`);
            error["stack"] = `AGIError: ${error.message}\n${e["stack"]}`;
            throw error;
        }
    }
}

export class VolumeV2 extends Volume {
    constructor(storage: StorageAccess, info: Info) {
        super(storage, info);

        this.loadDirectory(ResourceType.Picture);
        this.loadDirectory(ResourceType.View);
        this.loadDirectory(ResourceType.Sound);
        this.loadDirectory(ResourceType.Logic);
        this.calculateEntryLengths();
    }

    private loadDirectory(type: ResourceType) {
        this.parseDirectoryEntries(type, this.storage.fileRead(this.info.files[type]));
    }

    public load(type: ResourceType, id: number): ArrayBuffer {
        const buffer = this.loadRaw(type, id);
        if (buffer.byteLength < 5)
            return new ArrayBuffer(0);

        const view                = new DataView(buffer);
        const uncompressedLength  = view.getUint16(3, true);
        const bufferWithoutHeader = buffer.slice(5, uncompressedLength + 5);

        if (type === ResourceType.Logic)
            decryptLogic(bufferWithoutHeader, CRYPT_KEY_SIERRA);

        return bufferWithoutHeader;
    }
}

export class VolumeV3 extends Volume {
    constructor(storage: StorageAccess, info: Info) {
        super(storage, info);

        const dirData     = this.storage.fileRead(this.info.files[ResourceType.Directory]);
        const dirDataView = new DataView(dirData, 0, 4 * 2);
        const dirOffsets  = [0, 0, 0, 0, 0];

        for (let i = 0; i < 4; i++) {
            dirOffsets[i] = dirDataView.getUint16(i * 2, true);
        }

        dirOffsets[4] = dirData.byteLength;

        this.parseDirectoryEntries(ResourceType.Logic,   dirData.slice(dirOffsets[0], dirOffsets[1]));
        this.parseDirectoryEntries(ResourceType.Picture, dirData.slice(dirOffsets[1], dirOffsets[2]));
        this.parseDirectoryEntries(ResourceType.View,    dirData.slice(dirOffsets[2], dirOffsets[3]));
        this.parseDirectoryEntries(ResourceType.Sound,   dirData.slice(dirOffsets[3], dirOffsets[4]));
        this.calculateEntryLengths();
    }

    public load(type: ResourceType, id: number): ArrayBuffer {
        const buffer              = this.loadRaw(type, id);
        const view                = new DataView(buffer);
        const flags               = view.getUint8(2);
        const uncompressedLength  = view.getUint16(3, true);
        const compressedLength    = view.getUint16(5, true);
        const bufferWithoutHeader = buffer.slice(7, compressedLength + 7);

        if (uncompressedLength === compressedLength) {
            if (type == ResourceType.Logic)
                decryptLogic(bufferWithoutHeader, CRYPT_KEY_SIERRA);

            return bufferWithoutHeader;
        }

        if ((type === ResourceType.Picture) && (flags & 0x80))
            return picExpand(bufferWithoutHeader, uncompressedLength);
        else
            return expand(bufferWithoutHeader, uncompressedLength);
    }
}
