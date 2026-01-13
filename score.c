#include "score.h"

#define HIGH_SCORE_FILE "highscore.txt"

void init_score(ScoreSystem *score) {
    score->currentScore = 0;
    load_high_score(score);
}

void update_score(ScoreSystem *score, int points) {
    score->currentScore += points;
    if (score->currentScore > score->highScore) {
        score->highScore = score->currentScore;
        save_high_score(score);
    }
}

void save_high_score(ScoreSystem *score) {
    FILE *f = fopen(HIGH_SCORE_FILE, "w");
    if (f) {
        fprintf(f, "%d", score->highScore);
        fclose(f);
    }
}

void load_high_score(ScoreSystem *score) {
    FILE *f = fopen(HIGH_SCORE_FILE, "r");
    if (f) {
        fscanf(f, "%d", &score->highScore);
        fclose(f);
    } else {
        score->highScore = 0;
    }
}