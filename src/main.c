#include <stdio.h>
#include <stdlib.h>
#include "fileIO.h"
#include "pngDecoder.h"
#include "util.h"
#include "inflate/decoder.h"
#include "SDL3/SDL.h"

int main(int argc, char *argv[]){

    if (!SDL_Init(SDL_INIT_VIDEO)){
        printf("SDL Init failed");
        return -1;
    }

    RGBA *image = NULL;
    uint16_t w,h = 0;
    if (argc <= 1){
        printf("readPNG exited with code: %d\n",readPNG("../resources/image.png",&image,&w,&h));
    } else {
        printf("reading: %s\n",argv[1]);
        printf("readPNG exited with code: %d\n",readPNG(argv[1],&image,&w,&h));
    }

    SDL_Window *window = SDL_CreateWindow("PNG viewer",w,h,SDL_WINDOW_RESIZABLE);

    if (window == NULL){
        printf("Window Creation Failed!");
        return -2;
    }

    SDL_Surface *screen = SDL_GetWindowSurface(window);
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
        SDL_Rect dst = {0,0,800,800};
        //SDL_BlitSurfaceScaled(img,NULL,screen,&dst,SDL_SCALEMODE_NEAREST);
        SDL_BlitSurface(img,NULL,screen,&dst);
        SDL_UpdateWindowSurface(window);
    }

    free(image);
    SDL_DestroySurface(img);
    SDL_DestroySurface(screen);
    SDL_Quit();
    
    return 0;

}   