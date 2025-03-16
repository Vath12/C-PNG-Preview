#include "inflate.h"
#include <stdio.h>
#include <stdlib.h>


uint8_t getBit(uint8_t *buffer,uint64_t position){
    return (buffer[position/8] >> (8 - (position%8))) & 0b1;
}

int inflate(uint8_t *buffer,size_t size,uint8_t **output){

    uint64_t ptr = 0;

    uint8_t testBuffer[] = {0b00000000,0b01000000};
    printf("testbuffer: %d \n",getBit(&testBuffer[0],10));

    return 1;
}