#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "map.h"
#include "score.h"

#define MAX_OBSTACLES 20

typedef struct {
    float x, y;
    float velX, velY;
    int width, height;
    int isJumping;
    int DoubleJump;
    int Crouching;
    int baseHeight, baseWidth;
    int life;
} Player;

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

void render_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color);

void reset_game(Player* player) {
    player->x = 100;
    player->y = GROUND_Y;
    player->velX = 0;
    player->velY = 0;
    player->isJumping = 0;
    player->DoubleJump = 0;
    player->Crouching = 0;
    player->height = player->baseHeight;
    player->width = player->baseWidth;
    player->life = 3;
}

void player_die(Player *p) {
    p->life--;
    if (p->life <= 0) {
        reset_game(p);
    } else {
        p->x = 100;
        p->y = GROUND_Y;
        p->velX = 0;
        p->velY = 0;
        p->isJumping = 0;
        p->DoubleJump = 0;
        p->Crouching = 0;
        p->height = p->baseHeight;
        p->width = p->baseWidth;
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

    if (TTF_Init() < 0) {
        printf("Erreur TTF_Init: %s\n", TTF_GetError());
        SDL_Quit();
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

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer) {
        printf("Erreur SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    TTF_Font *font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
    if (!font) {
        printf("Erreur TTF_OpenFont: %s\n", TTF_GetError());
        printf("Le texte ne sera pas affiché.\n");
    }

    Player player = {
        .x = 100,
        .y = GROUND_Y,
        .velX = 0,
        .velY = 0,
        .width = 50,
        .height = 50,
        .isJumping = 0,
        .DoubleJump = 0,
        .Crouching = 0,
        .baseHeight = 50,
        .baseWidth = 50,
        .life = 3
    };

    Map gameMap;
    init_map(&gameMap, cfg.width, cfg.height);

    ScoreSystem scoreSys;
    init_score(&scoreSys);

    int gameOver = 0;
    int leftPressed = 0, rightPressed = 0;
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

                if (event.key.keysym.sym == get_key_from_char(cfg.key_jump) && player.isJumping && !player.DoubleJump && !gameOver) {
                    player.velY = JUMP_FORCE;
                    player.DoubleJump = 1;
                }

                if (event.key.keysym.sym == get_key_from_char(cfg.key_crouch) && !gameOver) {
                    if (!player.Crouching) {
                        player.Crouching = 1;
                        player.height = player.baseHeight / 2;
                        player.y += player.baseHeight / 2;
                    } else {
                        player.Crouching = 0;
                        player.height = player.baseHeight;
                        player.y -= player.baseHeight / 2;
                    }
                }

                if (event.key.keysym.sym == SDLK_r) {
                    reset_game(&player);
                    init_map(&gameMap, cfg.width, cfg.height);
                    scoreSys.currentScore = 0;
                    gameOver = 0;
                }

                if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_q) {
                    leftPressed = 1;
                }
                if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d) {
                    rightPressed = 1;
                }
            }

            if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_q) {
                    leftPressed = 0;
                }
                if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d) {
                    rightPressed = 0;
                }
            }
        }

        if (!gameOver) {
            // Mouvement horizontal AVEC collision intelligente
            player.velX = 0;
            if (leftPressed) player.velX -= PLAYER_SPEED;
            if (rightPressed) player.velX += PLAYER_SPEED;
            
            float newX = player.x + player.velX;
            
            // Collision horizontale uniquement si on est au même niveau
            int horizontalCollision = 0;
            SDL_Rect futurePlayerRect = {(int)newX, (int)player.y, player.width, player.height};
            
            for (int i = 0; i < MAX_OBSTACLES; i++) {
                if (gameMap.obstacles[i].active) {
                    SDL_Rect obstacleRect = {(int)gameMap.obstacles[i].x, (int)gameMap.obstacles[i].y,
                                              gameMap.obstacles[i].width, gameMap.obstacles[i].height};
                    
                    if (SDL_HasIntersection(&futurePlayerRect, &obstacleRect)) {
                        float obstacleTop = gameMap.obstacles[i].y;
                        float playerBottom = player.y + player.height;
                        
                        // Si le joueur est DANS l'obstacle verticalement (pas en train de sauter par-dessus)
                        if (playerBottom > obstacleTop + 5) {
                            horizontalCollision = 1;
                            
                            float playerCenter = player.x + player.width / 2.0f;
                            float obstacleCenter = gameMap.obstacles[i].x + gameMap.obstacles[i].width / 2.0f;
                            
                            if (playerCenter < obstacleCenter) {
                                player.x = gameMap.obstacles[i].x - player.width - 1;
                            } else {
                                player.x = gameMap.obstacles[i].x + gameMap.obstacles[i].width + 1;
                            }
                            break;
                        }
                    }
                }
            }
            
            if (!horizontalCollision) {
                player.x = newX;
            }
            
            if (player.x < 0) player.x = 0;
            if (player.x + player.width > cfg.width) player.x = cfg.width - player.width;

            // Gravité et collision verticale
            player.velY += GRAVITY;
            float newY = player.y + player.velY;
            
            int onGround = 0;
            SDL_Rect futurePlayerRectY = {(int)player.x, (int)newY, player.width, player.height};
            
            if (newY >= GROUND_Y) {
                player.y = GROUND_Y;
                player.velY = 0;
                player.isJumping = 0;
                player.DoubleJump = 0;
                onGround = 1;
            } else {
                int landed = 0;
                for (int i = 0; i < MAX_OBSTACLES; i++) {
                    if (gameMap.obstacles[i].active) {
                        SDL_Rect obstacleRect = {(int)gameMap.obstacles[i].x, (int)gameMap.obstacles[i].y,
                                                  gameMap.obstacles[i].width, gameMap.obstacles[i].height};
                        
                        if (player.velY > 0 && SDL_HasIntersection(&futurePlayerRectY, &obstacleRect)) {
                            if (player.y + player.height <= gameMap.obstacles[i].y + 5) {
                                player.y = gameMap.obstacles[i].y - player.height;
                                player.velY = 0;
                                player.isJumping = 0;
                                player.DoubleJump = 0;
                                landed = 1;
                                onGround = 1;
                                break;
                            }
                        }
                        
                        if (player.velY < 0 && SDL_HasIntersection(&futurePlayerRectY, &obstacleRect)) {
                            if (player.y >= gameMap.obstacles[i].y + gameMap.obstacles[i].height - 5) {
                                player.y = gameMap.obstacles[i].y + gameMap.obstacles[i].height;
                                player.velY = 0;
                                break;
                            }
                        }
                    }
                }
                
                if (!landed) {
                    player.y = newY;
                }
            }

            if (player.y > cfg.height + 200) {
                player_die(&player);
                init_map(&gameMap, cfg.width, cfg.height);
                scoreSys.currentScore = 0;
            }
            
            update_map(&gameMap, cfg.width, cfg.height);
            update_score(&scoreSys, (int)gameMap.speed);
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
        for (int i = 0; i < MAX_OBSTACLES; i++) {
            if (gameMap.obstacles[i].active) {
                SDL_Rect obstacleRect = {(int)gameMap.obstacles[i].x, (int)gameMap.obstacles[i].y, 
                                          gameMap.obstacles[i].width, gameMap.obstacles[i].height};
                SDL_RenderFillRect(renderer, &obstacleRect);
            }
        }

        char scoreText[64];
        sprintf(scoreText, "Score: %d", scoreSys.currentScore);
        SDL_Color white = {255, 255, 255, 255};
        render_text(renderer, font, scoreText, 10, 10, white);

        sprintf(scoreText, "High Score: %d", scoreSys.highScore);
        render_text(renderer, font, scoreText, 10, 40, white);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    if (font) TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void render_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color) {
    if (!font) return;

    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    if (!surface) return;

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect dst = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dst);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}