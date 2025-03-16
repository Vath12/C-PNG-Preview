#pragma once
#include <stdlib.h>

/*
Implementation assumes the zlib data format used in PNGs
http://www.libpng.org/pub/png/book/chapter09.html#png.ch09.div.2

If you would like to understand how the compressed data header is structured
read rfc 1950:
https://www.hjp.at/doc/rfc/rfc1950.pdf


If you would like to understand how the inflate implementation
works please read rfc 1951:
https://www.rfc-editor.org/rfc/rfc1951
*/

/*
get bit n from a bitstream
reads bytes MSB first!
*/
uint8_t getBit(uint8_t *buffer,uint64_t position);

int inflate(uint8_t *buffer,size_t size,uint8_t **output);