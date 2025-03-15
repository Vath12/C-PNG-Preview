#include <stdio.h>
#include <stdlib.h>
#include "fileIO.h"
#include "inflate.h"
#include "pngDecoder.h"

int main(){

    /*
    unsigned char writeData[] = "qwertyuiop";
    unsigned char *data;
    size_t dataSize = 0;
    (void*) writeFile("../resources/data.txt",&writeData[0],sizeof(writeData)-1);
    if (readFile("../resources/data.txt",&data,&dataSize)){
        for (int i = 0; i < dataSize; i++){
            printf("%c",data[i]);
        }
        printf("\n");

        free(data);
    }
    */

    printf("readPNG exited with code: %d\n",readPNG("../resources/image.png"));
    uint8_t buffer[] = {11,73,45,46,201,204,75,15,129,80,0};

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

    return 0;

}   