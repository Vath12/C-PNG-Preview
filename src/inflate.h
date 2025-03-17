#pragma once
#include <stdlib.h>

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

typedef struct ZlibBlock{
    uint8_t length;
    uint8_t *bitstream;
} ZlibBlock;


typedef struct ZlibBlockStore{
    uint8_t length;
    uint8_t *bitstream;
} ZlibBlockStore;

typedef struct ZlibBlockFixedHuffman{
    uint8_t *bitstream;
} ZlibBlockFixedHuffman;

typedef struct ZlibBlockDynamicHuffman{
    uint8_t *bitstream;
} ZlibBlockDynamicHuffman;

ZlibBlock parseBlockHeader(uint8_t *buffer,uint64_t *ptr);

int inflate(uint8_t *buffer,size_t size,uint8_t **output);