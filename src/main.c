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

    //for (int i = 0; i < outputSize && i < 1 << 12;i++){
    //    printf("%c",uncompressed[i]);
    //}
    //printf("\n");

    writeFile("../resources/testFileUncompressed.txt",uncompressed,outputSize);

    free(uncompressed);
    free(fileData);

    
    //printf("readPNG exited with code: %d\n",readPNG("../resources/image.png"));
    /*
    uint8_t buffer[] = {5,193,1,1,0,0,8,195,32,110,255,206,19,68,210,77,194,3};
    
    printf("buffer: ");
    for (int i = 0; i < sizeof(buffer); i++){
        uint8_t x = buffer[i];
        for (int i = 0; i < 8; i++){
            printf("%d", (x>>i) & 1);
        }
        printf(" ");
    }
    printf("\n");
    uint8_t* output = malloc(0);
    printf("Inflate exited with code: %d",inflate(&buffer,sizeof(buffer),&output));
    free(output);
    */
    
    /*
    CPrefixCodeTable LL = {};
    CPrefixCodeTable distance = {};
    generateFixedLengthLiteralCodes(&LL);
    generateFixedLengthDistanceCodes(&distance);

    uint8_t buffer[] = {0b00110000,0b00110001};
    uint64_t ptr = 0;

    while (ptr < 16){
        uint16_t code = nextCode(&(buffer[0]),&ptr,&LL);
        if (code == ((uint16_t) -1)){
            printf("malformed prefix code \n");
        } else {
            printf("%d ",code);
        }
    }

    deallocateCPrefixCodeTable(&LL);
    deallocateCPrefixCodeTable(&distance);
    */

    return 0;

}   