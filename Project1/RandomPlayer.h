#ifndef RANDOMPLAYER_H
#define RANDOMPLAYER_H

#include "Player.h"
#include <random>

class RandomPlayer : public Player {
private:
    int side; // 0 = player1 (pits 0-4), 1 = player2 (pits 6-10)
    std::mt19937 rng;

public:
    RandomPlayer(int side);
    Move chooseMove(const GameState& state) override;
};

#endif
