#include <stdio.h>
#include <stdlib.h>
#include "fileIO.h"
#include "pngDecoder.h"
#include "inflate/decoder.h"

int main(){

    uint8_t *fileData = NULL;
    size_t size = 0;
    readFile("../resources/testFile",&fileData,&size);
    uint8_t *uncompressed = NULL;
    size_t outputSize = 0;
    printf("inflate exited with code %d\n",inflate(&uncompressed,&outputSize,fileData,size));
    writeFile("../resources/testFileUncompressed.txt",uncompressed,outputSize);

    free(uncompressed);
    free(fileData);

    //printf("readPNG exited with code: %d\n",readPNG("../resources/image.png"));

    return 0;

}   