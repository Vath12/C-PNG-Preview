#include "util.h"
#include <stdlib.h>
#include <stdio.h>

/*
needed for bytes being in network order
cause both memcpy and casting a pointer to reverse the result
*/
void revmemcpy(void *dest, void *src, size_t length){
    char *d = dest;
    char *s = src;
    for (int i = 0; i < length; i++){
        d[i] = s[length-1-i];
    }
}

uint8_t getBit_r(uint8_t *buffer,uint64_t ptr){
    return (buffer[ptr/8] >> (7 - (ptr%8))) & 0b1;
}
uint8_t getBit(uint8_t *buffer,uint64_t ptr){
    return (buffer[ptr/8] >> (ptr%8)) & 0b1;
}
uint32_t getBitsMSB(uint8_t *buffer,uint64_t ptr,uint8_t num){
    uint32_t out = 0;
    for (uint8_t i = 0; i < num; i++){
        out  <<= 1;
        out |= getBit_r(buffer,ptr+i);
    }   
    return out;
}
uint32_t getBitsLSB(uint8_t *buffer,uint64_t ptr,uint8_t num){
    uint32_t out = 0;
    for (uint8_t i = 0; i < num; i++){
        out <<= 1;
        out |= getBit(buffer,ptr+num-i-1);
    }   
    return out;
}

//print bits
void f_b(uint64_t value,const uint8_t numBits){
    printf("0b");
    for (int i = 0; i < numBits;i++){
        printf("%llu",(value >> (numBits-i-1)) & 1);
    }
}