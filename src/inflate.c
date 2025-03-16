#include "inflate.h"
#include <stdio.h>
#include <stdlib.h>



uint8_t nextBit(BitStream *stream){
    uint8_t out = (stream->buffer)[stream->byte];
    out = (out >> (7-stream->bit) ) & 0b1;
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

int inflate(uint8_t *buffer,size_t size,uint8_t **output){
    /*
    if (size < 16){
        return -1; //file is unreasonably small
    }
    if (buffer == NULL){
        return -2; //buffer was null
    }   
    uint16_t check = (buffer[0] << 4) + buffer[1];
    if (check % 31 != 0){
        return -3; //data buffer check bits were invalid
    }
    //CINFO
    uint8_t compressionMethod = (buffer[0] & 0b00001111);
    uint8_t compressionInfo = (buffer[0] & 0b11110000) >> 4;
    //FLAGS
    uint8_t dictionaryPresent = (buffer[1] & 0b00000100) >> 3;
    uint8_t compressionLevel = (buffer[1] & 0b00000011);

    if (compressionMethod != 8){
        return -4; //compression method is unsupported
    }
    */

    //for testing
    uint8_t compressionInfo = 7;

    BitStream stream = {};
    stream.buffer = buffer;
    stream.byte = 0;
    stream.bit = 0;
    //stream.byte = 2;
    
    uint32_t slidingWindowSize = 1 << (compressionInfo+8);
    uint8_t* slidingWindow = malloc((size_t) slidingWindow);

    /*
    if (dictionaryPresent){
        stream.byte+=6;
        uint16_t dictionaryChecksum_S1 = (buffer[2] << 8) | buffer[4];
        uint16_t dictionaryChecksum_S2 = (buffer[5] << 8) | buffer[6];

        uint16_t adler32_S1 = 1;
        uint16_t adler32_S2 = 0;
        
        //int i = 7;
        //while (adler32_S1 != dictionaryChecksum_S1 && adler32_S2 != dictionaryChecksum_S2){
        //    if (i >= size){
        //        return -5; //dictionary length extended past EOF
        //    }
        //    adler32_S1 = (adler32_S1+buffer[i])%(65521);
        //    adler32_S2 = (adler32_S2 + adler32_S1)%(65521);
        //    i++;
        //}
        
    }
    */

    printf("read buffer: ");
    for (int i = 0; i < 2;i++){
        for (int k = 0; k < 8;k++){
            printf("%d", nextBit(&stream));
        }
        printf(" ");
    }
    printf("\n");
    stream.bit -= 2;

    printf("read buffer: ");
    for (int i = 0; i < 2;i++){
        printf("%x ", nextBits(&stream,8));
    }
    printf("\n");
    stream.bit -= 2;

    uint8_t isLastBlock = 0;
    
    while (!isLastBlock){

        //TODO: handle the block not being on a byte boundary
        isLastBlock = (uint8_t) nextBit(&stream);
        uint8_t blockType = (uint8_t) nextBits(&stream,2);

        printf("Block Type: %x \n",blockType);
        switch(blockType){
            case 0b00: //No compression
                //uint16_t length = (uint16_t) nextBits(&stream,16);
                /*uint16_t lengthCheck = (uint16_t) nextBits(&stream,16);

                if (length != !lengthCheck){
                    return -5; //length is corrupted
                }
                */
                //TODO: copy length bytes from buffer to output
                break;
            case 0b01://fixed huffman codes
                break;
            case 0b10://dynamic huffman codes
                break; 
            case 0b11:
                return -100; //reserved type (something has gone really bad)
        }
    }

    //uint8_t presetDictionary = (*buffer)[1];
    //uint8_t compressionLevle = (*buffer)[1];
    free(slidingWindow);

    return 1;
}