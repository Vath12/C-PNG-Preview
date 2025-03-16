#include "canonicalPrefixGen.h"
#include <stdio.h>
#include <stdlib.h>

int compareLength(const void *A, const void *B){
    return (*((CPrefixCode*)A)).length-(*((CPrefixCode*)B)).length;
}

int generateFixedLengthLiteralCodes(CPrefixCodeTable* output){

    output->size = 286;
    output->codes = malloc(output->size * sizeof(CPrefixCode));

    if (output->codes == NULL){
        return -1; //out of memory
    }

    for (uint16_t i = 0; i < 286;i++){
        output->codes[i].value = i;
        output->codes[i].code = 0;
        if (i <= 143){
            output->codes[i].length = 8;
        } else if (i <= 255){
            output->codes[i].length = 9;
        } else if (i <= 279){
            output->codes[i].length = 7;
        } else {
            output->codes[i].length = 9;
        }
    }
    if (!generateCodes(output)){
        return -2; //code gen failed
    }

    for (int i = 0; i < output->size;i++){
        printf("%d %d %d\n",
            output->codes[i].length,
            output->codes[i].code,
            output->codes[i].value);
    }

    return 1;
}

int generateCodes(CPrefixCodeTable* table){
    //it would be really nice if qsort was stable...
    qsort(&(table->codes[0]),table->size,sizeof(CPrefixCode),compareLength);
    return 1;
}