#ifndef PLAYER_H
#define PLAYER_H

#include "GameState.h"

struct Move {
    int pitIndex;
    bool clockwise;
};

class Player {
public:
    virtual Move chooseMove(const GameState& state) = 0;
    virtual ~Player() = default;
};

#endif
