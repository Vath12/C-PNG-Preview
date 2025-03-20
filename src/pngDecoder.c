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
    int p = a+b-c;
    int da = abs(p-a);
    int db = abs(p-b);
    int dc = abs(p-c);
    
    if (da <= db && da <= dc){return a;}
    if (db <= dc){return b;}
    return c;
}

void unfilter(uint8_t filter,uint8_t *prior,uint8_t *current,uint8_t bytesPerPixel,uint16_t width){

    for (uint16_t i = 0; i < (width*bytesPerPixel);i++){
        uint8_t left = 0,up = 0,leftUp = 0;
        if (i>=bytesPerPixel){
            left = current[i-bytesPerPixel];
        }
        if (prior != NULL) {
            up = prior[i];
        }
        if (prior != NULL && i >= bytesPerPixel) {
            leftUp = prior[i-bytesPerPixel];
        }
        
        switch (filter){
            case 1: //sub
                current[i] += left;
                break;
            case 2: //up
                current[i] += up;
                break;
            case 3: //average
                current[i] += (up+left) >>1;
                break;
            case 4: //paeth
                current[i] += paeth(left,up,leftUp);
                break;
        }
        
    }

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

    printf("bd:%d x%d,%d",header.bitDepth,header.valuesPerPixel,bytesPerPixel);
    uint8_t *prior = NULL;
    for (uint16_t row = 0;row<header.height;row++){
        uint8_t filter = uncompressed[imgPtr/8];
        imgPtr += 8;
        //printf("filter row %d: %d\n",row,filter);
        //reverse the filter
        uint8_t *scanline = &(uncompressed[imgPtr/8]);
        unfilter(filter,prior,scanline,bytesPerPixel,header.width);
        prior = scanline;
        
        //read the pixel data
        for (uint16_t i = 0; i < header.width;i++){
            if (usePallate){
                RGB8 color = pallate.colors[getBitsMSB(uncompressed,imgPtr,header.bitDepth)];
                (*image)[pixel].r = color.r;
                (*image)[pixel].g = color.g;
                (*image)[pixel].b = color.b;
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