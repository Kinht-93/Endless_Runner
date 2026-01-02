// config.h
#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int height;
    int width;
    char sound[4];
    char username[256];
    char key_jump;
    char key_quit;
} Config;

Config load_config();

#endif