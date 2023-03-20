#include <iostream>
#define SDL_MAIN_HANDLED
#include <SDL.h>



int main() {
    SDL_SetMainReady();
    auto window = SDL_CreateWindow( "Okienko SDL",
                                    SDL_WINDOWPOS_UNDEFINED,
                                    SDL_WINDOWPOS_UNDEFINED,
                                    640, 480,
                                    SDL_WINDOW_SHOWN );


    SDL_Delay(1000);
    SDL_DestroyWindow( window );
    SDL_Quit();
    return 0;
}