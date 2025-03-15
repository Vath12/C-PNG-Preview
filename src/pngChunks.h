#pragma once
#include <stdlib.h>


const char* CHUNK_CODE_IHDR;
const char* CHUNK_CODE_IDAT;
const char* CHUNK_CODE_PLTE;


/*
specification sourced from 
W3C Recommendation 01-October-1996
https://www.w3.org/TR/PNG-Chunks.html
*/

typedef struct RGB{
    uint16_t R;
    uint16_t G;
    uint16_t B;
} RGB;

typedef struct ChunkData {
    uint32_t length;
    //code includes 4 chars and a null terminator
    char code[5];
    uint8_t *chunkData;
    uint32_t CRC;
} ChunkData;

ChunkData readChunk(uint8_t *stream,uint32_t ptr);

/*
image header specifying the dimensions and color data
*/
typedef struct IHDR{
    uint32_t width;
    uint32_t height;
    /* From W3C:
    Color Type  Allowed Bit Depths  Interpretation
        0       1,2,4,8,16      Each pixel is a grayscale sample.

        2       8,16            Each pixel is an R,G,B triple.

        3       1,2,4,8         Each pixel is a palette index;
                                a PLTE chunk must appear.

        4       8,16            Each pixel is a grayscale sample,
                                followed by an alpha sample.

        6       8,16            Each pixel is an R,G,B triple,
                                followed by an alpha sample.
    */
    uint8_t bitDepth;
    uint8_t colorType;
    uint8_t compressionMethod;
} IHDR;


/* From W3C:
The PLTE chunk contains from 1 to 256 palette entries, each a three-byte series of the form:
    Red:   1 byte (0 = black, 255 = red)
    Green: 1 byte (0 = black, 255 = green)
    Blue:  1 byte (0 = black, 255 = blue)

*/
typedef struct PLTE {
    RGB colors[256];
} PLTE;

/*
Image data can be split among multiple IDAT chunks,
and the boundaries can be anywhere,

from W3C:

IDAT chunk boundaries have no semantic significance and 
can occur at any point in the compressed datastream.
A PNG file in which each IDAT chunk contains only one data byte is legal

For this reason image data will not be uncompressed in parse IDAT
and the IDAT struct will simply wrap the data stream
*/
typedef struct IDAT{
    uint8_t* data;
} IDAT;

int parseIHDR(ChunkData* c, IHDR *output);
int parseIDAT(ChunkData* c, IDAT *output);
int parsePLTE(ChunkData* c, PLTE *output);