#include "decoder.h"
#include "../util.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const int MAX_CODE_LENGTH = 15;
const uint16_t END_OF_BLOCK = 256;
/*
The EXTRA_BITS_XX and EXTRA_BITS_XX_OFFSET define the behavior listed 
in rfc section 3.2.5 (page 11). 

LZSS length distance pairs are encoded using two alphabets

The length is specified by literals 257-285 which are encoded
in the primary length/literal prefix code alphabet.

After a length come a certain number of "extra bits"
these are defined in the array EXTRA_BITS_LENGTH 
where the index corresponds to the literal value - 257

The value of these extra bits is then offset by an amount
specified by EXTRA_BITS_DISTANCE_OFFSET, 
where the index corresponds to the literal value - 257

Following this is a prefix code from the distance code alphabet
These are values 0-29. The same process of reading extra bits 
and offsetting them is the same for the distance as for the length.
The only difference is that you use the distance code to index 
the EXTRA_BITS_DISTANCE and EXTRA_BITS_DISTANCE_OFFSET tables.
*/
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
    1025,1537,2049,3073,4097,6145,8193,12289,16385,24577
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

size_t ringBufferIndex(size_t current,int64_t offset,size_t size){
    if (offset < 0){
        offset = size+(offset % size);
    }
    return (current + offset) % size;
}

void ringBufferWrite(uint8_t value,uint8_t *ringBuffer,size_t *write,size_t size){
    ringBuffer[*write] = value;
    *write = ringBufferIndex(*write,1,size);
}
//used to qsort prefix codes
int comparePrefixCode(const void* A,const void* B){
    return ((prefixCode*) A)->length - ((prefixCode*) B)->length;
}
/*
see rfc 1951 pg 7

This algorithm produces canonical prefix codes 
given code lengths specified by the input alphabet. 
the alphabet's literals are set to their indices so 
you shouldn't set them prior to generating codes
*/
int generateCodes(struct prefixAlphabet *alphabet,uint16_t size){
    uint16_t length_count[MAX_CODE_LENGTH] = {0};
    uint16_t next_code[MAX_CODE_LENGTH] = {0};
    //for length N, count the number of codes with length N
    for (int i = 0; i <= size;i++){
        alphabet->code[i].literal = i;
        length_count[alphabet->code[i].length] += 1;
    }
    //initialize codes
    uint16_t code = 0;
    length_count[0] = 0;
    for (int i = 2; i <= MAX_CODE_LENGTH;i++){
        //code = (next_code[i-1] + length_count[i-1]) << 1;
        //next_code[i] = code;
        next_code[i] = (next_code[i-1] + length_count[i-1]) << 1;
    }
    uint16_t lastL = 0;
    //assign codes to all literals
    for (uint16_t i = 0; i < size;i++){
        //only assign codes with nonzero length
        uint16_t length = alphabet->code[i].length;
        lastL = length;
        if (length != 0){
            alphabet->code[i].code = next_code[length];
            next_code[length] += 1;
        }
        
    }
    return 1;
}

struct prefixAlphabet fixedLiteralLength = {NULL};
struct prefixAlphabet fixedDistance = {NULL};
/*
DEFLATE uses predefined code lengths to 
generate prefix codes for block type 1
*/
int generateFixedCodes()
{
    printf("generate fixed codes\n");
    fixedLiteralLength.code = malloc(287 * sizeof(prefixCode));
    fixedDistance.code = malloc(30 * sizeof(prefixCode));
    //TODO: check that malloc did not fail and return code if it did

    for (int i = 0; i < 30;i++){
        fixedDistance.code[i].length = 5;
    }
    for (int i = 0; i <= 287;i++){
        if (i <= 143){
            fixedLiteralLength.code[i].length = 8;
        } else if (i <= 255){
            fixedLiteralLength.code[i].length = 9;
        } else if (i <= 279){
            fixedLiteralLength.code[i].length = 7;
        } else {
            fixedLiteralLength.code[i].length = 8;
        }
    }
    
    int success = generateCodes(&fixedLiteralLength, 287);
    if (!success){
        return success;
    }
    return generateCodes(&fixedDistance, 30);    
}
/*
Read the next prefix code from the given
bitstream and increment the given pointer by
the length of the code
*/
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
    (*buffer)[*length] = value;
    *length = *length+1;
}

int inflate(uint8_t **out,size_t *outputLength,uint8_t *src,size_t srcLength){
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

    slidingWindowSize = 1 << (compressionInfo+8);
    slidingWindow = malloc( slidingWindowSize );

    printf("Compression Method: %d\n",compressionMethod);
    printf("Compression Info: %d\n",compressionInfo);
    printf("Compression Uses Dict: %d\n",hasDict);
    printf("Compression Level: %d\n",compressionLevel);
    printf("Valid: %d\n",isValid);

    uint8_t color = 0;

    if (!isValid){
        return -2; //invalid stream header
    }

    if (!generateFixedCodes()){
        return -3; //fixed code generation failed
    }
    /*
    for (int i = 0; i < 287;i++){
        if (fixedLiteralLength.code[i].length == 0){
            continue;
        }
        printf("%d ",fixedLiteralLength.code[i].literal);
        f_b(fixedLiteralLength.code[i].code,fixedLiteralLength.code[i].length);
        printf("\n");
    }
    */
    uint8_t isLast = 0;
    while (!isLast){
        isLast = getBitsLSB(src,ptr,1);
        uint8_t blockType = getBitsLSB(src,ptr+1,2);
        ptr += 3;

        //printf("isLast : %d\n",isLast);
        //printf("blockType : %d\n",blockType);

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
            //read data MSB first
            for (int i = 0; i < blockSize;i++){
                uint8_t byte = getBitsMSB(src,ptr,8);
                appendToBuffer(byte,out,allocatedOutput,outputLength);
                ptr+=8;
            }
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

                    //appendToBuffer(27,out,&allocatedOutput,outputLength);
                    //appendToBuffer('[',out,&allocatedOutput,outputLength);
                    //appendToBuffer('3',out,&allocatedOutput,outputLength);
                    //appendToBuffer(49+color++,out,&allocatedOutput,outputLength);
                    //color %= 5;
                    //appendToBuffer(';',out,&allocatedOutput,outputLength);
                    //appendToBuffer('4',out,&allocatedOutput,outputLength);
                    //appendToBuffer('m',out,&allocatedOutput,outputLength);

                    //LZSS  backreferencing
                    for (int i = 0; i < length;i++){
                        //repeat literal
                        size_t index = ringBufferIndex(slidingWindowWrite,-distance,slidingWindowSize);
                        uint8_t repeat = slidingWindow[index];
                        ringBufferWrite(repeat,slidingWindow,&slidingWindowWrite,slidingWindowSize);
                        appendToBuffer(repeat,out,&allocatedOutput,outputLength);
                    }
                    //appendToBuffer(27,out,&allocatedOutput,outputLength);
                    //appendToBuffer('[',out,&allocatedOutput,outputLength);
                    //appendToBuffer('0',out,&allocatedOutput,outputLength);
                    //appendToBuffer('m',out,&allocatedOutput,outputLength);

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
    }

    //truncate memory garbage at the end
    *out = realloc(*out,*outputLength);

    free(fixedLiteralLength.code);
    free(fixedDistance.code);
    free(slidingWindow);

    return 1;
}