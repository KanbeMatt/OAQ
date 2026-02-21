#ifndef GAME_H
#define GAME_H

#include "GameState.h"
#include "Player.h"

class Game {
private:
    GameState state;
    Player* player1;
    Player* player2;

    int nextIndex(int index, bool clockwise) const;

    int scatter(int startIndex, bool clockwise);
    void capture(int lastPosition, bool clockwise, Player* player);

    bool isValidMove(int startIndex, Player* player) const;
	bool isAllPitsEmpty(Player* player) const;

public:
    Game(Player* p1, Player* p2);

    void applyMove(int startIndex, bool clockwise, Player* player);
    void redistribute(Player* player);

    void printBoard() const;
    void printStats() const;

    bool isGameOver();
    void run();
    void runSilent();

    int getScore1() const;
    int getScore2() const;
    int getMoves() const;

};

#endif
