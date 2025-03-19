#include "pngChunks.h"
#include "util.h"
#include <string.h>

const char* CHUNK_CODE_IHDR = "IHDR";
const char* CHUNK_CODE_IDAT = "IDAT";
const char* CHUNK_CODE_PLTE = "PLTE";

//TODO: improve aesthetic of ptr+= (very yucky)
ChunkData readChunk(uint8_t *stream,uint32_t ptr){
    ChunkData data = {0,{0,0,0,0},NULL,0};

    revmemcpy(&data.length,&stream[ptr],4);
    ptr += 4;
    memcpy(data.code,&stream[ptr],4); // the chunk's 4 character ASCII ID
    data.code[4] = '\0';
    ptr += 4;
    data.chunkData = &stream[ptr]; // the chunk's actual data
    ptr+=data.length;
    //TODO: verify checksum is correct given the data 
    //see section 3.4
    //http://www.libpng.org/pub/png/spec/1.2/PNG-Structure.html#CRC-algorithm
    //cyclic rendundancy code (4 byte unsigned int)
    revmemcpy(&data.CRC,&stream[ptr],4);
    return data;
}

int parseIHDR(ChunkData* c, IHDR *output){
    revmemcpy(&output->width,c->chunkData,4);
    revmemcpy(&output->height,&(c->chunkData[4]),4);
    output->bitDepth = c->chunkData[8];
    output->colorType = c->chunkData[9];
    output->compressionMethod = c->chunkData[10];
    switch (output->colorType){
        case 2:
            output->valuesPerPixel = 3;
            break;
        case 4:
            output->valuesPerPixel = 2;
            break;
        case 6:
            output->valuesPerPixel = 4;
            break;
        default:
            output->valuesPerPixel = 1;
            break;
    }
    output->bitsPerPixel = output->bitDepth * output->valuesPerPixel;
    return 1;
}
int parseIDAT(ChunkData* c, IDAT *output){
    uint8_t* resizedBuffer = malloc(output->size + c->length);
    if (resizedBuffer == NULL){
        return -1;
    }
    memcpy(resizedBuffer,output->buffer,output->size);
    memcpy(resizedBuffer+output->size,c->chunkData,c->length);

    free(output->buffer);
    output->size = output->size+c->length;
    output->buffer = resizedBuffer;
    return 1;
}
int parsePLTE(ChunkData* c, PLTE *output){
    memcpy(&(output->colors),c->chunkData,c->length);
    return 1;
}


