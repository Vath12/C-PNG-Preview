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

//see rfc section 3.2.5 (page 11)
//correspond to values 257-285
static const uint8_t EXTRA_BITS_LENGTH[] = {
    0,0,0,0,0,0,0,0,
    1,1,1,1,
    2,2,2,2, 
    3,3,3,3,
    4,4,4,4,
    5,5,5,5,0
};
static const uint16_t EXTRA_BITS_LENGTH_OFFSET[] = {
    3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258
};
//correspond to values 0-29
static const uint8_t EXTRA_BITS_DISTANCE[] = {
    0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13
};
static const uint16_t EXTRA_BITS_DISTANCE_OFFSET[] = {
    1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,
    1025,1537,2049,3073,4097,6145,8193,12289,16385,24557
};

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
    CPrefixCodeTable lengthLiteral;
    CPrefixCodeTable distance;
} zlibAlphabets;

ZlibHeader parseZlibHeader(uint8_t *buffer,uint64_t *ptr);
ZlibBlock parseZlibBlockHeader(uint8_t *buffer,uint64_t *ptr);

int inflate(uint8_t *buffer,size_t size,uint8_t **output);