#pragma once
#include "pngChunks.h"

int readPNG(char path[],RGBA **image,uint16_t *width, uint16_t *height);