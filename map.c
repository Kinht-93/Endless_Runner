#include "map.h"

void init_map(Map *map, int screenWidth, int screenHeight) {
    srand(time(NULL));
    map->speed = 5.0f;
    map->acceleration = 0.005f;
    map->distance = 0;
    
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        map->obstacles[i].active = 0;
    }
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        map->enemies[i].active = 0;
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
    
    // Déplacer les ennemis
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (map->enemies[i].active) {
            map->enemies[i].x -= map->speed;
            
            if (map->enemies[i].x + map->enemies[i].width < 0) {
                map->enemies[i].active = 0;
            }
        }
    }

    int lastObstacleX = 0;
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (map->obstacles[i].active && map->obstacles[i].x > lastObstacleX) {
            lastObstacleX = (int)map->obstacles[i].x;
        }
    }
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (map->enemies[i].active && map->enemies[i].x > lastObstacleX) {
            lastObstacleX = (int)map->enemies[i].x;
        }
    }

    if (lastObstacleX < screenWidth + SEGMENT_WIDTH) {
        generate_segment(map, lastObstacleX + SEGMENT_WIDTH, screenWidth, screenHeight);
    }
}

void generate_segment(Map *map, int startX, int screenWidth, int screenHeight) {
    // Générer obstacles normaux (blancs)
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
        map->obstacles[idx].y = GROUND_Y;
        map->obstacles[idx].active = 1;
    }
    
    // Générer ennemis (rouges) - 30% de chance
    if (rand() % 100 < 30) {
        int idx = -1;
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (!map->enemies[j].active) {
                idx = j;
                break;
            }
        }
        
        if (idx != -1) {
            map->enemies[idx].x = startX + (rand() % (SEGMENT_WIDTH - 100));
            map->enemies[idx].width = 35 + rand() % 20;
            map->enemies[idx].height = 35 + rand() % 20;
            
            // 50% chance d'être en l'air, 50% au sol
            if (rand() % 2 == 0) {
                // Au sol
                map->enemies[idx].y = GROUND_Y;
                map->enemies[idx].isFlying = 0;
            } else {
                // En l'air (hauteur aléatoire)
                map->enemies[idx].y = GROUND_Y - 100 - (rand() % 150);
                map->enemies[idx].isFlying = 1;
            }
            
            map->enemies[idx].active = 1;
        }
    }
}