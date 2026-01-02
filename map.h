#ifndef MAP_H
#define MAP_H

#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>

#define MAX_OBSTACLES 20
#define SEGMENT_WIDTH 400
#define MIN_GAP 80

typedef struct {
    float x, y;
    int width, height;
    int active;
} Obstacle;

typedef struct {
    Obstacle obstacles[MAX_OBSTACLES];
    int obstacleCount;
    float speed;
    float acceleration;
    int distance;
} Map;

void init_map(Map *map, int screenWidth, int screenHeight);
void update_map(Map *map, int screenWidth, int screenHeight);
int is_map_feasible(Map *map, int screenWidth, int screenHeight);
void generate_segment(Map *map, int startX, int screenWidth, int screenHeight);

#endif