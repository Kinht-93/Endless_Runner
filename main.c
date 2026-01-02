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
    int DoubleJump;
    int Crouching;
    int baseHeight;basewidth;
    int life;
} Player;

#define GRAVITY 0.8f
#define JUMP_FORCE -15.0f
#define GROUND_Y 500

SDL_Keycode get_key_from_char(char c) {
    if (c == ' ') return SDLK_SPACE;
    if (c == 'q' || c == 'Q') return SDLK_q;
    return SDLK_SPACE;
}

void resetgame(Player* player) {
    player->x = 100;
    player->y = GROUND_Y;
    player->velY = 0;
    player->isJumping = 0;
    player->DoubleJump = 0;
    player->Crouching = 0;
    player->height = player->baseHeight;
    player->width = player->basewidth;
    player->life = 3;
}

void playerDie(Player *p) {
    p->lives--;

    if (p->lives <= 0) {
        resetGame(p);
    } else {
        p->x = 100;
        p->y = GROUND_Y;
        p->velY = 0;
        p->isJumping = 0;
        p->jumpCount = 0;
        p->isCrouching = 0;
    }
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
        .DoubleJump = 0
        .Crouching = 0
        .baseHeight = 50
        .basewidth = 50
        .life = 3
    };

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
                if (event.key.keysym.sym == get_key_from_char(cfg.key_jump) && !player.isJumping) {
                    player.velY = JUMP_FORCE;
                    player.isJumping = 1;
                }
                if(event.key.keysym.sym == get_key_from_char(cfg.key_jump) && player.isJumping && !player.DoubleJump) {
                    player.velY = JUMP_FORCE;
                    player.DoubleJump = 1;
                }
                if (event.key.keysym.sym == get_key_from_char(cfg.key_crouch)) {
                    player.Crouching = 1;
                    player.height = player.baseHeight / 2;
                    player.y += player.baseHeight / 2;
                }
                if(event.key.keysym.sym == get_key_from_char(cfg.key_crouch) && player.Crouching) {
                    player.Crouching = 0;
                    player.height = player.baseHeight;
                    player.y -= player.baseHeight / 2;
                }
                if(event.key.keysym.sym == get_key_from_char(cfg.key_reset)) {
                    resetgame(&player);
                }
           }

        }

        player.velY += GRAVITY;
        player.y += player.velY;
        
        if (player.y >= GROUND_Y) {
            player.y = GROUND_Y;
            player.velY = 0;
            player.isJumping = 0;
            player.DoubleJump = 0;
        }

        if (player.y > cfg.height + 200) {
            playerDie(&player);
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_Rect ground = {0, GROUND_Y + player.height, cfg.width, cfg.height - GROUND_Y - player.height};
        SDL_RenderFillRect(renderer, &ground);

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_Rect playerRect = {(int)player.x, (int)player.y, player.width, player.height};
        SDL_RenderFillRect(renderer, &playerRect);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}