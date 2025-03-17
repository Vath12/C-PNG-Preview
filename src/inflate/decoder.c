#include "decoder.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const int MAX_CODE_LENGTH = 15;
const uint16_t END_OF_BLOCK = 256;

//see rfc section 3.2.5 (page 11)
//correspond to values 257-285
const uint8_t EXTRA_BITS_LENGTH[] = {
    0,0,0,0,0,0,0,0,
    1,1,1,1,
    2,2,2,2, 
    3,3,3,3,
    4,4,4,4,
    5,5,5,5,0
};
const uint16_t EXTRA_BITS_LENGTH_OFFSET[] = {
    3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258
};
//correspond to values 0-29
const uint8_t EXTRA_BITS_DISTANCE[] = {
    0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13
};
const uint16_t EXTRA_BITS_DISTANCE_OFFSET[] = {
    1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,
    1025,1537,2049,3073,4097,6145,8193,12289,16385,24557
};

struct prefixAlphabet{
    uint16_t *literals;
    uint8_t *length;
    uint16_t *codes;
};

int max(a,b){
    return a>b ? a:b;
}
/*
needed because bytes being in network order
cause both memcpy and casting a pointer to reverse the result
*/
static void revmemcpy(void *dest, void *src, size_t length){
    char *d = dest;
    char *s = src;
    for (int i = 0; i < length; i++){
        d[i] = s[length-1-i];
    }
}

/*
bits are read from the byte in this order:
MSB | 7 6 5 4 3 2 1 0 | LSB

Packing:

Packed into the output number beginning with the LSB
ex.
read 4 bits from 00001000
output: 1000

Packed into the output number beginning with the MSB
ex.
read 4 bits from 00001000
output: 0001
*/
uint8_t getBit(uint8_t *buffer,uint64_t ptr){
    return (buffer[ptr/8] >> (ptr%8)) & 0b1;
}
uint32_t getBitsMSB(uint8_t *buffer,uint64_t ptr,uint8_t num){
    uint32_t out = 0;
    for (uint8_t i = 0; i < num; i++){
        out <<= 1;
        out |= getBit(buffer,ptr+i);
    }   
    return out;
}
uint32_t getBitsLSB(uint8_t *buffer,uint64_t ptr,uint8_t num){
    uint32_t out = 0;
    for (uint8_t i = 0; i < num; i++){
        out <<= 1;
        out |= getBit(buffer,ptr+num-i-1);
    }   
    return out;
}

//print bits
void f_b(uint64_t value,const uint8_t numBits){
    printf("0b");
    for (int i = 0; i < numBits;i++){
        printf("%d",(value >> (numBits-i-1)) & 1);
    }
}

struct prefixAlphabet fixedLiteralLength;
struct prefixAlphabet fixedDistance;

//see rfc 1951 pg 7
int generateCodes(struct prefixAlphabet *alphabet, uint8_t *lengths, uint16_t size){
    //count the number of distinct lengths present
    uint8_t maxLength = 0;
    for (int i = 0; i < size;i++){
        maxLength = max(maxLength,lengths[i]);
    }
    //allocate extra elem bc codes with length 0 are the 0th element
    uint16_t length_count[MAX_CODE_LENGTH] = {0};
    uint16_t next_code[MAX_CODE_LENGTH] = {0};
    //for length N, count the number of codes with length N
    for (int i = 0; i < size;i++){
        length_count[lengths[i]] += 1;
    }
    //initialize codes
    uint16_t code = 0;
    length_count[0] = 0;
    for (int i = 1; i <= maxLength;i++){
        code = (code + length_count[i-1]) << 1;
        next_code[i] = code;
    }
    //assign codes to all literals
    for (uint16_t i = 0; i < size;i++){
        //only assign codes with nonzero length
        uint16_t length = lengths[i];
        if (length != 0){
            alphabet->codes[i] = next_code[length] & (((uint16_t) -1) >> (16-length));
            next_code[length] += 1;
        }
    }
    return 1;
}

int generateFixedCodes()
{
    fixedLiteralLength.literals = malloc(286 * sizeof(uint16_t));
    fixedLiteralLength.length = malloc(286 * sizeof(uint8_t));
    fixedLiteralLength.codes = malloc(286 * sizeof(uint16_t));
    fixedDistance.literals = malloc(30 * sizeof(uint16_t));
    fixedDistance.length = malloc(30 * sizeof(uint8_t));
    fixedDistance.codes = malloc(30 * sizeof(uint16_t));

    //TODO: check that malloc did not fail and return code if it did

    uint8_t lengthLL[286] = {0};
    uint8_t lengthD[30] = {5};
    for (int i = 0; i < 30;i++){
        fixedDistance.literals[i] = i;
        fixedDistance.length[i] = 5;
    }
    for (int i = 0; i < 286;i++){
        fixedLiteralLength.literals[i] = i;
        if (i < 144){
            lengthLL[i] = 8;
        } else if (i < 256){
            lengthLL[i] = 9;
        } else if (i < 280){
            lengthLL[i] = 7;
        } else {
            lengthLL[i] = 9;
        }
        fixedLiteralLength.length[i] = lengthLL[i];
    }

    int success = generateCodes(&fixedLiteralLength, lengthLL, 286);
    if (!success){
        return success;
    }
    return generateCodes(&fixedDistance, lengthLL, 30);    
}

uint16_t nextCode(
    uint8_t *src, 
    uint64_t *ptr,
    struct prefixAlphabet *alphabet,
    uint16_t alphabetSize
){
    uint8_t length = 0;
    uint16_t code = 0;
    while (length < MAX_CODE_LENGTH){
        code <<= 1;
        code |= getBit(src,*ptr);
        length++;
        *ptr = *ptr + 1;
        //f_b(code,length);
        //printf("\n");
        for (int i = 0; i < alphabetSize; i++){
            if (length == alphabet->length[i] && code == alphabet->codes[i]){
                return alphabet->literals[i];
            }
        }
    }
    return -1;
}

void appendToBuffer(uint8_t value, uint8_t **buffer, size_t *allocatedSize, size_t *length){
    if (*length+1 >= *allocatedSize){
        *allocatedSize = *allocatedSize+8129;
        *buffer = realloc(*buffer,*allocatedSize);
    }
    *length = *length+1;
    (*buffer)[*length] = value;
}

int deflate(uint8_t **out,size_t *outputLength,uint8_t *src,size_t srcLength){
    uint64_t ptr = 0;

    f_b(getBitsLSB(src,ptr,8),8);
    printf("\n");
    f_b(getBitsLSB(src,ptr,4),4);
    printf("\n");
    f_b(getBitsLSB(src,ptr+4,4),4);
    printf("\n");
    uint8_t compressionMethod = getBitsLSB(src,ptr,4);
    uint8_t compressionInfo = getBitsLSB(src,ptr+4,4);
    uint8_t hasDict = getBitsLSB(src,ptr+13,1);
    uint8_t compressionLevel = getBitsLSB(src,ptr+14,2);
    uint16_t isValid = (((src[0] << 8) | src[1]) % 31) == 0;
    ptr+=16;

    printf("Compression Method: %d\n",compressionMethod);
    printf("Compression Info: %d\n",compressionInfo);
    printf("Compression Uses Dict: %d\n",hasDict);
    printf("Compression Level: %d\n",compressionLevel);
    printf("Valid: %d\n",isValid);

    if (!isValid){
        return -2; //invalid stream header
    }

    if (!generateFixedCodes()){
        return -3; //fixed code generation failed
    }

    for (int i = 0; i < 286; i++){
        printf("%d ",fixedLiteralLength.literals[i]);
        f_b(fixedLiteralLength.codes[i],fixedLiteralLength.length[i]);
        printf("\n");
    }

    size_t allocatedOutput = *outputLength;

    uint8_t isLast = getBitsLSB(src,ptr,1);
    uint8_t blockType = getBitsLSB(src,ptr+1,2);
    ptr += 3;

    printf("isLast : %d\n",isLast);
    printf("blockType : %d\n",blockType);
    
    if (blockType==3){
        return -3;//invalid blocktype
    }
    if (blockType != 0){
        struct prefixAlphabet literalLength = {};
        struct prefixAlphabet distance = {};
        if (blockType == 2){
            
        } else {
            while (1){
                uint16_t code = nextCode(src,&ptr,&fixedLiteralLength,286);
                if (code == END_OF_BLOCK){
                    //EOB
                    break;
                }
                else if (code > END_OF_BLOCK){
                    //extra bits for length code
                    uint8_t extraBitsLength = EXTRA_BITS_LENGTH[code-257];
                    uint8_t length = 
                        getBitsMSB(src,ptr,extraBitsLength) + EXTRA_BITS_LENGTH_OFFSET[code-257]; 
                    ptr += extraBitsLength;
                    //get distance code
                    uint16_t distanceCode = nextCode(src,&ptr,&fixedDistance,30);
                    //extra bits for distance code
                    uint8_t extraBitsDistance = EXTRA_BITS_DISTANCE[distanceCode];
                    uint8_t distance = 
                        getBitsMSB(src,ptr,extraBitsDistance) + EXTRA_BITS_DISTANCE_OFFSET[distanceCode];
                    ptr += extraBitsDistance;
                    printf("Length %d Distance %d\n",length,distance);
                } else {
                    //literal
                    appendToBuffer(code,out,&allocatedOutput,outputLength);
                }
            }
        }
    } else {
        
    }

    //truncate memory garbage at the end
    *out = realloc(*out,*outputLength);

    return 1;
}