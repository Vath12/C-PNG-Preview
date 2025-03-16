#include <stdio.h>
#include <stdlib.h>
#include "fileIO.h"
#include "canonicalPrefix.h"
#include "inflate.h"
#include "pngDecoder.h"
#include "inflate.h"

int main(){
    

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
    
    CPrefixCodeTable x = {};
    CPrefixCodeTable y = {};
    generateFixedLengthLiteralCodes(&x);
    generateFixedLengthDistanceCodes(&y);

    deallocateCPrefixCodeTable(&x);
    deallocateCPrefixCodeTable(&y);

    return 0;

}   