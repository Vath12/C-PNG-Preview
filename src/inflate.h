#pragma once
#include <stdlib.h>
#include "canonicalPrefix.h"

/*
Implementation assumes the zlib data format used in PNGs
http://www.libpng.org/pub/png/book/chapter09.html#png.ch09.div.2

If you would like to understand how the compressed data header is structured
read rfc 1950:
https://www.hjp.at/doc/rfc/rfc1950.pdf


If you would like to understand how the inflate implementation
works please read rfc 1951:
https://www.rfc-editor.org/rfc/rfc1951
*/

/*
Fixed Codes see rfc 1951 pg 11

Length Literal
0-255: literal byte values
256: setinal, EOB marker
257-285: length codes

Distance
There are 29 codes

*/

static const uint16_t END_OF_BLOCK = 256;

typedef struct ZlibHeader{
    uint8_t compressionMethod;
    uint8_t compressionInfo;
    uint8_t hasDictionary;
    uint8_t compressionLevel;
    uint8_t isValid;
    uint8_t length;
} ZlibHeader;

typedef struct ZlibBlock{
    uint8_t *bitstream;
    uint8_t isLastBlock;
    uint8_t blockType;
} ZlibBlock;

typedef struct ZlibDynamicHeader{
    uint16_t numLengthLiteralCodes;
    uint8_t numDistanceCodes;
    uint8_t numCodeLengthCodes;
} ZlibDynamicHeader;

typedef struct zlibAlphabets{
    CPrefixCodeTable *lengthLiteral;
    CPrefixCodeTable *distance;
} zlibAlphabets;

ZlibHeader parseZlibHeader(uint8_t *buffer,uint64_t *ptr);
ZlibBlock parseZlibBlockHeader(uint8_t *buffer,uint64_t *ptr);

int inflate(uint8_t *buffer,size_t size,uint8_t **output);