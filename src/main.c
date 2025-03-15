#include <stdio.h>
#include <stdlib.h>
#include "fileIO.h"
#include "inflate.h"
#include "pngDecoder.h"

int main(){

    /*
    char writeData[] = "qwertyuiop";
    char *data;
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

    return 0;

}   