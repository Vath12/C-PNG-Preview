#pragma once
#include <stdlib.h>

typedef struct CPrefixCode{
    uint8_t length;
    uint16_t code;
    uint16_t value;
} CPrefixCode;

typedef struct CPrefixCodeTable{
    uint16_t size;
    CPrefixCode *codes;
} CPrefixCodeTable;

int allocateCPrefixCodeTable(CPrefixCodeTable* output,uint16_t size);
//see rfc 1951 section 3.2.6.
int generateFixedLengthDistanceCodes(CPrefixCodeTable* output);
//see rfc 1951 section 3.2.6.
int generateFixedLengthLiteralCodes(CPrefixCodeTable* output);
int generateCodes(CPrefixCodeTable* table);