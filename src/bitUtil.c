#include "bitUtil.h"
#include <stdlib.h>

uint8_t getBit(uint8_t *buffer,uint64_t ptr){
    return (buffer[ptr/8] >> (7 - (ptr%8))) & 0b1;
}

uint32_t getBitsMSB(uint8_t *buffer,uint64_t ptr,uint8_t num){
    uint32_t out = 0;
    for (uint8_t i = 0; i < num; i++){
        out <<= 1;
        out |= getBit(buffer,ptr+i);
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

uint8_t getBit_r(uint8_t *buffer,uint64_t ptr){
    return (buffer[ptr/8] >> (ptr%8)) & 0b1;
}

uint32_t getBitsMSB_r(uint8_t *buffer,uint64_t ptr,uint8_t num){
    uint32_t out = 0;
    for (uint8_t i = 0; i < num; i++){
        out <<= 1;
        out |= getBit_r(buffer,ptr+i);
    }   
    return out;
}

uint32_t getBitsLSB_r(uint8_t *buffer,uint64_t ptr,uint8_t num){
    uint32_t out = 0;
    for (uint8_t i = 0; i < num; i++){
        out <<= 1;
        out |= getBit_r(buffer,ptr+num-i-1);
    }   
    return out;
}