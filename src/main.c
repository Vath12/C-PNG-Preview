#include <stdio.h>
#include <stdlib.h>
#include "fileIO.h"
#include "pngDecoder.h"
#include "util.h"
#include "inflate/decoder.h"
#include "SDL3/SDL.h"

int main(){

    if (!SDL_Init(SDL_INIT_VIDEO)){
        printf("SDL Init failed");
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow("PNG viewer",800,800,SDL_WINDOW_RESIZABLE);

    if (window == NULL){
        printf("Window Creation Failed!");
        return -2;
    }

    SDL_Surface *screen = SDL_GetWindowSurface(window);

    //uint8_t *fileData = NULL;
    //size_t size = 0;
    //readFile("../resources/testFile",&fileData,&size);
    //uint8_t *uncompressed = NULL;
    //size_t outputSize = 0;

    //printf("inflate exited with code %d\n",inflate(&uncompressed,&outputSize,fileData,size));
    //writeFile("../resources/testFileUncompressed.txt",uncompressed,outputSize);
    //free(uncompressed);
    //free(fileData);

    //uint8_t buffer[] = {0b00000001,0b00000000};
    //f_b(getBitsLSB(&buffer[0],0,16),16);
    //printf("\n");

    RGBA *image = NULL;
    uint16_t w,h = 0;

    printf("readPNG exited with code: %d\n",readPNG("../resources/image.png",&image,&w,&h));

    /*
    SDL_Surface *img = SDL_CreateSurface(w,h,SDL_PIXELFORMAT_RGBA8888);
    uint32_t *pixels = img->pixels;
    for (int y = 0; y < h; y++){
        for (int x = 0; x < h; x++){
            int i = (x+(y*w));
            pixels[i] = 
                (uint8_t) image[i].r << 24 |
                (uint8_t) image[i].g << 16 |
                (uint8_t) image[i].b << 8 |
                (uint8_t) image[i].a;
        }
    }
    */
    SDL_Surface *img = SDL_CreateSurface(w,h,SDL_PIXELFORMAT_RGBA8888);
    uint32_t *pixels = img->pixels;
    int i = 0;
    for (int y = 0; y < h; y++){
        for (int x = 0; x < h; x++){
            //printf("%d %d %d\n",image[i].r,image[i].g,image[i].b);
            pixels[i] = SDL_MapSurfaceRGB(img,image[i].r,image[i].g,image[i].b);
            i++;
        }
    }

    uint8_t run = 1;
    while (run){

        SDL_Event event;
        while (SDL_PollEvent(&event)){
            if (event.type == SDL_EVENT_QUIT){
                run = 0;
            }
        }
        SDL_Rect screenRect = {0,0,screen->w,screen->h};
        SDL_FillSurfaceRect(screen,&screenRect,(Uint32) -1);
        SDL_Rect dst = {0,0,800,1500};
        if (!SDL_BlitSurfaceScaled(img,NULL,screen,&dst,SDL_SCALEMODE_NEAREST)){
            printf("%s",SDL_GetError());
            return -1;
        }
        SDL_UpdateWindowSurface(window);
    }

    free(image);
    SDL_DestroySurface(img);
    SDL_DestroySurface(screen);
    SDL_Quit();
    
    return 0;

}   