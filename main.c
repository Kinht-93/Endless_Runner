// MOHAMMADINE ANAS
// gcc main.c -o endless_runner -lSDL2

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int height;
    int width;
    char sound[4];
    char username[256];
} Config;

// Ajoute ces structures après Config
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

// Constantes
#define GRAVITY 0.8f
#define JUMP_FORCE -15.0f
#define GROUND_Y 500
#define PLAYER_SPEED 5.0f

void set_defaults(Config *c) {
    c->height = 600;
    c->width  = 800;
    strcpy(c->sound, "ON");
    strcpy(c->username, "anaslebg");
}

char *trim(char *s) {
    char *end;
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') s++;
    if (*s == 0) return s;
    end = s + strlen(s) - 1;
    while (end > s && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r'))
        *end-- = '\0';
    return s;
}

int str_equal_ignorecase(const char *a, const char *b) {
    while (*a && *b) {
        char c1 = (*a >= 'a' && *a <= 'z') ? *a - 32 : *a;
        char c2 = (*b >= 'a' && *b <= 'z') ? *b - 32 : *b;
        if (c1 != c2) return 0;
        a++; b++;
    }
    return *a == *b;
}

void strtoupper(char *s) {
    while (*s) {
        if (*s >= 'a' && *s <= 'z') *s -= 32;
        s++;
    }
}

int main(void) {
    const char *path = "config.ini";
    Config cfg;
    set_defaults(&cfg);

    FILE *f = fopen(path, "r");
    if (!f) {
        printf("Aucun fichier trouvé -> valeurs par défaut utilisées.\n");
    } else {
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            char *ln = trim(line);
            if (*ln == 0 || *ln == '#' || *ln == ';') continue;
            char *eq = strchr(ln, '=');
            if (!eq) continue;
            *eq = '\0';
            char *key = trim(ln);
            char *val = trim(eq + 1);

            if (str_equal_ignorecase(key, "height")) {
                int v = atoi(val);
                if (v > 0) cfg.height = v;
            }
            else if (str_equal_ignorecase(key, "width")) {
                int v = atoi(val);
                if (v > 0) cfg.width = v;
            }
            else if (str_equal_ignorecase(key, "sound")) {
                strtoupper(val);
                if (strcmp(val, "ON") == 0 || strcmp(val, "OFF") == 0)
                    strcpy(cfg.sound, val);
            }
            else if (str_equal_ignorecase(key, "username")) {
                if (*val) {
                    strncpy(cfg.username, val, sizeof(cfg.username)-1);
                    cfg.username[sizeof(cfg.username)-1] = '\0';
                }
            }
        }
        fclose(f);
    }

    if (cfg.height <= 0) cfg.height = 600;
    if (cfg.width  <= 0) cfg.width  = 800;
    strtoupper(cfg.sound);
    if (strcmp(cfg.sound, "ON") != 0 && strcmp(cfg.sound, "OFF") != 0)
        strcpy(cfg.sound, "ON");
    if (cfg.username[0] == '\0')
        strcpy(cfg.username, "Player");

    FILE *fw = fopen(path, "w");
    if (fw) {
        fprintf(fw, "height=%d\n", cfg.height);
        fprintf(fw, "width=%d\n", cfg.width);
        fprintf(fw, "sound=%s\n", cfg.sound);
        fprintf(fw, "username=%s\n", cfg.username);
        fclose(fw);
    }

    printf("Configuration actuelle:\n");
    printf("height   = %d\n", cfg.height);
    printf("width    = %d\n", cfg.width);
    printf("sound    = %s\n", cfg.sound);
    printf("username = %s\n", cfg.username);

    // === Initialisation SDL2 ===
    
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
        SDL_WINDOW_SHOWN
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

    // === Boucle principale ===
    
    SDL_Event event;
    int running = 1;

    while (running) {
        // Gestion des événements
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = 0;
                }
            }
        }

        // Effacer l'écran (fond noir)
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // === Initialisation du jeu ===

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

// === Boucle principale ===

SDL_Event event;
int running = 1;

while (running) {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = 0;
        }
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                running = 0;
            }
            if (event.key.keysym.sym == SDLK_SPACE && !player.isJumping && !gameOver) {
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

        // Afficher le rendu
        SDL_RenderPresent(renderer);

        SDL_Delay(16); // ~60 FPS
    }

    // Nettoyage
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}