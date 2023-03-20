#include <iostream>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <memory>
#include <array>
#include <cmath>

std::shared_ptr<SDL_Texture> load_texture(SDL_Renderer *renderer, std::string fname) {
    SDL_Surface *bmp = SDL_LoadBMP(("images/" + fname).c_str());
    if (!bmp) {
        throw std::invalid_argument("Could not load bmp");
    }

    SDL_SetColorKey(bmp, SDL_TRUE, 0x0ff00ff); //przezroczystoć na ten 255.0.255

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, bmp);
    if (!texture) {
        throw std::invalid_argument("Could not create texture");
    }
    SDL_FreeSurface(bmp);
    return std::shared_ptr<SDL_Texture>(texture, [](SDL_Texture *tex) {
        std::cout << "Texture destroyed!" << std::endl;
        SDL_DestroyTexture(tex);
    });
}

std::pair<int,int> get_texture_w_h(std::shared_ptr<SDL_Texture> texture) {
    int w, h;
    SDL_QueryTexture(texture.get(), nullptr, nullptr,
                     &w, &h);
    return {w, h};
}

SDL_Rect get_texture_rect(std::shared_ptr<SDL_Texture> texture){
    auto [w,h] = get_texture_w_h(texture);
    return {0, 0, w, h};
}

using vec2d = std::array<double,2>;

vec2d operator+(vec2d a, vec2d b){
    return {a[0]+b[0],a[1]+b[1]};
}

vec2d operator-(vec2d a, vec2d b){
    return {a[0]-b[0],a[1]-b[1]};
}

class player_c {
public:
    double angle;
    vec2d position;
};

vec2d angle_to_vector(double angle);

void play_the_game(SDL_Renderer *renderer){
    auto player_texture = load_texture(renderer, "boat.bmp");
    SDL_Rect player_rect = get_texture_rect(player_texture);
    player_c player = {0,{-320.0,-240.0}};

    bool gaming = true;
    while(gaming){
        SDL_Event e;

        while(SDL_PollEvent(&e) != 0){
            switch (e.type) {
                case SDL_QUIT:
                    gaming = false;
                    break;
                case SDL_KEYDOWN:
                    if(e.key.keysym.sym == SDLK_q) gaming = false;
                    break;
            }
        }

        auto *keyboard_state =  SDL_GetKeyboardState(nullptr);
        if (keyboard_state[SDL_SCANCODE_UP]) {
            vec2d forward_vec = angle_to_vector(player.angle);
            player.position = player.position - forward_vec;
        }
        if (keyboard_state[SDL_SCANCODE_DOWN]) {
            vec2d forward_vec = angle_to_vector(player.angle);
            player.position = player.position + forward_vec;
        }
        if (keyboard_state[SDL_SCANCODE_LEFT]) player.angle-=M_PI/100.0;
        if (keyboard_state[SDL_SCANCODE_RIGHT]) player.angle+=M_PI/100.0;



        SDL_SetRenderDrawColor(renderer,10,40,128,255);
        //SDL_RenderDrawLine(renderer, 0,0, 640,240);
        //SDL_RenderCopy(renderer, player_texture.get(), nullptr, &player_rect);


        auto rect = player_rect;

        rect.x -= player.position[0] - rect.w/2;
        rect.y -= player.position[1] - rect.h/2;

        SDL_RenderCopyEx(renderer, player_texture.get(),
                         nullptr,&rect, 180*player.angle/M_PI,
                         nullptr, SDL_FLIP_NONE);

        SDL_RenderPresent(renderer);
        SDL_RenderClear(renderer);
        SDL_Delay(10);
    }
}

vec2d angle_to_vector(double angle) {
    return {std::cos(angle), sin(angle)};
}

int main() {
    SDL_Init(SDL_INIT_EVERYTHING);


    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_CreateWindowAndRenderer(640, 480,
                                SDL_WINDOW_SHOWN,
                                &window, &renderer);

    play_the_game(renderer);
    SDL_DestroyWindow( window );
    SDL_Quit();
    return 0;



}