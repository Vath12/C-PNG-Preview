#include "decoder.h"
#include <stdlib.h>

const int MAX_CODE_LENGTH = 15;

struct prefixAlphabet{
    uint16_t *literals;
    uint16_t *codes;
};
/*
needed because bytes being in network order
cause both memcpy and casting a pointer to reverse the result
*/
static void revmemcpy(void *dest, void *src, size_t length){
    char *d = dest;
    char *s = src;
    for (int i = 0; i < length; i++){
        d[i] = s[length-1-i];
    }
}

/*
bits are read from the byte in this order:
MSB | 7 6 5 4 3 2 1 0 | LSB

Packing:

Packed into the output number beginning with the LSB
ex.
read 4 bits from 00001000
output: 1000

Packed into the output number beginning with the MSB
ex.
read 4 bits from 00001000
output: 0001
*/
uint8_t getBit(uint8_t *buffer,uint64_t ptr){
    return (buffer[ptr/8] >> (ptr%8)) & 0b1;
}
uint32_t getBitsLSB(uint8_t *buffer,uint64_t ptr,uint8_t num){
    uint32_t out = 0;
    for (uint8_t i = 0; i < num; i++){
        out <<= 1;
        out |= getBit(buffer,ptr+num-i-1);
    }   
    return out;
}
uint32_t getBitsMSB(uint8_t *buffer,uint64_t ptr,uint8_t num){
    uint32_t out = 0;
    for (uint8_t i = 0; i < num; i++){
        out <<= 1;
        out |= getBit(buffer,ptr+i);
    }   
    return out;
}

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

int deflate(uint8_t *out,size_t *outputLength,uint8_t *src,size_t srcLength){

    return 1;
}