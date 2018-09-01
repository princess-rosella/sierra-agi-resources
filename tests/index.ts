
import { ResourceType, StorageAccess, Volume } from "../lib/index"
import * as fs from "fs";
import * as path from "path";

class NodeStorageAccess implements StorageAccess {
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

        if (!length)
            length = fs.statSync(fullPath).size - offset;

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

Volume.for(new NodeStorageAccess("/Volumes/Work/Private/spu-agi-resources/games/KQ1"));
Volume.for(new NodeStorageAccess("/Volumes/Work/Private/spu-agi-resources/games/KQ2"));
Volume.for(new NodeStorageAccess("/Volumes/Work/Private/spu-agi-resources/games/KQ3"));
Volume.for(new NodeStorageAccess("/Volumes/Work/Private/spu-agi-resources/games/KQ4"));
