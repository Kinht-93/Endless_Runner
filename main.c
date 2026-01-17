#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "map.h"
#include "score.h"

#define MAX_OBSTACLES 20
#define MAX_ENEMIES 20

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

typedef enum {
    MAIN_MENU,
    OPTIONS,
    SCORE_VIEW,
    GAME
} MenuState;

void render_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color) {
    if (!font) return;
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    if (!surface) return;
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }
    SDL_Rect rect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}





void draw_menu(SDL_Renderer *renderer, TTF_Font *font, int selected, Config *cfg) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {255, 255, 0, 255};
    const char *options[] = {"Lancer le jeu", "Options", "Score", "Quitter"};
    for (int i = 0; i < 4; i++) {
        SDL_Color color = (i == selected) ? yellow : white;
        render_text(renderer, font, options[i], 100, 100 + i * 50, color);
    }
    SDL_RenderPresent(renderer);
}

void draw_options(SDL_Renderer *renderer, TTF_Font *font, int selected, Config *cfg) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {255, 255, 0, 255};
    char buffer[256];
    sprintf(buffer, "Resolution: %dx%d", cfg->width, cfg->height);
    SDL_Color color = (selected == 0) ? yellow : white;
    render_text(renderer, font, buffer, 100, 100, color);
    sprintf(buffer, "Sound: %s", cfg->sound);
    color = (selected == 1) ? yellow : white;
    render_text(renderer, font, buffer, 100, 150, color);
    sprintf(buffer, "Username: %s", cfg->username);
    color = (selected == 2) ? yellow : white;
    render_text(renderer, font, buffer, 100, 200, color);
    sprintf(buffer, "Key Jump: %c", cfg->key_jump);
    color = (selected == 3) ? yellow : white;
    render_text(renderer, font, buffer, 100, 250, color);
    sprintf(buffer, "Key Quit: %c", cfg->key_quit);
    color = (selected == 4) ? yellow : white;
    render_text(renderer, font, buffer, 100, 300, color);
    sprintf(buffer, "Key Crouch: %c", cfg->key_crouch);
    color = (selected == 5) ? yellow : white;
    render_text(renderer, font, buffer, 100, 350, color);
    sprintf(buffer, "Key Reset: %c", cfg->key_reset);
    color = (selected == 6) ? yellow : white;
    render_text(renderer, font, buffer, 100, 400, color);
    render_text(renderer, font, "Appuyez sur Entree pour modifier, Echap pour retour", 100, 450, white);
    SDL_RenderPresent(renderer);
}

void draw_score(SDL_Renderer *renderer, TTF_Font *font, int highScore) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_Color white = {255, 255, 255, 255};
    char buffer[256];
    sprintf(buffer, "Meilleur Score: %d", highScore);
    render_text(renderer, font, buffer, 100, 100, white);
    render_text(renderer, font, "Appuyez sur une touche pour retour", 100, 150, white);
    SDL_RenderPresent(renderer);
}

int edit_option(Config *cfg, int selected, SDL_Renderer *renderer, TTF_Font *font) {
    if (selected == 1) {
        if (strcmp(cfg->sound, "ON") == 0) {
            strcpy(cfg->sound, "OFF");
        } else {
            strcpy(cfg->sound, "ON");
        }
        return 1;
    }
    if (selected == 2) {
        SDL_StartTextInput();
        char input[256] = "";
        int done = 0;
        SDL_Event event;
        while (!done) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_TEXTINPUT) {
                    strncat(input, event.text.text, sizeof(input) - strlen(input) - 1);
                } else if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_RETURN) {
                        strcpy(cfg->username, input);
                        done = 1;
                    } else if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(input) > 0) {
                        input[strlen(input)-1] = '\0';
                    }
                }
            }
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            SDL_Color white = {255, 255, 255, 255};
            render_text(renderer, font, "Entrez le nom d'utilisateur:", 100, 100, white);
            render_text(renderer, font, input, 100, 150, white);
            SDL_RenderPresent(renderer);
            SDL_Delay(10);
        }
        SDL_StopTextInput();
        return 1;
    }
    if (selected >= 3 && selected <= 6) {
        SDL_Event event;
        int done = 0;
        while (!done) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_KEYDOWN) {
                    char key = (char)event.key.keysym.sym;
                    if (key >= 32 && key <= 126) {
                        if (selected == 3) cfg->key_jump = key;
                        else if (selected == 4) cfg->key_quit = key;
                        else if (selected == 5) cfg->key_crouch = key;
                        else if (selected == 6) cfg->key_reset = key;
                        done = 1;
                    }
                }
            }
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            SDL_Color white = {255, 255, 255, 255};
            render_text(renderer, font, "Appuyez sur une touche pour changer", 100, 100, white);
            SDL_RenderPresent(renderer);
            SDL_Delay(10);
        }
        return 1;
    }
    if (selected == 0) {
        if (cfg->width == 800 && cfg->height == 600) {
            cfg->width = 1024; cfg->height = 768;
        } else if (cfg->width == 1024 && cfg->height == 768) {
            cfg->width = 1280; cfg->height = 720;
        } else if (cfg->width == 1280 && cfg->height == 720) {
            cfg->width = 1920; cfg->height = 1080;
        } else {
            cfg->width = 800; cfg->height = 600;
        }
        return 1;
    }
    return 0;
}






int main(void) {
    Config cfg = load_config();
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erreur SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() < 0) {
        printf("Erreur TTF_Init: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_Window* menuWindow = SDL_CreateWindow("Endless Runner Menu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    if (!menuWindow) {
        printf("Erreur SDL_CreateWindow: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    SDL_Renderer* menuRenderer = SDL_CreateRenderer(menuWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!menuRenderer) {
        printf("Erreur SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(menuWindow);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    TTF_Font *font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
    if (!font) {
        printf("Erreur TTF_OpenFont: %s\n", TTF_GetError());
        printf("Le texte ne sera pas affiché.\n");
    }
    MenuState state = MAIN_MENU;
    int selected = 0;
    int menu_running = 1;
    SDL_Event menu_event;





    while (menu_running) {
        while (SDL_PollEvent(&menu_event)) {
            if (menu_event.type == SDL_QUIT) {
                menu_running = 0;
            } else if (menu_event.type == SDL_KEYDOWN) {
                if (state == MAIN_MENU) {
                    if (menu_event.key.keysym.sym == SDLK_UP) {
                        selected = (selected - 1 + 4) % 4;
                    } else if (menu_event.key.keysym.sym == SDLK_DOWN) {
                        selected = (selected + 1) % 4;
                    } else if (menu_event.key.keysym.sym == SDLK_RETURN) {
                        if (selected == 0) {
                            state = GAME;
                            menu_running = 0;
                        } else if (selected == 1) {
                            state = OPTIONS;
                            selected = 0;
                        } else if (selected == 2) {
                            state = SCORE_VIEW;
                        } else if (selected == 3) {
                            menu_running = 0;
                        }
                    }
                } else if (state == OPTIONS) {
                    if (menu_event.key.keysym.sym == SDLK_UP) {
                        selected = (selected - 1 + 7) % 7;
                    } else if (menu_event.key.keysym.sym == SDLK_DOWN) {
                        selected = (selected + 1) % 7;
                    } else if (menu_event.key.keysym.sym == SDLK_RETURN) {
                        if (edit_option(&cfg, selected, menuRenderer, font)) {
                            save_config(&cfg);
                        }
                    } else if (menu_event.key.keysym.sym == SDLK_ESCAPE) {
                        state = MAIN_MENU;
                        selected = 0;
                    }
                } else if (state == SCORE_VIEW) {
                    state = MAIN_MENU;
                    selected = 0;
                }
            }
        }
        if (state == MAIN_MENU) {
            draw_menu(menuRenderer, font, selected, &cfg);
        } else if (state == OPTIONS) {
            draw_options(menuRenderer, font, selected, &cfg);
        } else if (state == SCORE_VIEW) {
            ScoreSystem dummy;
            init_score(&dummy);
            load_high_score(&dummy);
            draw_score(menuRenderer, font, dummy.highScore);
        }
        SDL_Delay(16);
    }
    SDL_DestroyRenderer(menuRenderer);
    SDL_DestroyWindow(menuWindow);
    if (state != GAME) {
        if (font) TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return 0;
    }





    
    cfg = load_config();
    if (strcmp(cfg.sound, "ON") == 0) {
        SDL_InitSubSystem(SDL_INIT_AUDIO);
    }
    SDL_Window* window = SDL_CreateWindow(
        "Endless_Runner"
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        cfg.width,
        cfg.height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    if (!window) {
        printf("Erreur SDL_CreateWindow: %s\n", SDL_GetError());
        if (font) TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Erreur SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        if (font) TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return 1;
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
                else if (event.key.keysym.sym == get_key_from_char(cfg.key_jump) && player.isJumping && !player.DoubleJump && !gameOver) {
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
            player.velX = 0;
            if (leftPressed) player.velX -= PLAYER_SPEED;
            if (rightPressed) player.velX += PLAYER_SPEED;
            
            float newX = player.x + player.velX;
            
            int horizontalCollision = 0;
            SDL_Rect futurePlayerRect = {(int)newX, (int)player.y, player.width, player.height};
            
            for (int i = 0; i < MAX_OBSTACLES; i++) {
                if (gameMap.obstacles[i].active) {
                    SDL_Rect obstacleRect = {(int)gameMap.obstacles[i].x, (int)gameMap.obstacles[i].y,
                                              gameMap.obstacles[i].width, gameMap.obstacles[i].height};
                    
                    if (SDL_HasIntersection(&futurePlayerRect, &obstacleRect)) {
                        float obstacleTop = gameMap.obstacles[i].y;
                        float playerBottom = player.y + player.height;
                        
                        if (playerBottom > obstacleTop + 1) {
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
            
            if (player.x < 0) {
                player_die(&player);
                init_map(&gameMap, cfg.width, cfg.height);
                scoreSys.currentScore = 0;
            }
            if (player.x + player.width > cfg.width) player.x = cfg.width - player.width;






            player.velY += GRAVITY;
            float newY = player.y + player.velY;
            
            int onGround = 0;
            SDL_Rect futurePlayerRectY = {(int)player.x, (int)newY, player.width, player.height};
            
            if (newY + player.height >= GROUND_Y + 50) {
                player.y = GROUND_Y + 50 - player.height;
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
            
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (gameMap.enemies[i].active) {
                    SDL_Rect playerRect = {(int)player.x, (int)player.y, player.width, player.height};
                    SDL_Rect enemyRect = {(int)gameMap.enemies[i].x, (int)gameMap.enemies[i].y, 
                                           gameMap.enemies[i].width, gameMap.enemies[i].height};
                    
                    if (SDL_HasIntersection(&playerRect, &enemyRect)) {
                        player_die(&player);
                        init_map(&gameMap, cfg.width, cfg.height);
                        scoreSys.currentScore = 0;
                    }
                }
            }
        }





        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_Rect ground = {0, GROUND_Y + 50, cfg.width, cfg.height - GROUND_Y - 50};
        SDL_RenderFillRect(renderer, &ground);

        if (!gameOver) {
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
        
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (gameMap.enemies[i].active) {
                SDL_Rect enemyRect = {(int)gameMap.enemies[i].x, (int)gameMap.enemies[i].y, 
                                       gameMap.enemies[i].width, gameMap.enemies[i].height};
                SDL_RenderFillRect(renderer, &enemyRect);
            }
        }

        char scoreText[64];
        sprintf(scoreText, "Score: %d", scoreSys.currentScore);
        SDL_Color white = {255, 255, 255, 255};
        render_text(renderer, font, scoreText, 10, 10, white);

        sprintf(scoreText, "High Score: %d", scoreSys.highScore);
        render_text(renderer, font, scoreText, 10, 40, white);

        render_text(renderer, font, cfg.username, (int)player.x - 20, (int)player.y - 30, white);
        
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
