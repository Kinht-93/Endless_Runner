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
    char key_crouch;
    char key_reset;
} Config;

Config load_config();
void save_config(Config *cfg);

#endif