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

typedef struct BitStream{
    uint8_t* buffer;
    uint32_t byte; 
    uint8_t bit;
} BitStream;

typedef struct DeflateHeader{
    uint8_t compressionMethod;
    uint8_t compressionInfo;
    uint8_t hasDictionary;
    uint8_t compressionLevel;
    uint8_t isValid;
} DeflateHeader;

DeflateHeader readHeader(uint8_t *buffer);
uint8_t nextBit(BitStream *stream);
uint32_t nextBits(BitStream *stream,int num);

int inflate(uint8_t *buffer,size_t size,uint8_t **output);