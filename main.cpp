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
bool fly = false;
const int ASTEROID_WIDTH = 60;


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

vec2d operator/(vec2d a, int i){
    return {a[0]/i,a[1]/i};
}

vec2d operator*(vec2d a, int i){
    return {a[0]*i,a[1]*i};
}

// KLASY OBIEKTÓW
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
    x2 = -340;
    y2 = -340;
    height_distance = y2-y1;
    euclides_distance = sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
    //angle = height_distance/euclides_distance;

    angle = asin(height_distance/euclides_distance);
    return angle;
}

//generatoor koordynatów asteroid
vec2d asteroid_spawn_XY (){
    vec2d position;
    int x = random(-800, 0);
    int y = random(-800, 0);
    position[0] = x;
    position[1] = y;
    return position;
}

//stworzenie i sprawdzenie koordynatów
vec2d asteroid_spawn(){
    vec2d position = asteroid_spawn_XY();


    bool is_position_OK = 0;
    while(true){
        if(   (position[0] > -25)
           || (position[0] < -775)
           || (position[1] > -25)
           || (position[1] < -775)){
            return position;
        }
        else position = asteroid_spawn_XY();
    }

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

void asteroidDESTROY(asteroid_c &asteroid){
    std::cout<<"HIT"<<"\n";

}


    //RESET POCISKU
void bullet_reset(bullet_c &bullet){
    fly = false;
    bullet.position = {-382,-382};
}

void bullet_colision(bullet_c &bullet){
    //POZA PLANSZE
    if ((bullet.position[0] > 18 || bullet.position[1] > 18) ||
        (bullet.position[0] < -782 || bullet.position[1] < -782)){
        bullet_reset(bullet);
    }
}

    //RESET ASTEROIDY
void asteroid_reset(asteroid_c &asteroid){
    asteroid.position=asteroid_spawn();
    asteroid.angle= asteroid_angle_set(asteroid);
}

    //POZA PLANSZE
void asteroid_collision(asteroid_c &asteroid){
    if((asteroid.position[0] > 60 || asteroid.position[1] > 60) ||
       (asteroid.position[0] < -800 || asteroid.position[1] < -800)){
        asteroid_reset(asteroid);
    }
}

void asteroidHIT(asteroid_c &asteroid, bullet_c &bullet){
    //PRZYPADEK 1 ĆWIARTKA
    if(asteroid.position[0]>=-400 && asteroid.position[1]>=-400){
        if(bullet.position[0]>asteroid.position[0]-60)
            if(bullet.position[0]<asteroid.position[0])
                if(bullet.position[1]>asteroid.position[1]-60)
                    if(bullet.position[1]<asteroid.position[1]){
                        bullet_reset(bullet);
                        asteroid_reset(asteroid);
                        asteroidDESTROY(asteroid);}
    }

    //PRZYPADEK 2 ĆWIARTKA
    if(asteroid.position[0]<=-400 && asteroid.position[1]>=-400){
        if(bullet.position[0]+24>asteroid.position[0]-60)
            if(bullet.position[0]+24<asteroid.position[0])
                if(bullet.position[1]>asteroid.position[1]-60)
                    if(bullet.position[1]<asteroid.position[1]){
                        bullet_reset(bullet);
                        asteroid_reset(asteroid);
                        asteroidDESTROY(asteroid);}
    }

    //PRZYPADEK 3 ĆWIARTKA
    if(asteroid.position[0]<=-400 && asteroid.position[1]<=-400){
        if(bullet.position[0]+24<asteroid.position[0])
            if(bullet.position[0]+24>asteroid.position[0]-60)
                if(bullet.position[1]+24<asteroid.position[1])
                    if(bullet.position[1]+24>asteroid.position[1]-60){
                        bullet_reset(bullet);
                        asteroid_reset(asteroid);
                        asteroidDESTROY(asteroid);}
    }

    //PRZYPADEK 4 ĆWIARTKA
    if(asteroid.position[0]>=-400 && asteroid.position[1]<=-400){
        if(bullet.position[0]>asteroid.position[0]-60)
            if(bullet.position[0]<asteroid.position[0])
                if(bullet.position[1]<asteroid.position[1])
                    if(bullet.position[1]>asteroid.position[1]-60){
                        bullet_reset(bullet);
                        asteroid_reset(asteroid);
                        asteroidDESTROY(asteroid);}
    }
}




void play_the_game(SDL_Renderer *renderer){
    //ładowanie tekstury gracza
    auto player_texture = load_texture(renderer, "ship.bmp");

    //ładowanie tekstury asteroidy
    auto asteroid_texture = load_texture(renderer, "asteroid.bmp");

    //ładowanie tekstury pocisku
    auto bullet_texture = load_texture(renderer, "bullet.bmp");

    //ładowanie tekstury tła
    auto background_texture = load_texture(renderer, "background.bmp");

    //obiekt gracza
    SDL_Rect player_rect = get_texture_rect(player_texture);

    //obiekt asteroid
    SDL_Rect asteroid_rect = get_texture_rect(asteroid_texture);

    //obiakt pocisku
    SDL_Rect bullet_rect = get_texture_rect(bullet_texture);

    //inicjacja gracza
    player_c player = {M_PI*1.5,{-352,-352}};

    //inicjacja asteroid
    asteroid_c asteroid1 = {0, {-10,-100 }};
    asteroid1.angle = (asteroid_angle_set(asteroid1));
    asteroid_c asteroid2 = {0,{-100,-700}};
    asteroid2.angle = (asteroid_angle_set(asteroid2));
    asteroid_c asteroid3 = {0,asteroid_spawn()};
    asteroid3.angle = (asteroid_angle_set(asteroid3));

    //inicjacja pocisku
    bullet_c bullet = {M_PI*1.5, {-382,-382}};

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




        // STRZELANIE
        if (!fly) {
            if (keyboard_state[SDL_SCANCODE_SPACE]) {
                bullet.angle = player.angle;
                fly = true;
            }
        }

        if (fly){
            vec2d forward_vec_b = angle_to_vector(bullet.angle);
            bullet.position = bullet.position - forward_vec_b*3;
        }


        // KOLIZJE ASTEROID
        asteroid_collision(asteroid1);
        asteroid_collision(asteroid2);
        asteroid_collision(asteroid3);

        // KOLIZJE POCISKU
        bullet_colision(bullet);

        // TRAFIENIE ASTEROIDY
        asteroidHIT(asteroid1, bullet);
        asteroidHIT(asteroid2, bullet);
        asteroidHIT(asteroid3, bullet);




        if (keyboard_state[SDL_SCANCODE_LEFT]) player.angle-=M_PI/75.0;
        if (keyboard_state[SDL_SCANCODE_RIGHT]) player.angle+=M_PI/75.0;




        //poruszanie się asteroid
        vec2d forward_vec_a1 = angle_to_vector(asteroid1.angle);
        asteroid1.position = asteroid1.position - forward_vec_a1/2;

        vec2d forward_vec_a2 = angle_to_vector(asteroid2.angle);
        asteroid2.position = asteroid2.position - forward_vec_a2/2;

        vec2d forward_vec_a3 = angle_to_vector(asteroid3.angle);
        asteroid3.position = asteroid3.position - forward_vec_a3/2;


        //SDL_SetRenderDrawColor(renderer,10,40,128,255);





        //środek obiektów
        auto p_rect = player_rect;
        p_rect.x -= player.position[0] - p_rect.w / 2;
        p_rect.y -= player.position[1] - p_rect.h / 2;

        auto a1_rect = asteroid_rect;
        a1_rect.x -= asteroid1.position[0] - a1_rect.w / 2;
        a1_rect.y -= asteroid1.position[1] - a1_rect.h / 2;

        auto a2_rect = asteroid_rect;
        a2_rect.x -= asteroid2.position[0] - a2_rect.w / 2;
        a2_rect.y -= asteroid2.position[1] - a2_rect.h / 2;

        auto a3_rect = asteroid_rect;
        a3_rect.x -= asteroid3.position[0] - a3_rect.w / 2;
        a3_rect.y -= asteroid3.position[1] - a3_rect.h / 2;

        auto b_rect = bullet_rect;
        b_rect.x -= bullet.position[0] - b_rect.w / 2;
        b_rect.y -= bullet.position[1] - b_rect.h / 2;


        // render kosmosu
        SDL_RenderCopy(renderer, background_texture.get(), nullptr, nullptr);

        // render gracza
        SDL_RenderCopyEx(renderer, player_texture.get(),
                         nullptr, &p_rect, 180 * player.angle / M_PI,
                         nullptr, SDL_FLIP_NONE);

        // render asteroidy
        SDL_RenderCopyEx(renderer, asteroid_texture.get(),
                         nullptr, &a1_rect, 180 * asteroid1.angle / M_PI,
                         nullptr, SDL_FLIP_NONE);

        SDL_RenderCopyEx(renderer, asteroid_texture.get(),
                         nullptr, &a2_rect, 180 * asteroid2.angle / M_PI,
                         nullptr, SDL_FLIP_NONE);

        SDL_RenderCopyEx(renderer, asteroid_texture.get(),
                         nullptr, &a3_rect, 180 * asteroid3.angle / M_PI,
                         nullptr, SDL_FLIP_NONE);


        // render pocisku
        SDL_RenderCopyEx(renderer, bullet_texture.get(),
                         nullptr, &b_rect, 180 * bullet.angle / M_PI,
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

    srand((unsigned)time(NULL));
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