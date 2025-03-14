#include <stdio.h>
#include <stdlib.h>

int readFile(char filePath[],char** data,size_t* size){
    FILE *filePointer = fopen(filePath,"r");

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

    
    return 1;
}

int main(){

    char *data;
    size_t dataSize = 0;

    if (readFile("../resources/data.txt",&data,&dataSize)){
        printf("file data %ld bytes:\n",dataSize);
        for (int i = 0; i < dataSize; i++){
            printf("%c",data[i]);
        }
        printf("\n");

        free(data);
    }
    return 0;

}   