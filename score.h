#ifndef SCORE_H
#define SCORE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int currentScore;
    int highScore;
} ScoreSystem;

void init_score(ScoreSystem *score);
void update_score(ScoreSystem *score, int points);
void save_high_score(ScoreSystem *score);
void load_high_score(ScoreSystem *score);

#endif