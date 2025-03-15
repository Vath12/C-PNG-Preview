#pragma once
#include <stdlib.h>

/*
conforms to the zlib data format used in PNGs
it is an LZSS implementation
details here: 

http://www.libpng.org/pub/png/book/chapter09.html#png.ch09.div.2

https://www.hjp.at/doc/rfc/rfc1950.pdf

https://www.rfc-editor.org/rfc/rfc1951

from rfc 1951 (5):

*   Data elements are packed into bytes in order of
    increasing bit number within the byte, i.e., starting
    with the least-significant bit of the byte.
*   Data elements other than Huffman codes are packed
    starting with the least-significant bit of the data
    element.
*   Huffman codes are packed starting with the most-
    significant bit of the code.


from rfc 1951 sec 3.2.3:

Each block of compressed data begins with 3 header bits
containing the following data:

first bit       BFINAL
next 2 bits     BTYPE

Note that the header bits do not necessarily begin on a byte
boundary, since a block does not necessarily occupy an integral
number of bytes.

BFINAL is set if and only if this is the last block of the data set.

BTYPE specifies how the data are compressed, as follows:

00 - no compression
01 - compressed with fixed Huffman codes
10 - compressed with dynamic Huffman codes
11 - reserved (error)
*/


int inflate(uint8_t* stream,size_t size,uint8_t** output);