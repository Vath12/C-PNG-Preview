#pragma once
#include <stdlib.h>

int readFile(char filePath[],char** data,size_t* size);
int writeFile(char filePath[],char* data, size_t size);