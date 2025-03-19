#include <stdio.h>
#include <stdlib.h>
#include "fileIO.h"
#include "pngDecoder.h"
#include "util.h"
#include "inflate/decoder.h"
#include "SDL3/SDL.h"

int main(){

    uint8_t *fileData = NULL;
    size_t size = 0;
    readFile("../resources/testFile",&fileData,&size);
    uint8_t *uncompressed = NULL;
    size_t outputSize = 0;

    //printf("inflate exited with code %d\n",inflate(&uncompressed,&outputSize,fileData,size));
    //writeFile("../resources/testFileUncompressed.txt",uncompressed,outputSize);
    //free(uncompressed);
    //free(fileData);

    //uint8_t buffer[] = {0b00000001,0b00000000};
    //f_b(getBitsLSB(&buffer[0],0,16),16);
    //printf("\n");

    printf("readPNG exited with code: %d\n",readPNG("../resources/image4.png"));

    return 0;

}   