#pragma once
#include <stdlib.h>

int readFile(char filePath[],char** data,size_t* size);
int fileWrite(char filePath[],char* data, size_t size);