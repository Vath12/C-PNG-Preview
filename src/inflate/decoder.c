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
    1,1,1,1,2,2,2,2, 
    3,3,3,3,4,4,4,4,
    5,5,5,5,0
};
const uint16_t EXTRA_BITS_LENGTH_OFFSET[] = {
    3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,
    35,43,51,59,67,83,99,115,131,163,195,227,258
};
//correspond to values 0-29
const uint8_t EXTRA_BITS_DISTANCE[] = {
    0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13
};
const uint32_t EXTRA_BITS_DISTANCE_OFFSET[] = {
    1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,
    1025,1537,2049,3073,4097,6145,8193,12289,16385,24557
};

/*
This lil fella's existence is brought to you by rfc 1951 pg 13
In block type 2 we've got the cl (code length) code 
encoding the code lengths of the prefix codes.
The cl code itself is a canonical prefix code encoding the symbols 0-19

Symbols 0-15 are literal values corresponding to code lengths that we pass
to the prefix code generator. 

Symbols 16-19 are for run length encoding.

Symbol 16: copy the previous code 3 + (value of next 3 bits) times

Symbol 17: write 0s 3 + (value of next 3 bits) times

Symbol 18: Write 0s 11 + (value of next 11 bits) times

What I still don't understand is why the specification has index 19 and 0
for a table with 18 symbols defined. I thought indexing might start at 1
for some reason (they had lua back then didn't they?) but 0 is also present.

Oh and if that couldn't get more confusing the assignment order is non-linear
and happens in an arbitrary order (defined by clCodeAssignmentOrder)

My best guess as to why that is is that it allows for the encoder to truncate
more of the cl code because codes with a length likely to be 0 are 
encoded at the end of the sequence. 

How they determined which codes are likeliest to be 0 in general
is beyond me but here we are.
*/
static const uint8_t clCodeAssignmentOrder[] = {
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};

typedef struct {
    uint16_t literal;
    uint8_t length;
    uint16_t code;
} prefixCode;

struct prefixAlphabet{
    prefixCode *code;
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
        printf("%llu",(value >> (numBits-i-1)) & 1);
    }
}


size_t ringBufferIndex(size_t current,long long offset,size_t size){
    if (offset < 0){
        offset = size+(offset%size);
    }
    return (current + offset) % size;
}

void ringBufferWrite(uint8_t value,uint8_t *ringBuffer,size_t *write,size_t size){
    ringBuffer[*write] = value;
    *write = ringBufferIndex(*write,1,size);
}

int comparePrefixCode(const void* A,const void* B){
    return ((prefixCode*) A)->length - ((prefixCode*) B)->length;
}

//see rfc 1951 pg 7
int generateCodes(struct prefixAlphabet *alphabet,uint16_t size){
    //count the number of distinct lengths present
    uint8_t maxLength = 0;
    for (int i = 0; i < size;i++){
        alphabet->code[i].literal = i;
        maxLength = max(maxLength,alphabet->code[i].length);
    }
    //allocate extra elem bc codes with length 0 are the 0th element
    uint16_t length_count[MAX_CODE_LENGTH] = {0};
    uint16_t next_code[MAX_CODE_LENGTH] = {0};
    //for length N, count the number of codes with length N
    for (int i = 0; i < size;i++){
        length_count[alphabet->code[i].length] += 1;
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
        uint16_t length = alphabet->code[i].length;
        if (length != 0){
            alphabet->code[i].code = next_code[length] & (((uint16_t) -1) >> (16-length));
            next_code[length] += 1;
        }
    }
    qsort(alphabet->code,size,sizeof(prefixCode),comparePrefixCode);
    return 1;
}

struct prefixAlphabet fixedLiteralLength = {NULL};
struct prefixAlphabet fixedDistance = {NULL};

int generateFixedCodes()
{
    printf("generate fixed codes\n");
    fixedLiteralLength.code = malloc(286 * sizeof(prefixCode));
    fixedDistance.code = malloc(30 * sizeof(prefixCode));
    //TODO: check that malloc did not fail and return code if it did

    for (int i = 0; i < 30;i++){
        fixedDistance.code[i].length = 5;
    }
    for (int i = 0; i < 286;i++){
        if (i < 144){
            fixedLiteralLength.code[i].length = 8;
        } else if (i < 256){
            fixedLiteralLength.code[i].length = 9;
        } else if (i < 280){
            fixedLiteralLength.code[i].length = 7;
        } else {
            fixedLiteralLength.code[i].length = 9;
        }
    }

    int success = generateCodes(&fixedLiteralLength, 286);
    if (!success){
        return success;
    }
    return generateCodes(&fixedDistance, 30);    
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
            if (length == alphabet->code[i].length && code == alphabet->code[i].code){
                return alphabet->code[i].literal;
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
    //this is separate from *outputLength because 
    //memory is re-allocated in blocks of 8129 as needed
    size_t allocatedOutput = *outputLength;
    uint8_t *slidingWindow = NULL;
    size_t slidingWindowWrite = 0;
    size_t slidingWindowSize = 0;

    uint8_t compressionMethod = getBitsLSB(src,ptr,4);
    uint8_t compressionInfo = getBitsLSB(src,ptr+4,4);
    uint8_t hasDict = getBitsLSB(src,ptr+13,1);
    uint8_t compressionLevel = getBitsLSB(src,ptr+14,2);
    uint16_t isValid = (((src[0] << 8) | src[1]) % 31) == 0;
    ptr+=16;

    slidingWindowSize = 1 << (compressionInfo+7);
    slidingWindow = malloc( slidingWindowSize );

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

    uint8_t isLast = getBitsLSB(src,ptr,1);
    uint8_t blockType = getBitsLSB(src,ptr+1,2);
    ptr += 3;

    printf("isLast : %d\n",isLast);
    printf("blockType : %d\n",blockType);
    
    if (blockType==3){
        return -3;//invalid blocktype
    }
    if (blockType == 0){
         //block is uncompressed, the implicit size limit is 65536 because the length is 2 bytes
        //rfc 1951 sec 3.2.4
        //align pointer to next byte boundary
        if (ptr % 8 != 0){
            ptr = ( (ptr/8) + 1) * 8;
        }
        uint16_t blockSize = getBitsLSB(src,ptr,16);
        uint16_t blockSizeCheck = ~((uint16_t) getBitsLSB(src,ptr,16));
        ptr+=16;
        if (blockSize != blockSizeCheck){
            return -4; //uncompressed block length was invalid
        }
        //weird and bad, FIX SOON
        allocatedOutput += blockSize;
        //*out = realloc(*out,allocatedOutput);
        //size_t writeBegin = (*out) + *outputLength;
        //size_t readBegin = &src[(ptr/8)];
        //memcpy(writeBegin,readBegin,blockSize);
        //*outputLength = *outputLength + blockSize;
        ptr += blockSize;
    }
    else{
        //block is compressed using prefix codes and LZSS
        //prefix alphabets point to the fixed codes because that is the default
        //TODO: ensure malloc actually allocates the requested memory and return code if not
        struct prefixAlphabet *literalLengthAlphabet = &fixedLiteralLength;
        struct prefixAlphabet *distanceAlphabet = &fixedDistance;

        if (blockType == 2){
            //block is compressed using dynamically specified prefix codes
            literalLengthAlphabet = malloc(sizeof(struct prefixAlphabet));
            distanceAlphabet = malloc(sizeof(struct prefixAlphabet));

            uint16_t numLiteralLengthCodes = getBitsLSB(src,ptr,5)+257;
            uint8_t numDistanceCodes= getBitsLSB(src,ptr+5,5)+1;
            uint8_t numCLCodes = getBitsLSB(src,ptr+10,4)+4;
            ptr += 14;

            struct prefixAlphabet clCodeAlphabet = {};
            clCodeAlphabet.code = calloc(19,sizeof(prefixCode));

            literalLengthAlphabet->code = calloc(286,sizeof(prefixCode));

            distanceAlphabet->code = calloc(30,sizeof(prefixCode));
            for (int i = 0; i < numCLCodes;i++){
                clCodeAlphabet.code[clCodeAssignmentOrder[i]].length = getBitsLSB(src,ptr,3);
                ptr+=3;
            }
            generateCodes(&clCodeAlphabet,19);

            uint8_t lastClCode = 0;
            int symbolCount = 0;

            const uint8_t extraBits[] = {2,3,7};
            const uint8_t extraBitOffset[] = {3,3,11};

            while (symbolCount < numLiteralLengthCodes+numDistanceCodes){
                uint8_t clCode = nextCode(src,&ptr,&clCodeAlphabet,19);
                if (clCode <= 15){
                    if (symbolCount < numLiteralLengthCodes){
                        literalLengthAlphabet->code[symbolCount].length = clCode;
                    } else {
                        distanceAlphabet->code[symbolCount-numLiteralLengthCodes].length = clCode;
                    }
                    lastClCode = clCode;

                    symbolCount++;
                } else {
                    uint8_t runLength = extraBitOffset[clCode-16]+getBitsLSB(src,ptr,extraBits[clCode-16]);
                    uint8_t runSymbol = clCode == 16 ? lastClCode : 0;
                    for (int i = 0; i < runLength;i++){
                        if (symbolCount < numLiteralLengthCodes){
                            literalLengthAlphabet->code[symbolCount].length = runSymbol;
                        } else {
                            distanceAlphabet->code[symbolCount-numLiteralLengthCodes].length = runSymbol;
                        }
                        symbolCount++;
                    }
                    ptr += extraBits[clCode-16];
                }
            }
            generateCodes(literalLengthAlphabet,286);
            generateCodes(distanceAlphabet,30);            
        }
        while (ptr/8 <= srcLength){
            uint16_t code = nextCode(src,&ptr,literalLengthAlphabet,286);
            if (code == END_OF_BLOCK){
                //EOB
                break;
            }
            else if (code > END_OF_BLOCK){
                //extra bits for length code
                uint8_t extraBitsLength = EXTRA_BITS_LENGTH[code-257];
                uint16_t length = getBitsLSB(src,ptr,extraBitsLength) + EXTRA_BITS_LENGTH_OFFSET[code-257]; 
                ptr += extraBitsLength;
                //get distance code
                uint16_t distanceCode = nextCode(src,&ptr,distanceAlphabet,30);
                //extra bits for distance code
                uint8_t extraBitsDistance = EXTRA_BITS_DISTANCE[distanceCode];
                uint32_t distance = getBitsLSB(src,ptr,extraBitsDistance) + EXTRA_BITS_DISTANCE_OFFSET[distanceCode];
                ptr += extraBitsDistance;
                //printf("l%d d%d l%d d%d\n",code,distanceCode,length,distance);

                //LZSS  backreferencing
                for (int i = 0; i < length;i++){
                    //repeat literal
                    size_t index = ringBufferIndex(slidingWindowWrite,-distance,slidingWindowSize);
                    uint8_t repeat = slidingWindow[index];
                    ringBufferWrite(repeat,slidingWindow,&slidingWindowWrite,slidingWindowSize);
                    appendToBuffer(repeat,out,&allocatedOutput,outputLength);
                }

            } else {
                //literal
                ringBufferWrite(code,slidingWindow,&slidingWindowWrite,slidingWindowSize);
                appendToBuffer(code,out,&allocatedOutput,outputLength);
            }
        }
        if (blockType == 2){
            //free dynamic prefix code tables
            free(literalLengthAlphabet->code);
            free(literalLengthAlphabet);

            free(distanceAlphabet->code);
            free(distanceAlphabet);
        }

    }

    //truncate memory garbage at the end
    *out = realloc(*out,*outputLength);

    free(fixedLiteralLength.code);
    free(fixedDistance.code);
    free(slidingWindow);

    return 1;
}