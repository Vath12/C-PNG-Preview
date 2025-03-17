#pragma once
#include <stdlib.h>

//retrieval treating byte read order as MSB |0 1 2 3 4 5 6 7| LSB
uint8_t getBit(uint8_t *buffer,uint64_t ptr);
uint32_t getBitsMSB(uint8_t *buffer,uint64_t ptr,uint8_t num);
uint32_t getBitsLSB(uint8_t *buffer,uint64_t ptr,uint8_t num);
//retrieval treating byte read order as MSB |7 6 5 4 3 2 1 0| LSB
uint8_t getBit_r(uint8_t *buffer,uint64_t ptr);
uint32_t getBitsMSB_r(uint8_t *buffer,uint64_t ptr,uint8_t num);
uint32_t getBitsLSB_r(uint8_t *buffer,uint64_t ptr,uint8_t num);