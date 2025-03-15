#include <stdio.h>
#include <stdlib.h>
#include "fileIO.h"

int main(){

    char writeData[] = "qwertyuiop";
    
    char *data;
    size_t dataSize = 0;

    (void*) fileWrite("../resources/data.txt",&writeData[0],sizeof(writeData)-1);

    if (readFile("../resources/data.txt",&data,&dataSize)){
        for (int i = 0; i < dataSize; i++){
            printf("%c",data[i]);
        }
        printf("\n");

        free(data);
    }

    return 0;

}   