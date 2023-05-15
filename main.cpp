#include <iostream>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <memory>
#include <array>
#include <cmath>
#include <string>


const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
bool menu = true;
bool player_fly = false;
bool game_over = false;
bool fly = false;
int score = 0;
int fire_time = 0;
TTF_Font* font;
SDL_Color textColor = { 255, 255, 255, 255 };
Mix_Chunk* asteroid_explosion;
Mix_Chunk* player_explosion;
Mix_Chunk* shoot;
Mix_Chunk* space_sound;
Mix_Chunk* fire;


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

vec2d operator+(vec2d a, int b){
    return {a[0]+b,a[1]+b};
}

vec2d operator-(vec2d a, vec2d b){
    return {a[0]-b[0],a[1]-b[1]};
}

vec2d operator/(vec2d a, int i){
    return {a[0]/i,a[1]/i};
}

vec2d operator*(vec2d a, int i){
    return {a[0]*i,a[1]*i};
}

//klasy obiektów
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
double asteroid_angle(asteroid_c &asteroid){
    double angle;
    double euclides_distance;
    double height_distance;
    int x1, x2, y1, y2;
    x1 = asteroid.position[0];
    y1 = asteroid.position[1];
    x2 = -340;
    y2 = -340;
    height_distance = y2-y1;
    euclides_distance = sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
    angle = asin(height_distance/euclides_distance);
    return angle;
}

//generatoor koordynatów asteroid
vec2d asteroid_spawn_XY (){
    vec2d coos;

    coos[0] = random(-800, 60);
    coos[1] = random(0, 3);
    std::cout<<coos[0]<<" "<<coos[1]<<"\n";
    return coos;
}

//stworzenie i sprawdzenie koordynatów
vec2d asteroid_spawn(){
     vec2d coos = asteroid_spawn_XY();
     int wall = coos[1];
         switch(wall){
             case 0:
                 return {120, coos[0]};
             case 1:
                 return {coos[0], 120};
             case 2:
                 return {-860, coos[0]};
             case 3:
                 return {coos[0], -860};
         }
}


//ustawienie kąta lotu asteroid
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

void asteroidDESTROY(asteroid_c &asteroid){
    std::cout<<"HIT"<<"\n";
    score++;
    //wybuchy
    //dźwięki
    Mix_PlayChannel(-1, asteroid_explosion, 0);

}

//reset pocisku
void bullet_reset(bullet_c &bullet, player_c &player){
    fly = false;
    bullet.position = player.position+(-18);
}

//pocisk poza plansza
void bullet_colision(bullet_c &bullet, player_c &player){
    if ((bullet.position[0] > 18 || bullet.position[1] > 18) ||
        (bullet.position[0] < -782 || bullet.position[1] < -782)){
        bullet_reset(bullet, player);
    }
}

//reset asteroidy
void asteroid_reset(asteroid_c &asteroid){
    asteroid.position=asteroid_spawn();
    asteroid.angle= asteroid_angle_set(asteroid);
}

//asteroida poza plansza
void asteroid_collision(asteroid_c &asteroid, bullet_c &bullet,SDL_Rect &a_rect, SDL_Rect &b_rect, player_c &player){
    if((asteroid.position[0] > 120 || asteroid.position[1] > 120) ||
       (asteroid.position[0] < -900 || asteroid.position[1] < -900)){
        std::cout<<"POZA"<<"\n";
        asteroid_reset(asteroid);
    }
    if(SDL_HasIntersection(&a_rect, &b_rect)){
        bullet_reset(bullet, player);
        asteroid_reset(asteroid);
        asteroidDESTROY(asteroid);}
}

void gameOVER(asteroid_c &asteroid1){
    game_over = true;
    asteroid_reset(asteroid1);
    Mix_PlayChannel(-1, player_explosion, 0);
}


void player_collision(SDL_Rect &a_rect, SDL_Rect &p_rect, asteroid_c asteroid){
    if(SDL_HasIntersection(&a_rect, &p_rect))
        gameOVER(asteroid);;
}

void playerFLY(player_c &player, vec2d forward){
    player.position = player.position - forward/2;
}

void player_abroad(player_c &player){
    if(player.position[0] > 48){
        player.position[0] = -800;
    }
    if(player.position[0] < -800){
        player.position[0] = 48;
    }
    if(player.position[1] > 48){
        player.position[1] = -800;
    }
    if(player.position[1] < -800){
        player.position[1] = 48;
    }
}

void play_the_game(SDL_Renderer *renderer){
    //ładowanie tekstury gracza
    auto player_texture = load_texture(renderer, "ship.bmp");

    //ładowanie tekstury ognia
    auto fire_texture = load_texture(renderer, "fire.bmp");

    //ładowanie tekstury asteroidy
    auto asteroid_texture = load_texture(renderer, "asteroid.bmp");

    //ładowanie tekstury pocisku
    auto bullet_texture = load_texture(renderer, "bullet.bmp");

    //ładowanie tekstury menu
    auto menu_texture = load_texture(renderer, "menu.bmp");

    //ładowanie tekstury tła
    auto background_texture = load_texture(renderer, "background.bmp");

    //ładowanie tekstury game over
    auto game_over_texture = load_texture(renderer, "game_over.bmp");

    //obiekt gracza
    SDL_Rect player_rect = get_texture_rect(player_texture);

    //obiekt asteroid
    SDL_Rect asteroid_rect = get_texture_rect(asteroid_texture);

    //obiakt pocisku
    SDL_Rect bullet_rect = get_texture_rect(bullet_texture);

    //inicjacja gracza
    player_c player = {M_PI*1.5,{-352,-352}};


    //inicjacja asteroid
    int asteroid_count = 4;
    asteroid_c asteroid[4];
    for (int i = 0; i < asteroid_count; i++){
        asteroid[i] = {0,asteroid_spawn()};
    }

    //inicjacja pocisku
    bullet_c bullet = {M_PI*1.5, {-382,-382}};

    //inicjacja czcionek
    TTF_Init();

    //inicjacja mixera
    Mix_Init(MIX_INIT_MP3);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    //ładowanie dźwięków

    asteroid_explosion = Mix_LoadWAV("sounds/asteroid_explosion.mp3");
    player_explosion = Mix_LoadWAV("sounds/player_explosion.mp3");
    shoot = Mix_LoadWAV("sounds/shoot.mp3");
    space_sound = Mix_LoadWAV("sounds/space_sound.mp3");
    fire = Mix_LoadWAV("sounds/fire.mp3");

    vec2d forward = angle_to_vector(player.angle);
    bool gaming = true;
    Mix_PlayChannel(0, space_sound, -1);
    while(gaming){

        if(menu){
            auto *keyboard_state = SDL_GetKeyboardState(nullptr);
            SDL_Event e;
            while (SDL_PollEvent(&e) != 0) {
                switch (e.type) {
                    case SDL_QUIT:
                        gaming = false;
                        break;
                    case SDL_KEYDOWN:
                        if (e.key.keysym.sym == SDLK_q)
                            gaming = false;
                        break;
                }
            }

            if (keyboard_state[SDL_SCANCODE_SPACE]){
                menu = false;
                SDL_Delay(100);
            }
            SDL_RenderCopy(renderer, menu_texture.get(), nullptr, nullptr);

        }


        if(!menu) {

            font = TTF_OpenFont("font/PixelIntv.ttf", 40);

            SDL_Event e;
            while (SDL_PollEvent(&e) != 0) {
                switch (e.type) {
                    case SDL_QUIT:
                        gaming = false;
                        break;
                    case SDL_KEYDOWN:
                        if (e.key.keysym.sym == SDLK_q) gaming = false;
                        break;
                }
            }
            auto *keyboard_state = SDL_GetKeyboardState(nullptr);

            //strzelanie
            if (!fly && !game_over) {
                if (keyboard_state[SDL_SCANCODE_SPACE]) {
                    bullet.angle = player.angle;
                    fly = true;
                    Mix_PlayChannel(-1, shoot, 0);
                }
            }

            if (fly && !game_over) {
                vec2d forward_vec_b = angle_to_vector(bullet.angle);
                bullet.position = bullet.position - forward_vec_b * 3;
            }



            //kolizje pocisku
            if(fly) bullet_colision(bullet, player);

            //skręcanie
            if (keyboard_state[SDL_SCANCODE_LEFT]) player.angle -= M_PI / 75.0;
            if (keyboard_state[SDL_SCANCODE_RIGHT]) player.angle += M_PI / 75.0;

            //odpalenie silnika
            if (keyboard_state[SDL_SCANCODE_UP] && fire_time == 0) {
                fire_time = 50;
                player_fly = 1;
                forward = angle_to_vector(player.angle);
                Mix_PlayChannel(-1, fire, 0);
            }

            //if (keyboard_state[SDL_SCANCODE_DOWN]) player_fly = 0;


            if (!game_over) {

                //poruszanie sie asteroid
                for (int i = 0; i < asteroid_count; i++){
                    vec2d forward_vec_a1 = angle_to_vector(asteroid[i].angle);
                    asteroid[i].position = asteroid[i].position - forward_vec_a1 / 2;
                }

                //poruszanie się gracza
                if (player_fly){
                    playerFLY(player, forward);
                    player_abroad(player);
                }

                if(!fly){
                    bullet.position = player.position + -(28);
                }

            }



            auto p_rect = player_rect;
            p_rect.x -= player.position[0] - p_rect.w / 2;
            p_rect.y -= player.position[1] - p_rect.h / 2;

            auto b_rect = bullet_rect;
            b_rect.x -= bullet.position[0] - b_rect.w / 2;
            b_rect.y -= bullet.position[1] - b_rect.h / 2;

            //kolizje gracz - asteroida
            for (int i = 0; i < asteroid_count; i++){
                auto a_rect = asteroid_rect;
                a_rect.x -= asteroid[i].position[0] - a_rect.w / 2;
                a_rect.y -= asteroid[i].position[1] - a_rect.h / 2;
                player_collision(a_rect, p_rect, asteroid[i]);
            }

            //kolizje pocisk - asteroida
            if(fly){
                for (int i = 0; i < asteroid_count; i++){
                    auto a_rect = asteroid_rect;
                    a_rect.x -= asteroid[i].position[0] - a_rect.w / 2;
                    a_rect.y -= asteroid[i].position[1] - a_rect.h / 2;
                    asteroid_collision(asteroid[i], bullet, a_rect, b_rect, player);
                }
            }

            //wymiana punktów
            if (score >= 10) {
                if (keyboard_state[SDL_SCANCODE_X]) {
                    score = score-(10 + asteroid_count);
                    SDL_Delay(100);
                    for (int i = 0; i < asteroid_count; i++){
                        asteroid_reset(asteroid[i]);
                        asteroidDESTROY(asteroid[i]);

                    }
                }
            }
            std::string scoreStr = std::to_string(score);

            if (!game_over) {
                // render kosmosu
                SDL_RenderCopy(renderer, background_texture.get(), nullptr, nullptr);

                SDL_RenderCopyEx(renderer, bullet_texture.get(),
                                 nullptr, &b_rect, 180 * bullet.angle / M_PI,
                                 nullptr, SDL_FLIP_NONE);

                SDL_RenderCopyEx(renderer, player_texture.get(),
                                 nullptr, &p_rect, 180 * player.angle / M_PI,
                                 nullptr, SDL_FLIP_NONE);

                if (fire_time > 0){
                    SDL_RenderCopyEx(renderer, fire_texture.get(),
                                     nullptr, &p_rect, 180 * player.angle / M_PI,
                                     nullptr, SDL_FLIP_NONE);
                    fire_time--;

                }


                for (int i = 0; i < asteroid_count; i++){
                    auto a1_rect = asteroid_rect;
                    a1_rect.x -= asteroid[i].position[0] - a1_rect.w / 2;
                    a1_rect.y -= asteroid[i].position[1] - a1_rect.h / 2;
                    SDL_RenderCopyEx(renderer, asteroid_texture.get(),
                                     nullptr, &a1_rect, 180 * asteroid[i].angle / M_PI,
                                     nullptr, SDL_FLIP_NONE);
                }


                SDL_Surface *scoreSurface = TTF_RenderText_Solid(font, scoreStr.c_str(), textColor);
                SDL_Texture *scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);

                SDL_Rect scoreRect = {(WINDOW_WIDTH / 2) - 10, 10, scoreSurface->w, scoreSurface->h};

                SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);

            } else if (game_over) {

                SDL_RenderCopy(renderer, game_over_texture.get(), nullptr, nullptr);
                std::string gameOverStr = "GAME OVER";

                std::string yourScore = "YOUR SCORE: ";

                SDL_Surface *gameOverSurface = TTF_RenderText_Solid(font, gameOverStr.c_str(), textColor);
                SDL_Texture *gameOverTexture = SDL_CreateTextureFromSurface(renderer, gameOverSurface);
                SDL_Rect gameOverRect = {(WINDOW_WIDTH / 2) - 122, 260, gameOverSurface->w, gameOverSurface->h};
                SDL_RenderCopy(renderer, gameOverTexture, NULL, &gameOverRect);

                SDL_Surface *yourScoreSurface = TTF_RenderText_Solid(font, yourScore.c_str(), textColor);
                SDL_Texture *yourScoreTexture = SDL_CreateTextureFromSurface(renderer, yourScoreSurface);
                SDL_Rect yourScoreRect = {(WINDOW_WIDTH / 2) - 162, 360, yourScoreSurface->w, yourScoreSurface->h};
                SDL_RenderCopy(renderer, yourScoreTexture, NULL, &yourScoreRect);

                SDL_Surface *scoreSurface = TTF_RenderText_Solid(font, scoreStr.c_str(), textColor);
                SDL_Texture *scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
                SDL_Rect scoreRect = {(WINDOW_WIDTH / 2) + 137, 360, scoreSurface->w, scoreSurface->h};
                SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
            }
        }

        SDL_RenderPresent(renderer);
        SDL_RenderClear(renderer);
        SDL_Delay(10);
    }
}

vec2d angle_to_vector(double angle) {
    return {std::cos(angle), sin(angle)};
}

int main() {

    srand((unsigned)time(NULL));
    SDL_Init(SDL_INIT_EVERYTHING);

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