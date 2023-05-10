#include <iostream>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <memory>
#include <array>
#include <cmath>

//screen
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;

//asteroid
const int PADDLE_WIDTH = 20;
const int PADDLE_HEIGHT = 100;


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

//pobieranie wymiarów z wymiarów tekstury
std::pair<int,int> get_texture_w_h(std::shared_ptr<SDL_Texture> texture) {
    int w, h;
    SDL_QueryTexture(texture.get(), nullptr, nullptr,
                     &w, &h);
    return {w, h};
}
// pobieranie wymiarów obiektu z wymiarów tekstury
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

class asteroid_c {
public:
    double angle;
    vec2d position;
};

class bullet_c {
public:
    double angle;
    vec2d position;
};

vec2d angle_to_vector(double angle);

//generator liczb losowych na scope (X-Y)
double random(int min, int max){
    if (min > max){
        int tmp = max;
        max = min;
        min = tmp;
    }
    int range = max - min + 1;
    double num = rand() % range + min;
    return num;
}

//obliczanie kierunku asteroidy względem położenia gracza
double asteroid_angle(asteroid_c asteroid){
    double angle;
    double euclides_distance;
    double height_distance;
    int x1, x2, y1, y2;
    x1 = asteroid.position[0];
    y1 = asteroid.position[1];
    x2 = -400;
    y2 = -400;
    height_distance = y2-y1;
    euclides_distance = sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
    //angle = height_distance/euclides_distance;

    angle = asin(height_distance/euclides_distance);
    return angle;
}

//generatoor koordynatów asteroid
vec2d asteroid_spawn_XY (){
    vec2d position;
    int x = -random(-900, 100);
    int y = -random(-900, 100);
    position[0] = x;
    position[1] = y;
    return position;
}

//przekazanie gotowych koordynatów, sprawdzanie czy pasują
vec2d asteroid_spawn(){
    vec2d position = asteroid_spawn_XY();

    std::cout<<position[0]<<" "<<position[1]<<"\n";

    bool is_position_OK = 0;
    while(!is_position_OK){
        if((position[1] > -000)
           || (position[0] < -800)
           || (position[0] > -00)
           || (position[1] < -800)){
            is_position_OK = 1;
            std::cout<<position[0]<<" "<<position[1]<<"\n";
        }
        position = asteroid_spawn_XY();
    }
    return position;
}

double asteroid_angle_set(asteroid_c asteroid){
    if(asteroid.position[1] > -400 && asteroid.position[0] > -400){
        return -asteroid_angle(asteroid);
    }
    if(asteroid.position[1] < -400 && asteroid.position[0] > -400){
        return 2*M_PI-asteroid_angle(asteroid);
    }
    if(asteroid.position[1] < -400 && asteroid.position[0] < -400){
        return M_PI+asteroid_angle(asteroid);
    }
    if(asteroid.position[1] > -400 && asteroid.position[0] < -400){
        return M_PI+asteroid_angle(asteroid);
    }
}


void play_the_game(SDL_Renderer *renderer){
    //ładowanie tekstury gracza
    auto player_texture = load_texture(renderer, "ship.bmp");

    //ładowanie tekstury asteroidy
    auto asteroid_texture = load_texture(renderer, "asteroid.bmp");

    //ładowanie tekstury pocisku
    auto bullet_texture = load_texture(renderer, "bullet.bmp");

    //prostokat/wielkosc obiektu gracza
    SDL_Rect player_rect = get_texture_rect(player_texture);

    //obiekt asteroid
    SDL_Rect asteroid_rect = get_texture_rect(asteroid_texture);

    //obiakt pocisku
    SDL_Rect bullet_rect = get_texture_rect(bullet_texture);


    player_c player = {M_PI*1.5,{-400,-400}};
    asteroid_c asteroid1 = {0, asteroid_spawn()};

    std::cout<<asteroid_angle(asteroid1);
    asteroid1.angle = (asteroid_angle_set(asteroid1));



    //asteroid_c asteroid1 = {0,asteroid_spawn()};
    asteroid_c asteroid2 = {0,asteroid_spawn()};
    asteroid_c asteroid3 = {0,asteroid_spawn()};
    bullet_c bullet = {0, {-352,-352.0}};

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

        //STRZELA
        if (keyboard_state[SDL_SCANCODE_UP]) {
            //vec2d forward_vec = angle_to_vector(player.angle);
            //player.position = player.position - forward_vec;
        }

        if (keyboard_state[SDL_SCANCODE_DOWN]) {
            //vec2d forward_vec = angle_to_vector(player.angle);
            //player.position = player.position + forward_vec;
        }
        if (keyboard_state[SDL_SCANCODE_LEFT]) player.angle-=M_PI/75.0;
        if (keyboard_state[SDL_SCANCODE_RIGHT]) player.angle+=M_PI/75.0;




        vec2d forward_vec = angle_to_vector(asteroid1.angle);
        //asteroid1.angle = -M_PI/asteroid_angle(asteroid1,player);

        asteroid1.position = asteroid1.position - forward_vec;

        SDL_SetRenderDrawColor(renderer,10,40,128,255);
        //SDL_RenderDrawLine(renderer, 0,0, 640,240);
        //SDL_RenderCopy(renderer, player_texture.get(), nullptr, &player_rect);

        auto a_rect = asteroid_rect;
        auto p_rect = player_rect;

        p_rect.x -= player.position[0] - p_rect.w / 2;
        p_rect.y -= player.position[1] - p_rect.h / 2;

        a_rect.x -= asteroid1.position[0] - p_rect.w / 2;
        a_rect.y -= asteroid1.position[1] - p_rect.h / 2;


        std::cout<<asteroid_angle(asteroid1);

        // render gracza
        SDL_RenderCopyEx(renderer, player_texture.get(),
                         nullptr, &p_rect, 180 * player.angle / M_PI,
                         nullptr, SDL_FLIP_NONE);

        // render asteroidy
        SDL_RenderCopyEx(renderer, asteroid_texture.get(),
                         nullptr, &a_rect, 180 * asteroid1.angle / M_PI,
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
/*
    int help = 0;
    while (help <10){
        help++;
        vec2d astra = asteroid_spawn();
        std::cout<<astra[0]<< " "<<astra[1]<<"\n";
    }

 */

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT,
                                SDL_WINDOW_SHOWN,
                                &window, &renderer);

    play_the_game(renderer);
    SDL_DestroyWindow( window );
    SDL_Quit();
    return 0;



}