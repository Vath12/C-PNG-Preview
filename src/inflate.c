#include "inflate.h"
#include "bitUtil.h"
#include "canonicalPrefix.h"
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
    block.isLastBlock = getBit_r(buffer, *ptr);
    block.blockType = getBitsLSB_r(buffer,*ptr + 1,2);
    *ptr = *ptr + 3;
    return block;
}

ZlibDynamicHeader parseZlibDynamicHeader(uint8_t *buffer,uint64_t *ptr){
    ZlibDynamicHeader head = {};
    head.numLengthLiteralCodes = getBitsLSB_r(buffer,*ptr,5)+257;
    head.numDistanceCodes = getBitsLSB_r(buffer,*ptr+5,5)+1;
    head.numCodeLengthCodes = getBitsLSB_r(buffer,*ptr+10,4)+4;
    *ptr = *ptr + 14;
    return head;
}

//this lil fella's existence is brought to you by rfc 1951 pg 13:
static const uint8_t clCodeAssignmentOrder[] = {
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};
static const uint16_t clCodeLiteral[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19
};

static void addCodeLength(uint8_t *codeLengths,uint8_t codeLength,uint16_t *assignedCodeLengths){
    codeLengths[*assignedCodeLengths+1] = codeLength;
    *assignedCodeLengths = *assignedCodeLengths+1;
}

//assumes that output's code tables are uninitialized and do not need to be freed
int parseDynamicCodeTable(
    uint8_t *buffer, 
    uint64_t *ptr, 
    zlibAlphabets *output,
    ZlibDynamicHeader *header
){
    uint8_t codeLengths[19] = {0};
    /*
    read the lengths for each of the codes
    note: each code length is a 3 bit unsigned integer
    the assignment order is non-linear, and specified by clCodeAssignmentOrder
    */
    for (int i = 0; i < header->numCodeLengthCodes;i++){
        codeLengths[clCodeAssignmentOrder[i]] = getBitsLSB_r(buffer,*ptr,3);
        *ptr = *ptr + 3;
    }
    /*
    generate prefix codes from the supplied lengths and literals
    */
    CPrefixCodeTable codeLengthCode = {};
    int result = generateCodesFromLengthLiteral(
        &codeLengths[0],
        (uint16_t*) &(clCodeLiteral[0]), //cast suppresses warning
        19,
        &codeLengthCode
    );
    if (!result){
        return result; //code length code generation failed
    }

    //the code lengths that we want output
    uint8_t llCodeLengths[MAX_LENGTH_LITERAL_CODES] = {0};
    uint8_t dCodeLengths[MAX_DISTANCE_CODES] = {0};
    uint16_t assignedLLCodeLengths = 0;
    uint16_t assignedDCodeLengths = 0;
    //run length encoding parameters
    uint32_t rleDecodePtr = 0;
    uint32_t numCodes = 0;
    uint8_t lastRunLength = 0;
    uint8_t lastRunSymbol = 0;

    int i = 0;
    while (numCodes < header->numLengthLiteralCodes+header->numDistanceCodes){
        /*
        read and decode the compressed LL and distance codes using the code length code
        */
        uint8_t code = nextCode(buffer,ptr,&codeLengthCode);
        uint8_t extraBits = 0;
        uint8_t runLength = 1;
        switch (code){
            case 16:
                runLength += 2;
                extraBits = 2;
                break;
            case 17:
                code = 0;
                runLength += 2;
                extraBits = 3;
                break;
            case 18:
                code = 0;
                runLength += 10;
                extraBits = 7;
                break;
            default:
                break;
        }
        runLength += getBitsLSB_r(buffer,*ptr,extraBits);
        
        int actualRun = runLength;
        uint8_t actualCode = code;
        if (code == 16){
            actualRun *= lastRunLength;
            actualCode = lastRunSymbol;
        } 
        for (int i = 0; i < actualRun;i++){
            if (numCodes < header->numLengthLiteralCodes){
                addCodeLength(&llCodeLengths[0],actualCode,&assignedLLCodeLengths);
            } else {
                addCodeLength(&dCodeLengths[0],actualCode,&assignedDCodeLengths);
            }
            numCodes++;
        }
        lastRunSymbol = code;
        lastRunLength = runLength;
        *ptr = *ptr + extraBits;
        i++;
    }

    for (int i = 0; i < MAX_LENGTH_LITERAL_CODES;i++){
        printf("%d,",llCodeLengths[i]);
    }
    printf("\n");
    for (int i = 0; i < MAX_DISTANCE_CODES;i++){
        printf("%d,",dCodeLengths[i]);
    }
    printf("\n");
    
    int llResult = generateCodesFromLength(
        llCodeLengths,
        MAX_LENGTH_LITERAL_CODES,
        &output->lengthLiteral);
    printf("\nDistance\n");
    int dResult = generateCodesFromLength(
        dCodeLengths,
        MAX_DISTANCE_CODES,
        &output->distance);

    return llResult ? dResult : llResult;
}

int inflateBlock(
    CPrefixCodeTable *LengthLiteralCodes,
    CPrefixCodeTable *DistanceCodes,
    uint8_t *slidingWindowRingBuffer,
    uint16_t slidingWindowSize,
    uint8_t *data,
    uint8_t **output
){
    return 1;
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
            head.isValid
        );

    printf("isLast: %d blockType: %d\n",block.isLastBlock,block.blockType);

    switch (block.blockType){
        case 0: //uncompressed block (store)
            {
            //ptr = ((ptr/8) + 1)*8; //flush to byte
            uint16_t length = getBitsLSB_r(buffer,ptr,16);
            ptr+=16;
            uint16_t nlength = ~(getBitsLSB_r(buffer,ptr,16));
            printf("%d %d\n",length,nlength);
            if (length != nlength){
                printf("block length did not match check value\n");
                return -2; 
            }
            //TODO: write len number of bits from bitstream
            ptr += length*8;
            break;
            }
        case 1: //block compressed with fixed prefix codes
            break; 
        case 2: //block compressed with dynamic prefix codes
            {
                ZlibDynamicHeader blockHead = parseZlibDynamicHeader(buffer,&ptr);
                printf("LL: %d D: %d CL: %d\n",
                    blockHead.numLengthLiteralCodes,
                    blockHead.numDistanceCodes,
                    blockHead.numCodeLengthCodes
                );
                zlibAlphabets alphabets = {};
                parseDynamicCodeTable(buffer,&ptr,&alphabets,&blockHead);
            }
            break; 
        case 3:
            printf("invalid block type\n");
            return -1;

    }

    return 1;
}