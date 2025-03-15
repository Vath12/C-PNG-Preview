#pragma once;
#include <stdlib.h>

typedef struct ChunkData {
    uint32_t length;
    char code[4];
    uint8_t *chunkData;
    uint32_t CRC;
} ChunkData;


typedef struct IHDR{
    uint32_t width;
    uint32_t height;
    uint8_t bitDepth;
    uint8_t colorType;
    uint8_t compressionMethod;
} IHDR;