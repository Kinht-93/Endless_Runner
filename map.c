#include "map.h"

void init_map(Map *map, int screenWidth, int screenHeight) {
    srand(time(NULL));
    map->obstacleCount = 0;
    map->speed = 5.0f;
    map->acceleration = 0.001f;
    map->distance = 0;

    for (int i = 0; i < 3; i++) {
        generate_segment(map, i * SEGMENT_WIDTH + screenWidth, screenWidth, screenHeight);
    }
}

void update_map(Map *map, int screenWidth, int screenHeight) {
    map->speed += map->acceleration;
    map->distance += (int)map->speed;

    for (int i = 0; i < map->obstacleCount; i++) {
        if (map->obstacles[i].active) {
            map->obstacles[i].x -= map->speed;

            if (map->obstacles[i].x + map->obstacles[i].width < 0) {
                map->obstacles[i].active = 0;
            }
        }
    }

    int lastObstacleX = 0;
    for (int i = 0; i < map->obstacleCount; i++) {
        if (map->obstacles[i].active && map->obstacles[i].x > lastObstacleX) {
            lastObstacleX = (int)map->obstacles[i].x;
        }
    }

    if (lastObstacleX < screenWidth + SEGMENT_WIDTH) {
        generate_segment(map, lastObstacleX + SEGMENT_WIDTH, screenWidth, screenHeight);
    }
}

int is_map_feasible(Map *map, int screenWidth, int screenHeight) {
    for (int i = 0; i < map->obstacleCount - 1; i++) {
        if (map->obstacles[i].active && map->obstacles[i+1].active) {
            float gap = map->obstacles[i+1].x - (map->obstacles[i].x + map->obstacles[i].width);
            if (gap < MIN_GAP) {
                return 0;
            }
        }
    }
    return 1;
}

void generate_segment(Map *map, int startX, int screenWidth, int screenHeight) {
    int attempts = 0;
    const int maxAttempts = 10;

    do {
        for (int i = 0; i < map->obstacleCount; i++) {
            if (map->obstacles[i].x >= startX && map->obstacles[i].x < startX + SEGMENT_WIDTH) {
                map->obstacles[i].active = 0;
            }
        }

        int numObstacles = rand() % 3 + 1;
        for (int i = 0; i < numObstacles && map->obstacleCount < MAX_OBSTACLES; i++) {
            int idx = -1;
            for (int j = 0; j < MAX_OBSTACLES; j++) {
                if (!map->obstacles[j].active) {
                    idx = j;
                    break;
                }
            }
            if (idx == -1) break;

            map->obstacles[idx].x = startX + rand() % SEGMENT_WIDTH;
            map->obstacles[idx].y = screenHeight - 100;
            map->obstacles[idx].width = 40 + rand() % 20;
            map->obstacles[idx].height = 60 + rand() % 20;
            map->obstacles[idx].active = 1;
            map->obstacleCount++;
        }

        attempts++;
    } while (!is_map_feasible(map, screenWidth, screenHeight) && attempts < maxAttempts);

    if (attempts >= maxAttempts) {
        for (int i = 0; i < map->obstacleCount; i++) {
            if (map->obstacles[i].x >= startX && map->obstacles[i].x < startX + SEGMENT_WIDTH) {
                map->obstacles[i].active = 0;
            }
        }
        int idx = -1;
        for (int j = 0; j < MAX_OBSTACLES; j++) {
            if (!map->obstacles[j].active) {
                idx = j;
                break;
            }
        }
        if (idx != -1) {
            map->obstacles[idx].x = startX + SEGMENT_WIDTH / 2;
            map->obstacles[idx].y = screenHeight - 100;
            map->obstacles[idx].width = 40;
            map->obstacles[idx].height = 60;
            map->obstacles[idx].active = 1;
            map->obstacleCount++;
        }
    }
}