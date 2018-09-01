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

import { MD5 } from "spu-md5";
import { ResourceType } from "./ResourceType";
import { StorageAccess } from "./StorageAccess";

export const enum Platform {
    PCXT
}

type FileMap = { [key: number]: string };

function buildV2(storage: StorageAccess): FileMap {
    const map: FileMap = {};

    const constantNames: { [fileName: string]: ResourceType } = {
        "logdir":    ResourceType.Logic,
        "picdir":    ResourceType.Picture,
        "snddir":    ResourceType.Sound,
        "viewdir":   ResourceType.View,
        "words.tok": ResourceType.Words,
        "object":    ResourceType.Objects,
    }

    for (let fileName of storage.fileList()) {
        fileName = fileName.toLowerCase();

        if (constantNames[fileName]) {
            map[constantNames[fileName]] = fileName;
            continue;
        }

        const results = /^vol.(\d)$/.exec(fileName);
        if (!results || !results.length)
            continue;

        const type = <ResourceType>(ResourceType.Volume_0 + parseInt(results[1]));
        map[type] = fileName;
    }

    return map;
}

function buildV3(storage: StorageAccess): FileMap {
    const map: FileMap = {};

    const constantNames: { [fileName: string]: ResourceType } = {
        "words.tok": ResourceType.Words,
        "object":    ResourceType.Objects,
    }

    const volumeMatch = /^(\w+)vol.(\d)$/;
    const dirMatch    = /^(\w+)dir$/;

    for (let fileName of storage.fileList()) {
        fileName = fileName.toLowerCase();

        if (constantNames[fileName]) {
            map[constantNames[fileName]] = fileName;
            continue;
        }

        let results = volumeMatch.exec(fileName);
        if (results && results.length) {
            const type = <ResourceType>(ResourceType.Volume_0 + parseInt(results[2]));
            map[type] = fileName;
            continue;
        }

        results = dirMatch.exec(fileName);
        if (results && results.length) {
            map[ResourceType.Directory] = fileName;
            continue;
        }
    }

    return map;
}

type InfoConstruct = (map: FileMap) => Info;

const hashMap: { [hash: string]: InfoConstruct } = {
    "10ad66e2ecbd66951534a50aedcd0128": (map) => new Info("kq1", "King's Quest I",   0x2917, "2.0F 1987-05-05 5.25",   Platform.PCXT, 0, map),
    "759e39f891a0e1d86dd29d7de485c6ac": (map) => new Info("kq2", "King's Quest II",  0x2440, "2.1 1987-04-10",         Platform.PCXT, 0, map),
    "d3d17b77b3b3cd13246749231d9473cd": (map) => new Info("kq3", "King's Quest III", 0x2936, "2.14 1988-03-15 3.5",    Platform.PCXT, 0, map),
    "fe44655c42f16c6f81046fdf169b6337": (map) => new Info("kq4", "King's Quest IV",  0x3086, "2.0 1988-07-27 3.5",     Platform.PCXT, 0, map),
}

function detectV2(storage: StorageAccess): Info {
    const map           = buildV2(storage);
    const logicFilename = map[ResourceType.Logic];
    if (!logicFilename)
        throw new Error("Unable to find logic resource directory");

    const logicHash = MD5.process(storage.fileRead(logicFilename));

    if (hashMap[logicHash])
        return hashMap[logicHash](map);

    return new Info("agiv2", `Unknown AGI v2 game ${logicHash}`, 0x2917, "0.0", Platform.PCXT, 0, map)
}

function detectV3(storage: StorageAccess): Info {
    const map         = buildV3(storage);
    const dirFilename = map[ResourceType.Directory];
    if (!dirFilename)
        throw new Error("Unable to find resource directory");

    const dirHash = MD5.process(storage.fileRead(dirFilename));

    if (hashMap[dirHash])
        return hashMap[dirHash](map);

    return new Info("agiv3", `Unknown AGI v3 game ${dirHash}`, 0x3086, "0.0", Platform.PCXT, 0, map)
}

export class Info {
    readonly code:          string;
    readonly description:   string;
    readonly engineVersion: number;
    readonly gameVersion:   string;
    readonly platform:      Platform;
    readonly flags:         number;
    readonly files:         { [key: number]: string };

    constructor(code: string, description: string, engineVersion: number, gameVersion: string, platform: Platform, flags: number, files: FileMap) {
        this.code          = code;
        this.description   = description;
        this.engineVersion = engineVersion;
        this.gameVersion   = gameVersion;
        this.platform      = platform;
        this.flags         = flags;
        this.files         = files;
    }

    static detect(storage: StorageAccess): Info {
        if (storage.fileExists("logdir") &&
            storage.fileExists("object") && 
            storage.fileExists("picdir") &&
            storage.fileExists("snddir") &&
            storage.fileExists("viewdir") &&
            storage.fileExists("words.tok") &&
            storage.fileExists("vol.0")) {
                return detectV2(storage);
        }

        return detectV3(storage);
    }
};
