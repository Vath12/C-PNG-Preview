#include "canonicalPrefix.h"
#include "bitUtil.h"
#include <stdio.h>
#include <stdlib.h>

int max(a,b){
    return a>b ? a:b;
}

int compareCode(const void *A, const void *B)
{
    return (*((CPrefixCode*)A)).code - (*((CPrefixCode*)B)).code;
}

int compareValue(const void *A, const void *B)
{
    return (*((CPrefixCode*)A)).value - (*((CPrefixCode*)B)).value;
}

int compareLength(const void *A, const void *B)
{
    return (*((CPrefixCode*)A)).length - (*((CPrefixCode*)B)).length;
}

int compareLengthValue(const void *A, const void *B){
    int lengthDiff = compareLength(A,B);
    if (lengthDiff != 0) {
        return lengthDiff;
    }
    return compareValue(A,B);
}

int allocateCPrefixCodeTable(CPrefixCodeTable* output,uint16_t size){
    output->size = size;
    output->codes = malloc(output->size * sizeof(CPrefixCode));
    if (output->codes == NULL){
        return -1; //out of memory
    }
    return 1;
}

void deallocateCPrefixCodeTable(CPrefixCodeTable* table){
    free(table->codes);
}

int generateFixedLengthDistanceCodes(CPrefixCodeTable* output){
    if (!allocateCPrefixCodeTable(output,32)){
        return -1; //out of memory
    }
    for (uint16_t i = 0; i < 32;i++){
        output->codes[i].value = i;
        output->codes[i].length = 5;
    }
    if (!generateCodes(output)){
        return -2; //code generation failed
    }
    return 1;
}

int generateFixedLengthLiteralCodes(CPrefixCodeTable* output){

    if (!allocateCPrefixCodeTable(output,286)){
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
        return -2; //code generation failed
    }
    for (int i = 0; i < output->size;i++){
        printf("%4d %4d ",
            output->codes[i].length,
            output->codes[i].value);
        for (int k = output->codes[i].length - 1; k >= 0;k--){
            printf("%d",(output->codes[i].code >> k) & 0b1);
        }
        printf("\n");
    }
    
    return 1;
}

//see rfc 1951 pg 7
int generateCodes(CPrefixCodeTable* table){
    //sort by length and value
    qsort(&(table->codes[0]),table->size,sizeof(CPrefixCode),compareLengthValue);
    //count the number of distinct lengths present
    uint8_t maxLength = 0;
    for (int i = 0; i < table->size;i++){
        maxLength = max(maxLength,table->codes[i].length);
    }
    //allocate extra elem bc codes with length 0 are the 0th element
    uint16_t *lengths = calloc(maxLength+1,sizeof(uint16_t));
    uint32_t *next_code = calloc(maxLength+1,sizeof(uint32_t));

    if (lengths == NULL | next_code == NULL){
        return -1; //out of memory
    }
    //count codes with length N
    for (int i = 0; i < table->size;i++){
        lengths[table->codes[i].length] += 1;
    }
    //initialize codes
    uint32_t code = 0;
    for (int i = 1; i <= maxLength;i++){
        code = (code + lengths[i-1]) << 1;
        next_code[i] = code;
    }
    //assign codes to all literals
    for (int i = 0; i < table->size;i++){
        //only assign codes with nonzero length
        if (table->codes[i].length > 0){
            table->codes[i].code = next_code[table->codes[i].length]++;
        }
    }

    //qsort(&(table->codes[0]),table->size,sizeof(CPrefixCode),compareLength);

    free(next_code);
    free(lengths);

    return 1;
}

//see rfc section 3.2.5 (page 11) for all the length/distance details
uint8_t getExtraLengthBits(uint16_t length){
    if (length <= 256){
        return 0;
    }
    return extraBitsLength[length-257];
}
uint32_t getLengthOffset(uint16_t length,uint16_t extraBits){
    return extraBitsLength[length-257] + extraBitOffsetLength[length-257];
}
uint8_t getExtraDistanceBits(uint8_t distance){
    return extraBitsDistance[distance];
}
uint32_t getDistanceOffset(uint8_t distance,uint16_t extraBits){
    return extraBitsDistance[distance] + extraBitOffsetLength[distance];
}

//assumes that table's codes are sorted by their length in ascending order
uint16_t nextCode(uint8_t* buffer,uint64_t *ptr,CPrefixCodeTable* table){
    uint8_t maxLength = table->codes[table->size - 1].length;
    uint8_t length = 0;
    uint16_t code = 0;
    while(length <= maxLength){
        code <<= 1;
        code |= getBit(buffer,*ptr);
        length++;
        *ptr = *ptr + 1;
        //search for matching prefix codes
        for (int i = 0;i < table->size;i++){
            if (table->codes[i].length > length){
                break;
            }
            if (table->codes[i].length == length){
                if (table->codes[i].code == code){
                    return table->codes[i].value;
                }
            }
        }
    }
    return -1; //no matching codes found
}



