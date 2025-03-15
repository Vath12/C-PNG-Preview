#include "pngChunks.h"
#include <string.h>

const char* CHUNK_CODE_IHDR = "IHDR";
const char* CHUNK_CODE_IDAT = "IDAT";
const char* CHUNK_CODE_PLTE = "PLTE";

//TODO: improve aesthetic of ptr+= (very yucky)
ChunkData readChunk(uint8_t *stream,uint32_t ptr){
    ChunkData data = {0,{0,0,0,0},NULL,0};
    data.length = // length (4 byte unsigned int)
        (stream[ptr] << 24) | 
        (stream[ptr + 1] << 16) | 
        (stream[ptr + 2] << 8) | 
        (stream[ptr + 3]); 
    ptr += 4;
    memcpy(data.code,&stream[ptr],4); // the chunk's 4 character ASCII ID
    data.code[4] = '\0';
    ptr += 4;
    data.chunkData = &stream[ptr]; // the chunk's actual data
    ptr+=data.length;
    //TODO: verify checksum is correct given the data 
    //see section 3.4
    //http://www.libpng.org/pub/png/spec/1.2/PNG-Structure.html#CRC-algorithm
    data.CRC = //cyclic rendundancy code (4 byte unsigned int)
        (stream[ptr] << 24) | 
        (stream[ptr + 1] << 16) | 
        (stream[ptr + 2] << 8) | 
        (stream[ptr + 3]); 
    return data;
}

/*
needed because all the bytes are in network order
so both memcpy and casting a pointer reverse the result
*/
void revmemcpy(void *dest, void *src, size_t length){
    char *d = dest;
    char *s = src;
    for (int i = 0; i < length; i++){
        d[i] = s[length-1-i];
    }
}

int parseIHDR(ChunkData* c, IHDR *output){
    revmemcpy(&output->width,c->chunkData,4);
    revmemcpy(&output->height,&(c->chunkData[4]),4);
    output->bitDepth = c->chunkData[8];
    output->colorType = c->chunkData[9];
    output->compressionMethod = c->chunkData[10];


    return 1;
}
int parseIDAT(ChunkData* c, IDAT *output){
    revmemcpy(output,c->chunkData,c->length);
    return 1;
}
int parsePLTE(ChunkData* c, PLTE *output){
    revmemcpy(&output,c->chunkData,c->length);
    return 1;
}


