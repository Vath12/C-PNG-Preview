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

uint8_t paeth(uint8_t a,uint8_t b,uint8_t c){
    uint8_t p = a+b-c;
    uint16_t da = (a-p)*(a-p);
    uint16_t db = (b-p)*(b-p);
    uint16_t dc = (c-p)*(c-p);
    
    if (da < db){
        if (da < dc){
            return a;
        }
    }
    return db<dc ? b : c;
}

int readPNG(char path[],RGBA **image,uint16_t *width, uint16_t *height){
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

    uint64_t imgPtr = 0;
    uint32_t pixel = 0;
    *image = calloc(header.width*header.height,sizeof(RGBA));
    *width = header.width;
    *height = header.height;

    uint8_t bytesPerPixel = header.bitsPerPixel/8;
    bytesPerPixel = bytesPerPixel==0? 1:bytesPerPixel;
    //add one to account for the filter byte
    uint16_t bytesPerScanline = 1 + (bytesPerPixel*header.width);

    printf("bd:%d x%d,%d",header.bitDepth,header.valuesPerPixel,bytesPerPixel);

    for (uint16_t row = 0;row<header.height;row++){
        uint8_t filter = uncompressed[imgPtr/8];
        imgPtr += 8;
        //printf("filter row %d: %d\n",row,filter);
        size_t scanline = (imgPtr/8);
        //reverse the filter
        for (uint16_t i = 0; i < bytesPerScanline-1;i++){
            uint8_t left = 0,up = 0,leftUp = 0;
            if (i>0){
                left = uncompressed[scanline-bytesPerPixel];
            }
            if (row > 0) {
                up = uncompressed[scanline-bytesPerScanline];
            }
            if (row > 0 && i > 0) {
                leftUp = uncompressed[scanline-bytesPerScanline-bytesPerPixel];
            }
            switch (filter){
                case 1: //sub
                    uncompressed[scanline] += left;
                    break;
                case 2: //up
                    uncompressed[scanline] += up;
                    break;
                case 3: //average
                    uncompressed[scanline] += (up+left)/2;
                    break;
                case 4: //paeth
                    uncompressed[scanline] += paeth(left,up,leftUp);
                    break;
            }
            scanline++;
        }
        //read the pixel data
        for (uint16_t i = 0; i < header.width;i++){
            if (usePallate){
                RGB8 color = pallate.colors[getBitsMSB(uncompressed,imgPtr,header.bitDepth)];
                (*image)[pixel].r = color.r;
                (*image)[pixel].g = color.g;
                (*image)[pixel].b = color.b;
                //printf("%d | PLTE[%x]: ",pixel,getBitsMSB(uncompressed,imgPtr,header.bitDepth));
                //printf("%d %d %d\n",(*image)[pixel].r,(*image)[pixel].g,(*image)[pixel].b);
                imgPtr+=header.bitDepth;
                pixel++;
                continue;
            }
            if (header.colorType == 2 || header.colorType == 6){
                (*image)[pixel].r = getBitsMSB(uncompressed,imgPtr,header.bitDepth);
                imgPtr+=header.bitDepth;
                (*image)[pixel].g = getBitsMSB(uncompressed,imgPtr,header.bitDepth);
                imgPtr+=header.bitDepth;
                (*image)[pixel].b = getBitsMSB(uncompressed,imgPtr,header.bitDepth);
                imgPtr+=header.bitDepth;
                //printf("%d %d %d\n",(*image)[pixel].r,(*image)[pixel].g,(*image)[pixel].b);
            }
            if (header.colorType >= 6){
                (*image)[pixel].a = getBitsMSB(uncompressed,imgPtr,header.bitDepth);
                imgPtr+=header.bitDepth;
            }
            pixel++;
        }

        if (imgPtr%8 != 0){
            imgPtr = (imgPtr/8 + 1)*8;
        }
    }
    

    free(uncompressed);
    free(data.buffer);
    free(rawData);

    return 1;
}