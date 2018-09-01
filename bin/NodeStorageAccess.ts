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

import { StorageAccess } from "../lib";
import * as fs from "fs";
import * as path from "path";

export class NodeStorageAccess implements StorageAccess {
    readonly path: string;

    constructor(path: string) {
        this.path = path;
    }

    fileExists(fileName: string): boolean {
        return fs.existsSync(path.join(this.path, fileName));
    }

    fileSize(fileName: string): number {
        return fs.statSync(path.join(this.path, fileName)).size;
    }

    fileRead(fileName: string, offset: number = 0, length: number = 0): ArrayBuffer {
        const fullPath = path.join(this.path, fileName);

        if (!offset && !length) {
            const nodeBuffer = fs.readFileSync(fullPath);
            return nodeBuffer.buffer.slice(nodeBuffer.byteOffset, nodeBuffer.byteOffset + nodeBuffer.byteLength);
        }

        if (!length) {
            length = fs.statSync(fullPath).size - offset;

            if (length < 0)
                length = 0;
        }

        const fd     = fs.openSync(fullPath, "r");
        const buffer = new Uint8Array(length);

        fs.readSync(fd, buffer, 0, length, offset);
        fs.closeSync(fd);
        return buffer.buffer;
    }

    fileList(): string[] {
        return fs.readdirSync(this.path);
    }
};
