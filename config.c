#include "config.h"

void set_defaults(Config *c) {
    c->height = 600;
    c->width  = 800;
    strcpy(c->sound, "ON");
    strcpy(c->username, "Endless_Runner");
    c->key_jump = ' ';
    c->key_quit = 'q';
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

Config load_config() {
    const char *path = "config.ini";
    Config cfg;
    set_defaults(&cfg);

    FILE *f = fopen(path, "r");
    if (f) {
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
            else if (str_equal_ignorecase(key, "key_jump")) {
                if (val[0]) cfg.key_jump = val[0];
            }
            else if (str_equal_ignorecase(key, "key_quit")) {
                if (val[0]) cfg.key_quit = val[0];
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

    return cfg;
}
