#include "inflate.h"
#include <stdio.h>
#include <stdlib.h>


uint8_t getBit(uint8_t *buffer,uint64_t position){
    return (buffer[position/8] >> (7 - (position%8))) & 0b1;
}

uint32_t getBitsMSB(uint8_t *buffer,uint64_t position,uint8_t num){
    uint32_t out = 0;
    for (uint8_t i = 0; i < num; i++){
        out <<= 1;
        out |= getBit(buffer,position+i);
    }   
    return out;
}

uint32_t getBitsLSB(uint8_t *buffer,uint64_t position,uint8_t num){
    uint32_t out = 0;
    for (uint8_t i = 0; i < num; i++){
        out <<= 1;
        out |= getBit(buffer,position+num-i-1);
    }   
    return out;
}

int inflate(uint8_t *buffer,size_t size,uint8_t **output){

    uint64_t ptr = 0;

    uint8_t testBuffer[] = {0b01100000,0b10000000};
    printf("testbuffer: %x \n",getBit(&testBuffer[0],0));
    printf("testbuffer: %x \n",getBitsLSB(&testBuffer[0],0,3));
    printf("testbuffer: %x \n",getBitsMSB(&testBuffer[0],0,3));

    //read header
    uint8_t isLast = getBit(buffer,ptr++);


    return 1;
}