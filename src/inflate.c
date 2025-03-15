#include "inflate.h"
#include <stdio.h>
#include <stdlib.h>

int inflate(uint8_t* stream,size_t size,uint8_t** output){
    if (size < 16){
        return -1; //file is unreasonably small
    }
    if (stream == NULL){
        return -2; //stream was null
    }   
    uint16_t check = (stream[0] << 4) + stream[1];
    if (check % 31 != 0){
        return -3; //data stream check bits were invalid
    }
    //CINFO
    uint8_t compressionMethod = (stream[0] & 0b00001111);
    uint8_t compressionInfo = (stream[0] & 0b11110000) >> 4;
    //FLAGS
    uint8_t dictionaryPresent = (stream[1] & 0b00000100) >> 3;
    uint8_t compressionLevel = (stream[1] & 0b00000011);

    if (compressionMethod != 8){
        return -4; //compression method is unsupported
    }

    uint32_t ptr = 2;
    uint32_t slidingWindowSize = 1 << compressionInfo;
    uint8_t* slidingWindow = malloc((size_t) slidingWindow);
    if (dictionaryPresent){
        ptr += 6;
        uint16_t dictionaryChecksum_S1 = (stream[2] << 8) | stream[4];
        uint16_t dictionaryChecksum_S2 = (stream[5] << 8) | stream[6];

        uint16_t adler32_S1 = 1;
        uint16_t adler32_S2 = 0;
        /* 
        int i = 7;
        while (adler32_S1 != dictionaryChecksum_S1 && adler32_S2 != dictionaryChecksum_S2){
            if (i >= size){
                return -5; //dictionary length extended past EOF
            }
            adler32_S1 = (adler32_S1+stream[i])%(65521);
            adler32_S2 = (adler32_S2 + adler32_S1)%(65521);
            i++;
        }
        */
    }

    uint8_t isLastBlock = 0;

    while (!isLastBlock){
        isLastBlock = (stream[ptr] & 0b10000000) >> 7;
        uint8_t blockType = (stream[ptr] & 0b01100000) >> 5;
        printf("Block Type: %d \n",blockType);
        switch(blockType){
            case 0b00: //No compression
                ptr+=1;
                uint16_t length = stream[ptr]<<8 | stream[ptr+1];
                ptr+=2;
                uint16_t lengthCheck = stream[ptr]<<8 | stream[ptr+1];

                if (length != !lengthCheck){
                    return -5; //length is corrupted
                }

                //TODO: copy length bytes from stream to output
                ptr += length;

                break;
            case 0b01://fixed huffman codes
                break;
            case 0b10://dynamic huffman codes
                break; 
            case 0b11:
                return -100; //reserved type (something has gone really bad)
        }
    }

    //uint8_t presetDictionary = (*stream)[1];
    //uint8_t compressionLevle = (*stream)[1];
    free(slidingWindow);

    return 1;
}