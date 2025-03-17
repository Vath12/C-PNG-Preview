#pragma once
#include <stdlib.h>

static const int MAX_CODE_LENGTH = 15;
static const int MAX_LENGTH_LITERAL_CODES = 286;
static const int MAX_DISTANCE_CODES = 30;

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
int generateCodesFromLength(
    uint8_t *length,
    uint16_t num,
    CPrefixCodeTable* output
);
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