// MOHAMMADINE ANAS
// gcc config.c -o config

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int height;
    int width;
    char sound[4];
    char username[256];

    char key_up;
    char key_down;
    char key_left;
    char key_right;

} Config;

void set_defaults(Config *c) {
    c->height = 600;
    c->width  = 800;
    strcpy(c->sound, "ON");
    strcpy(c->username, "anaslebg");

    c->key_up    = 'W';
    c->key_down  = 'S';
    c->key_left  = 'A';
    c->key_right = 'D';
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

            else if (str_equal_ignorecase(key, "key_up")) {
                if (val[0]) cfg.key_up = val[0];
            }
            else if (str_equal_ignorecase(key, "key_down")) {
                if (val[0]) cfg.key_down = val[0];
            }
            else if (str_equal_ignorecase(key, "key_left")) {
                if (val[0]) cfg.key_left = val[0];
            }
            else if (str_equal_ignorecase(key, "key_right")) {
                if (val[0]) cfg.key_right = val[0];
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

        fprintf(fw, "key_up=%c\n", cfg.key_up);
        fprintf(fw, "key_down=%c\n", cfg.key_down);
        fprintf(fw, "key_left=%c\n", cfg.key_left);
        fprintf(fw, "key_right=%c\n", cfg.key_right);

        fclose(fw);
    }

    printf("Configuration actuelle:\n");
    printf("height   = %d\n", cfg.height);
    printf("width    = %d\n", cfg.width);
    printf("sound    = %s\n", cfg.sound);
    printf("username = %s\n", cfg.username);

    printf("key_up    = %c\n", cfg.key_up);
    printf("key_down  = %c\n", cfg.key_down);
    printf("key_left  = %c\n", cfg.key_left);
    printf("key_right = %c\n", cfg.key_right);

    return 0;
}