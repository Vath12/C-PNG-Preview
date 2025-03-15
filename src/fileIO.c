#include <stdio.h>
#include <stdlib.h>

int readFile(char filePath[],unsigned char** data,size_t* size){
    FILE *filePointer = fopen(filePath,"rb");

    if (filePointer == NULL){
        return 0;
    }

    //find EOF
    fseek(filePointer,0L,SEEK_END);
    //get position of file pointer
    *size = ftell(filePointer);
    
    *data = (char*) malloc(*size);
    
    if (*data == NULL){
        return 0;
    }
     
    rewind(filePointer);

    for (int i = 0; i < *size; i++){
        (*data)[i] = fgetc(filePointer);
    }

    fclose(filePointer);
    
    return 1;
}

int writeFile(char filePath[],unsigned char* data, size_t size){
    FILE *filePointer = fopen(filePath,"wb");

    if (filePointer == NULL){
        return 0;
    }
    
    if (data == NULL){
        return 0;
    }

    int success = fwrite(data,sizeof(char),size,filePointer) == size;

    fclose(filePointer);
    
    return success;
}