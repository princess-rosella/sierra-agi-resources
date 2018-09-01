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

function convertKeyToArray(key: string): number[] {
    const result: number[] = [];

    for (let index = 0; index < key.length; index++)
        result.push(key.charCodeAt(index));

    return result;
}

export const CRYPT_KEY_SIERRA = convertKeyToArray("Avis Durgan");
export const CRYPT_KEY_AGDS   = convertKeyToArray("Alex Simkin");

export function decrypt(data: Uint8Array, key: number[]) {
    const length    = data.length;
    const keyLength = key.length;

    for (let i = 0; i < length; i++)
        data[i] ^= key[i % keyLength];
}

export function decryptLogic(buffer: ArrayBuffer, key: number[]) {
    const data                = new DataView(buffer);
    const messageSectionStart = data.getUint16(0, true) + 2;
    const messageCount        = data.getUint8(messageSectionStart);

    if (messageCount <= 0)
        return;

    const messageSectionEnd   = messageSectionStart + data.getUint16(messageSectionStart + 1, true) + 1;
    const firstMessagePointer = messageSectionStart + 3;
    const firstMessage        = firstMessagePointer + (messageCount << 1);

    decrypt(new Uint8Array(buffer, firstMessage, messageSectionEnd - firstMessage), key);
}
