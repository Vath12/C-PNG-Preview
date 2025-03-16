#include "pngDecoder.h"
#include "pngChunks.h"
#include "inflate.h"
#include "fileIO.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
Incomplete png decoder, Eats PNG files, spits out a row major array of pixels
see http://www.libpng.org/pub/png/spec/1.2/PNG-Structure.html for details

PNG chunks are handled in the pngChunks.h and c files
*/


int readPNG(char path[]){
    uint8_t* rawData;
    size_t size;
    
    if (!readFile(path,&rawData,&size)){
        return -1; // file read failed
    }
    if (size < 16){
        return -2; // file is too small
    }
    //check MIME code
    const uint8_t PNG_MIME[8] = {137,80,78,71,13,10,26,10};
    for (int i = 0; i < 8; i++){
        if (rawData[i] != PNG_MIME[i]){
            return -3; // file is not a PNG
        }
    }

    uint32_t ptr = 8;
    IHDR header = {};
    IDAT data = {};
    data.buffer = malloc(0);
    PLTE pallate = {};
    while (ptr < size){
        ChunkData c = readChunk(rawData,ptr);
        printf("Chunk Code: %s",c.code);
        printf(" span %u at %u\n",c.length,ptr);
        ptr += c.length + 12;

        if (strcmp(c.code,CHUNK_CODE_IDAT) == 0){
           if (!parseIDAT(&c,&data)){
                return -64;// out of memory  
           }
        } else if (strcmp(c.code,CHUNK_CODE_IHDR) == 0){
            parseIHDR(&c,&header);
            printf("W:%d H:%d BitDepth:%d ColorType:%d compressionMethod:%d\n",
            header.width,header.height,header.bitDepth,header.colorType,header.compressionMethod);
        }  else if (strcmp(c.code,CHUNK_CODE_PLTE) == 0){
            parsePLTE(&c,&pallate);
        }
        
    }
    printf("IDAT size: %dkb\n",data.size/1000);
    uint8_t* uncompressed = malloc(0);
    inflate(data.buffer,data.size,&uncompressed);
    free(uncompressed);

    free(data.buffer);

    free(rawData);
    return 1;
}