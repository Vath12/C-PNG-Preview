#include "pngDecoder.h"
#include "pngChunks.h"
#include "util.h"
#include "inflate/decoder.h"
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
    uint8_t usePallate = 0;
    while (ptr < size){
        ChunkData c = readChunk(rawData,ptr);
        //printf("Chunk Code: %s",c.code);
        //printf(" span %u at %u\n",c.length,ptr);
        ptr += c.length + 12;

        if (strcmp(c.code,CHUNK_CODE_IDAT) == 0){
           if (!parseIDAT(&c,&data)){
                return -64;// out of memory  
           }
        } else if (strcmp(c.code,CHUNK_CODE_IHDR) == 0){
            parseIHDR(&c,&header);
            printf("PNG Data: W:%d H:%d BitDepth:%d ColorType:%d compressionMethod:%d\n",
            header.width,header.height,header.bitDepth,header.colorType,header.compressionMethod);
        }  else if (strcmp(c.code,CHUNK_CODE_PLTE) == 0){
            usePallate = 1;
            parsePLTE(&c,&pallate);
        }
        
    }
    //printf("Compressed image data size: %dkb\n",data.size/1000);
    uint8_t* uncompressed = malloc(0);
    size_t uncompressedSize = 0;

    printf("inflate exited with code %d\n",inflate(&uncompressed,&uncompressedSize,data.buffer,data.size));
    printf("bytestream size %lu\n",uncompressedSize);
    uint64_t imgPtr = 0;
    printf("bd:%d x%d\n",header.bitDepth,header.valuesPerPixel);


    imgPtr = 0;
    uint32_t pixel = 0;
    RGBA *image = calloc(uncompressedSize,sizeof(RGBA));

    for (int i = 0; i < uncompressedSize;i++){
        //f_b(uncompressed[i],8);
        printf("%d ",uncompressed[i]);
    }
    printf("\n");

    //header.heightx
    for (uint16_t row = 0;row<2;row++){
        uint8_t filter = uncompressed[imgPtr/8];
        imgPtr += 8;
        printf("filter row %d: %d\n",row,filter);
        for (uint16_t i = 0; i < header.width;i++){
            if (usePallate){
                RGB8 color = pallate.colors[getBitsMSB(uncompressed,imgPtr,header.bitDepth)];
                image[pixel].r = color.r;
                image[pixel].g = color.g;
                image[pixel].b = color.b;
                printf("PLTE[%x]: ",getBitsMSB(uncompressed,imgPtr,header.bitDepth));
                printf("%d %d %d\n",image[pixel].r,image[pixel].g,image[pixel].b);
                imgPtr+=header.bitDepth;
                pixel++;
                continue;
            }
            if (header.colorType == 2){
                image[pixel].r = getBitsMSB(uncompressed,imgPtr,header.bitDepth);
                imgPtr+=header.bitDepth;
                image[pixel].g = getBitsMSB(uncompressed,imgPtr,header.bitDepth);
                imgPtr+=header.bitDepth;
                image[pixel].b = getBitsMSB(uncompressed,imgPtr,header.bitDepth);
                imgPtr+=header.bitDepth;
                printf("%d %d %d\n",image[pixel].r,image[pixel].g,image[pixel].b);
                pixel++;
            }
            switch (filter){
                case 0:
                {
                    break;
                }
                case 1:
                {
                    break;
                }
                case 2:
                {
                    break;
                }
                case 3:
                {
                    break;
                }
                case 4:
                {
                    break;
                }
            }
        }
        if (imgPtr%8 != 0){
            imgPtr = (imgPtr/8 + 1)*8;
        }
    }
    

    free(image);
    free(uncompressed);
    free(data.buffer);
    free(rawData);

    return 1;
}