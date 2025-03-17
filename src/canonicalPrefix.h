#pragma once
#include <stdlib.h>


//see rfc section 3.2.5 (page 11)
//correspond to values 257-285
static const uint8_t extraBitsLength[] = {
    0,0,0,0,0,0,0,0,
    1,1,1,1,
    2,2,2,2, 
    3,3,3,3,
    4,4,4,4,
    5,5,5,5,0
};
static const uint16_t extraBitOffsetLength[] = {
    3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258
};
//correspond to values 0-29
static const uint8_t extraBitsDistance[] = {
    0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13
};
static const uint16_t extraBitOffsetDistance[] = {
    1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,
    1025,1537,2049,3073,4097,6145,8193,12289,16385,24557
};

typedef struct CPrefixCode{
    //the length is no larger than 7 in practice
    uint8_t length;
    uint16_t code;
    uint16_t value;
} CPrefixCode;

typedef struct CPrefixCodeTable{
    uint16_t size;
    CPrefixCode *codes;
} CPrefixCodeTable;



void deallocateCPrefixCodeTable(CPrefixCodeTable* table);
int allocateCPrefixCodeTable(CPrefixCodeTable* table,uint16_t size);
//see rfc 1951 section 3.2.6.
int generateFixedLengthDistanceCodes(CPrefixCodeTable* output);
//see rfc 1951 section 3.2.6.
int generateFixedLengthLiteralCodes(CPrefixCodeTable* output);
int generateCodesFromLengthLiteral(
    uint8_t *length,
    uint16_t *literal,
    uint16_t num,
    CPrefixCodeTable* output
);
int generateCodes(CPrefixCodeTable* table);

uint8_t getExtraLengthCodeBits(uint16_t length);
uint32_t getLengthOffset(uint16_t length,uint16_t extraBits);

uint8_t getExtraDistanceBits(uint8_t distance);
uint32_t getDistanceOffset(uint8_t distance,uint16_t extraBits);

uint16_t nextCode(uint8_t* buffer,uint64_t *ptr,CPrefixCodeTable *table);