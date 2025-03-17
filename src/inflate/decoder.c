#include "decoder.h"
#include <stdlib.h>

const int MAX_CODE_LENGTH = 15;

struct prefixAlphabet{
    uint16_t *literals;
    uint16_t *codes;
};

//see rfc 1951 pg 7
int generateCodes(struct prefixAlphabet *alphabet, uint8_t *lengths, uint16_t size){
    //count the number of distinct lengths present
    uint8_t maxLength = 0;
    for (int i = 0; i < size;i++){
        maxLength = max(maxLength,lengths[i]);
    }
    //allocate extra elem bc codes with length 0 are the 0th element
    uint16_t length_count[MAX_CODE_LENGTH] = {0};
    uint16_t next_code[MAX_CODE_LENGTH] = {0};
    //for length N, count the number of codes with length N
    for (int i = 0; i < size;i++){
        length_count[lengths[i]] += 1;
    }
    //initialize codes
    uint16_t code = 0;
    length_count[0] = 0;
    for (int i = 1; i <= maxLength;i++){
        code = (code + length_count[i-1]) << 1;
        next_code[i] = code;
    }
    //assign codes to all literals
    for (uint16_t i = 0; i < size;i++){
        //only assign codes with nonzero length
        uint16_t length = lengths[i];
        if (length != 0){
            alphabet->codes[i] = next_code[length] & (((uint16_t) -1) >> (16-length));
            next_code[length] += 1;
        }
    }
    return 1;
}

int deflate(uint8_t *out,uint64_t *outputLength,uint8_t *src,uint64_t srcLength){
    return 1;
}