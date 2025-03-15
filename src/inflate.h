#pragma once
#include <stdlib.h>

/*
conforms to the zlib data format used in PNGs
it is an LZSS implementation
details here: 
http://www.libpng.org/pub/png/book/chapter09.html#png.ch09.div.2
https://www.hjp.at/doc/rfc/rfc1950.pdf
*/
int inflate(char* stream,size_t size,char** output);