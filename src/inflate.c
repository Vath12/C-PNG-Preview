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
    //read block type
    uint8_t blockType = (uint8_t) getBitsLSB(buffer,ptr,2);
    ptr+=2;
    printf("isLast: %d blockType: %d\n",isLast,blockType);

    switch (blockType){
        case 0: //uncompressed block (store)
            ptr = ((ptr/8) + 1)*8; //flush to byte
            uint16_t length = getBitsLSB(buffer,ptr,16);
            ptr+=16;
            uint16_t nlength = ~(getBitsLSB(buffer,ptr,16));
            if (length != nlength){
                printf("block length did not match check value");
                return -2; 
            }
            //TODO: write len number of bits from bitstream
            ptr += length*8;
            break;
        case 1: //block compressed with fixed huffman codes
            break; 
        case 2: //block compressed with dynamic huffman codes
            break; 
        case 3:
            printf("invalid block type\n");
            return -1;

    }

    return 1;
}