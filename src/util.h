#pragma once 
#include <stdlib.h>

/*
needed for bytes being in network order
cause both memcpy and casting a pointer to reverse the result
*/
void revmemcpy(void *dest, void *src, size_t length);
uint8_t getBit(uint8_t *buffer,uint64_t ptr);
uint32_t getBitsMSB(uint8_t *buffer,uint64_t ptr,uint8_t num);
uint32_t getBitsLSB(uint8_t *buffer,uint64_t ptr,uint8_t num);

//print bits
void f_b(uint64_t value,const uint8_t numBits);