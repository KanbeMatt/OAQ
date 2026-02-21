#ifndef RULES_H
#define RULES_H

#include "GameState.h"

namespace Rules {

    void applyMove(GameState& state, int pit, bool clockwise, int side);
    void redistribute(GameState& state, int side);
    bool isTerminal(const GameState& state);
    bool prepareTurn(GameState& state, int side);
    bool isValidMove(const GameState& state, int startIndex, int side);

}

#endif
