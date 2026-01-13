#include "map.h"

void init_map(Map *map, int screenWidth, int screenHeight) {
    srand(time(NULL));
    map->speed = 5.0f;
    map->acceleration = 0.001f;
    map->distance = 0;
    
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        map->obstacles[i].active = 0;
    }
    
    for (int i = 0; i < 3; i++) {
        generate_segment(map, screenWidth + i * SEGMENT_WIDTH, screenWidth, screenHeight);
    }
}

void update_map(Map *map, int screenWidth, int screenHeight) {
    map->speed += map->acceleration;
    map->distance += (int)map->speed;

    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (map->obstacles[i].active) {
            map->obstacles[i].x -= map->speed;
            
            if (map->obstacles[i].x + map->obstacles[i].width < 0) {
                map->obstacles[i].active = 0;
            }
        }
    }

    int lastObstacleX = 0;
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (map->obstacles[i].active && map->obstacles[i].x > lastObstacleX) {
            lastObstacleX = (int)map->obstacles[i].x;
        }
    }

    if (lastObstacleX < screenWidth + SEGMENT_WIDTH) {
        generate_segment(map, lastObstacleX + SEGMENT_WIDTH, screenWidth, screenHeight);
    }
}

void generate_segment(Map *map, int startX, int screenWidth, int screenHeight) {
    int numObstacles = rand() % 2 + 1;
    
    for (int i = 0; i < numObstacles; i++) {
        int idx = -1;
        for (int j = 0; j < MAX_OBSTACLES; j++) {
            if (!map->obstacles[j].active) {
                idx = j;
                break;
            }
        }
        
        if (idx == -1) break;
        
        map->obstacles[idx].x = startX + (rand() % (SEGMENT_WIDTH - 100));
        map->obstacles[idx].width = 40 + rand() % 30;
        map->obstacles[idx].height = 60 + rand() % 40;
        map->obstacles[idx].y = GROUND_Y;  // AU SOL
        map->obstacles[idx].active = 1;
    }
}