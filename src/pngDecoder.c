#include "pngDecoder.h"
#include "fileIO.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


//http://www.libpng.org/pub/png/spec/1.2/PNG-Structure.html


typedef struct ChunkData {
    uint32_t length;
    char code[4];
    char *chunkData;
    uint32_t CRC;
} ChunkData;

ChunkData readChunk(char *stream,uint64_t ptr){
    ChunkData data = {0,{0,0,0,0},NULL,0};
    data.length = (stream[ptr] << 24) | (stream[ptr + 1] << 16) | (stream[ptr + 2] << 8) | (stream[ptr + 3]); // length
    ptr += 4;
    memcpy(data.code,&stream[ptr],4); // the chunk's 4 character ASCII identification
    ptr += 4;
    data.chunkData = &stream[ptr]; // the chunk's actual data
    ptr+=data.length;
    //TODO: verify checksum is correct given the data 
    //see section 3.4
    //http://www.libpng.org/pub/png/spec/1.2/PNG-Structure.html#CRC-algorithm
    data.CRC = (stream[ptr] << 24) | (stream[ptr + 1] << 16) | (stream[ptr + 2] << 8) | (stream[ptr + 3]); // the chunk's checksum
    return data;
}

int readPNG(char path[]){
    char* rawData;
    size_t size;
    
    if (!readFile(path,&rawData,&size)){
        return -1; // file read failed
    }
    if (size < 16){
        return -2; // file is too small
    }
    //check MIME code
    const char PNG_MIME[8] = {137,80,78,71,13,10,26,10};
    for (int i = 0; i < 8; i++){
        if (rawData[i] != PNG_MIME[i]){
            return -3; // file is not a PNG
        }
    }

    uint64_t ptr = 8;

    while (ptr < size){
        ChunkData c = readChunk(rawData,ptr);
        printf("Chunk Code: %s span %d at %lld\n",c.code,c.length,(unsigned long long) ptr);
        ptr += c.length + 12;
    }

    free(rawData);
    return 1;
}