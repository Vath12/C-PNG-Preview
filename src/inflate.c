#include "inflate.h"
#include "bitUtil.h"
#include <stdio.h>
#include <stdlib.h>

/*
TODO:
    Implement LZSS sliding window using 32kb ring buffer
    Utilize canonicalPrefix table to parse the bitstream
    Parse prefix code lengths and generate CPrefixCodeTable in dynamic mode
*/

/*
Note: bits are ordered |MSB 7 6 5 4 3 2 1 LSB| within the byte
values are in machine (MSB) order.
see rfc 1950 for details
*/
ZlibHeader parseZlibHeader(uint8_t *buffer,uint64_t *ptr){
    ZlibHeader header = {};
    header.compressionMethod = getBitsMSB(buffer,*ptr+4,4);
    header.compressionInfo = getBitsMSB(buffer,*ptr,4);
    header.hasDictionary = getBit(buffer,*ptr + 15);
    header.compressionLevel = getBitsMSB(buffer,*ptr+8,2);
    header.isValid = ((uint16_t)getBitsMSB(buffer,*ptr,16)) % 31 == 0;
    *ptr = *ptr + 16;
    return header;
}

ZlibBlock parseZlibBlockHeader(uint8_t *buffer,uint64_t *ptr){
    ZlibBlock block = {};
    block.isLastBlock = getBit(buffer, *ptr);
    block.isLastBlock = getBitsMSB(buffer,*ptr + 1,2);
    *ptr = *ptr + 3;
    return block;
}

int inflate(uint8_t *buffer,size_t size,uint8_t **output){

    uint64_t ptr = 0;

    ZlibHeader head = parseZlibHeader(buffer,&ptr);
    ZlibBlock block = parseZlibBlockHeader(buffer,&ptr);

    printf("CM: %d CINFO: %d FDICT %d FLEVEL %d VALID %d\n",
        head.compressionMethod,
        head.compressionInfo,
        head.hasDictionary,
        head.compressionLevel,
        head.isValid);

    printf("isLast: %d blockType: %d\n",block.isLastBlock,block.blockType);

    switch (block.blockType){
        case 0: //uncompressed block (store)
            {
            //ptr = ((ptr/8) + 1)*8; //flush to byte
            uint16_t length = getBitsLSB(buffer,ptr,16);
            ptr+=16;
            uint16_t nlength = ~(getBitsLSB(buffer,ptr,16));
            printf("%d %d\n",length,nlength);
            if (length != nlength){
                printf("block length did not match check value\n");
                return -2; 
            }
            //TODO: write len number of bits from bitstream
            ptr += length*8;
            break;
            }
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