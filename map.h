#ifndef MAP_H
#define MAP_H

#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>

#define MAX_OBSTACLES 20
#define MAX_ENEMIES 20
#define SEGMENT_WIDTH 400
#define MIN_GAP 80
#define GROUND_Y 500

typedef struct {
    float x, y;
    int width, height;
    int active;
} Obstacle;

typedef struct {
    float x, y;
    int width, height;
    int active;
    int isFlying;
} Enemy;

typedef struct {
    Obstacle obstacles[MAX_OBSTACLES];
    Enemy enemies[MAX_ENEMIES];
    int obstacleCount;
    float speed;
    float acceleration;
    int distance;
} Map;

void init_map(Map *map, int screenWidth, int screenHeight);
void update_map(Map *map, int screenWidth, int screenHeight);
void generate_segment(Map *map, int startX, int screenWidth, int screenHeight);

#endif
