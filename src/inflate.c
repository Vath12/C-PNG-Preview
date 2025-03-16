#include "inflate.h"
#include <stdio.h>
#include <stdlib.h>



uint8_t nextBit(BitStream *stream){
    uint8_t out = (stream->buffer)[stream->byte];
    out = (out >> (stream->bit) ) & 0b1;
    stream->bit++;
    if (stream->bit > 7){
        stream->bit = 0;
        stream->byte++;
    }
    return out;
}

uint32_t nextBits(BitStream *stream,int num){
    uint32_t out = 0;
    for (int i = 0; i < num; i++){
        out <<= 1;
        out |= nextBit(stream);
    }
    return out;
}

int decodeBlock(BitStream *stream, uint8_t *block){
    //idea for this trick came from zlib (see puff.c):
    unsigned long huffmanCode = 0;
    for (int i = 0; i < 15;i++){
        //add a bit to the queue
        huffmanCode |= nextBit(stream);
        
        //shift bit left in the queue
        huffmanCode <<= 1;
    }
    return -1;
}

DeflateHeader readHeader(uint8_t *buffer){
    DeflateHeader output = {};

    uint16_t check = (buffer[0] << 4) + buffer[1];
    output.isValid = check % 31 == 0;

    //CINFO
    output.compressionMethod = (buffer[0] & 0b00001111);
    output.compressionInfo = (buffer[0] & 0b11110000) >> 4;
    //FLAGS
    output.hasDictionary = (buffer[1] & 0b00000100) >> 3;
    output.compressionLevel = (buffer[1] & 0b00000011);

    return output;
}

int inflate(uint8_t *buffer,size_t size,uint8_t **output){
    if (size < 16){
        return -1; //too few bytes
    }
    BitStream stream = {buffer,2,0};
    
    DeflateHeader header = readHeader(buffer);

    printf("header: CM: %d CINFO: %d DICT: %d CLEVEL: %d VALID: %d\n",
    header.compressionMethod,
    header.compressionInfo,
    header.hasDictionary,
    header.compressionLevel,
    header.isValid);


    uint8_t isLastBlock = 0;
    
    while (!isLastBlock){

        isLastBlock = (uint8_t) nextBit(&stream);
        uint8_t blockType = (uint8_t) nextBits(&stream,2);

        printf("Block Type: %x \n",blockType);
        switch(blockType){
            case 0b00: //No compression
                {
                break;
                }
            case 0b01://fixed huffman codes
                { 
                printf("fixed huffman\n");
                break;
                }
            case 0b10://dynamic huffman codes
                {
                printf("dynamic huffman\n");
                break; 
                }
            case 0b11:
                return -100; //reserved type (something has gone really bad)
        }
    }

    return 1;
}