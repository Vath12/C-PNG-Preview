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

int generateFixedLengthLiteralCodes(CPrefixCodeTable* output);
int generateCodes(CPrefixCodeTable* table);