#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

typedef struct {
    float x, y;
    float velY;
    int width, height;
    int isJumping;
} Player;

typedef struct {
    float x, y;
    int width, height;
    int active;
} Obstacle;

#define GRAVITY 0.8f
#define JUMP_FORCE -15.0f
#define GROUND_Y 500
#define PLAYER_SPEED 5.0f

SDL_Keycode get_key_from_char(char c) {
    if (c == ' ') return SDLK_SPACE;
    if (c == 'w' || c == 'W') return SDLK_w;
    if (c == 'a' || c == 'A') return SDLK_a;
    if (c == 's' || c == 'S') return SDLK_s;
    if (c == 'd' || c == 'D') return SDLK_d;
    if (c == 'q' || c == 'Q') return SDLK_q;
    return SDLK_SPACE;
}

int main(void) {
    Config cfg = load_config();

    SDL_SetHint(SDL_HINT_VIDEODRIVER, "x11");

    
    int sdl_flags = SDL_INIT_VIDEO;
    if (strcmp(cfg.sound, "ON") == 0) {
        sdl_flags |= SDL_INIT_AUDIO;
    }
    
    if (SDL_Init(sdl_flags) < 0) {
        printf("Erreur SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        cfg.username,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        cfg.width,
        cfg.height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        printf("Erreur SDL_CreateWindow: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    if (!renderer) {
        printf("Erreur SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }


    Player player = {
        .x = 100,
        .y = GROUND_Y,
        .velY = 0,
        .width = 50,
        .height = 50,
        .isJumping = 0
    };

    Obstacle obstacles[5];
    for (int i = 0; i < 5; i++) {
        obstacles[i].x = cfg.width + i * 300;
        obstacles[i].y = GROUND_Y;
        obstacles[i].width = 40;
        obstacles[i].height = 60;
        obstacles[i].active = 1;
    }

    int gameOver = 0;


    SDL_Event event;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                printf("SDL_QUIT reçu\n");
                running = 0;
            }
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) {
                printf("SDL_WINDOWEVENT_CLOSE reçu\n");
                running = 0;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = 0;
                }
                if (event.key.keysym.sym == get_key_from_char(cfg.key_quit)) {
                    running = 0;
                }
                if (event.key.keysym.sym == get_key_from_char(cfg.key_jump) && !player.isJumping && !gameOver) {
                    player.velY = JUMP_FORCE;
                    player.isJumping = 1;
                }
            }
        }

        if (!gameOver) {
            player.velY += GRAVITY;
            player.y += player.velY;
            
            if (player.y >= GROUND_Y) {
                player.y = GROUND_Y;
                player.velY = 0;
                player.isJumping = 0;
            }
            
            for (int i = 0; i < 5; i++) {
                if (obstacles[i].active) {
                    obstacles[i].x -= PLAYER_SPEED;
                    
                    if (obstacles[i].x + obstacles[i].width < 0) {
                        obstacles[i].x = cfg.width;
                    }
                    
                    SDL_Rect playerRect = {(int)player.x, (int)player.y, player.width, player.height};
                    SDL_Rect obstacleRect = {(int)obstacles[i].x, (int)obstacles[i].y, obstacles[i].width, obstacles[i].height};
                    
                    if (SDL_HasIntersection(&playerRect, &obstacleRect)) {
                        gameOver = 1;
                        printf("Game Over!\n");
                    }
                }
            }
        }

        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_Rect ground = {0, GROUND_Y + player.height, cfg.width, cfg.height - GROUND_Y - player.height};
        SDL_RenderFillRect(renderer, &ground);

        if (gameOver) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); 
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); 
        }
        SDL_Rect playerRect = {(int)player.x, (int)player.y, player.width, player.height};
        SDL_RenderFillRect(renderer, &playerRect);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int i = 0; i < 5; i++) {
            if (obstacles[i].active) {
                SDL_Rect obstacleRect = {(int)obstacles[i].x, (int)obstacles[i].y, obstacles[i].width, obstacles[i].height};
                SDL_RenderFillRect(renderer, &obstacleRect);
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}