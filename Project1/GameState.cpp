#include "GameState.h"

GameState::GameState() {
    reset();
}

void GameState::reset() {
    int initial[12] = { 5,5,5,5,5,0,5,5,5,5,5,0 };
    for (int i = 0; i < 12; i++)
        board[i] = initial[i];

    quan[0] = 10;
    quan[1] = 10;
    moves = 0;
	score1 = 0;
	score2 = 0;
}
