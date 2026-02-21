#ifndef GAMESTATE_H
#define GAMESTATE_H

class GameState {
public:
    int board[12];
    int quan[2];
    int moves;
    int score1;
	int score2;

    GameState();
    void reset();
};

#endif
