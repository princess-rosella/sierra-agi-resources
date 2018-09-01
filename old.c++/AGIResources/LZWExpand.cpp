//
//  LZWExpand.cpp
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#include "LZWExpand.hpp"

#include <exception>
#include <memory>

using namespace AGI::Resources;

namespace AGI { namespace Resources {

    class LZWExpander {
    private:
        enum {
            MAXBITS     = 12,
            TABLE_SIZE  = 18041,    // strange number
            START_BITS  = 9
        };

        int32_t BITS, MAX_VALUE, MAX_CODE;
        uint32_t prefixCode[TABLE_SIZE];
        uint8_t appendCharacter[TABLE_SIZE];
        uint8_t decodeStack[8192];
        int32_t inputBitCount;    // Number of bits in input bit buffer
        uint32_t inputBitBuffer;

    public:
        LZWExpander();

        bool expand(const uint8_t* in, uint8_t* out, size_t len);

    private:
        bool setBits(int32_t value);
        uint8_t *decodeString(uint8_t *buffer, uint32_t code);
        uint32_t inputCode(const uint8_t** input);
    };

}}

LZWExpander::LZWExpander(): inputBitCount(0), inputBitBuffer(0), BITS(0), MAX_VALUE(0), MAX_CODE(0) {
    memset(decodeStack, 0, sizeof(decodeStack));
    memset(appendCharacter, 0, sizeof(appendCharacter));
    memset(prefixCode, 0, sizeof(prefixCode));
}

/**
 * Adjust the number of bits used to store codes to the value passed in.
 */
bool LZWExpander::setBits(int32_t value) {
    if (value == MAXBITS)
        return true;

    BITS = value;
    MAX_VALUE = (1 << BITS) - 1;
    MAX_CODE = MAX_VALUE - 1;
    return false;
}

/**
 * Return the string that the code taken from the input buffer
 * represents. The string is returned as a stack, i.e. the characters are
 * in reverse order.
 */
uint8_t* LZWExpander::decodeString(uint8_t* buffer, uint32_t code) {
    uint32_t i;

    for (i = 0; code > 255;) {
        *buffer++ = appendCharacter[code];
        code = prefixCode[code];
        if (i++ >= 4000)
            throw std::runtime_error("lzw: error in code expansion");
    }

    *buffer = code;
    return buffer;
}

/**
 * Return the next code from the input buffer.
 */
uint32_t LZWExpander::inputCode(const uint8_t** input) {
    uint32_t r;

    while (inputBitCount <= 24) {
        inputBitBuffer |= (uint32_t) * (*input)++ << inputBitCount;
        inputBitCount += 8;
    }

    r = (inputBitBuffer & 0x7FFF) % (1 << BITS);
    inputBitBuffer >>= BITS;
    inputBitCount -= BITS;
    return r;
}

/**
 * Uncompress the data contained in the input buffer and store
 * the result in the output buffer. The fileLength parameter says how
 * many bytes to uncompress. The compression itself is a form of LZW that
 * adjusts the number of bits that it represents its codes in as it fills
 * up the available codes. Two codes have special meaning:
 *
 *  code 256 = start over
 *  code 257 = end of data
 */
bool LZWExpander::expand(const uint8_t *in, uint8_t *out, size_t len) {
    int32_t c, lzwnext, lzwnew, lzwold;
    uint8_t* s;
    uint8_t* end;

    setBits(START_BITS); // Starts at 9-bits
    lzwnext = 257;       // Next available code to define

    end = out + len;

    lzwold = c = inputCode(&in); // Read in the first code
    lzwnew = inputCode(&in);

    while ((out < end) && (lzwnew != 0x101)) {
        if (lzwnew == 0x100) {
            // Code to "start over"
            lzwnext = 258;
            setBits(START_BITS);
            lzwold = inputCode(&in);
            c = lzwold;
            *out++ = (char)c;
            lzwnew = inputCode(&in);
        }
        else {
            if (lzwnew >= lzwnext) {
                // Handles special LZW scenario
                *decodeStack = c;
                s = decodeString(decodeStack + 1, lzwold);
            }
            else {
                s = decodeString(decodeStack, lzwnew);
            }

            // Reverse order of decoded string and store in out buffer
            c = *s;
            while (s >= decodeStack)
                *out++ = *s--;

            if (lzwnext > MAX_CODE)
                setBits(BITS + 1);

            prefixCode[lzwnext] = lzwold;
            appendCharacter[lzwnext] = c;
            lzwnext++;
            lzwold = lzwnew;

            lzwnew = inputCode(&in);
        }
    }

    return out == end;
}

bool AGI::Resources::LZWExpand(const uint8_t* input, size_t inputSize, uint8_t* output, size_t outputSize) {
    std::unique_ptr<LZWExpander> expander(new LZWExpander());
    return expander->expand(input, output, outputSize);
}
