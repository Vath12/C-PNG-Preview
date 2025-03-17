#pragma once
#include <stdlib.h>

uint8_t getBit(uint8_t *buffer,uint64_t ptr);
uint32_t getBitsMSB(uint8_t *buffer,uint64_t ptr,uint8_t num);
uint32_t getBitsLSB(uint8_t *buffer,uint64_t ptr,uint8_t num);