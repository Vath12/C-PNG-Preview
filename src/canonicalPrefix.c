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
    /*
    //Print LL Code 
    for (int i = 0; i < output->size;i++){
        printf("%4d %4d ",
            output->codes[i].length,
            output->codes[i].value);
        for (int k = output->codes[i].length - 1; k >= 0;k--){
            printf("%d",(output->codes[i].code >> k) & 0b1);
        }
        printf("\n");
    }
    */
    return 1;
}

int generateCodesFromLengthLiteral(
    uint8_t *length,
    uint16_t *literal,
    uint16_t num,
    CPrefixCodeTable* output
){
    if (!allocateCPrefixCodeTable(output,num)){
        return -1; //out of memory
    }

    for (uint16_t i = 0; i < num;i++){
        output->codes[i].length = length[i];
        output->codes[i].value = literal[i];
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
        printf(" %d \n",output->codes[i].code);
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
    uint16_t length_count[MAX_CODE_LENGTH] = {0};
    uint16_t next_code[MAX_CODE_LENGTH] = {0};

    //for length N, count the number of codes with length N
    for (int i = 0; i < table->size;i++){
        length_count[table->codes[i].length] += 1;
    }
    //initialize codes
    uint16_t code = 0;
    length_count[0] = 0;
    for (int i = 1; i <= maxLength;i++){
        code = (code + length_count[i-1]) << 1;
        next_code[i] = code;
    }
    //assign codes to all literals
    for (uint16_t i = 0; i < table->size;i++){
        //only assign codes with nonzero length
        uint16_t length = table->codes[i].length;
        if (length != 0){
            table->codes[i].code = next_code[length] & (((uint16_t) -1) >> (16-length));
            next_code[length] += 1;
        }
    }
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
    int nextSearch = 0;
    while(length <= maxLength){
        code <<= 1;
        code |= getBit_r(buffer,*ptr);
        length++;
        *ptr = *ptr + 1;
        //search for matching prefix codes
        for (int i = nextSearch;i < table->size;i++){
            if (table->codes[i].length > length){
                break;
            }
            if (table->codes[i].length == length){
                nextSearch = i;
                if (table->codes[i].code == code){
                    return table->codes[i].value;
                }
            }
        }
    }
    for (int i = 0; i < length;i++){
        printf("%d",code>>i & 0b1);
    }
    printf("\n");
    return -1; //no matching codes found
}



