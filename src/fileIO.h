#pragma once
#include <stdlib.h>

int readFile(char filePath[],unsigned char** data,size_t* size);
int writeFile(char filePath[],unsigned char* data, size_t size);