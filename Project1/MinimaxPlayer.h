#ifndef MINIMAXPLAYER_H
#define MINIMAXPLAYER_H

#include "Player.h"
#include "Rules.h"
#include <vector>

class MinimaxPlayer : public Player {
private:
    int side;          // 0 = P1, 1 = P2
    int maxDepth;

public:
    MinimaxPlayer(int side, int depth = 3);

    Move chooseMove(const GameState& state) override;

private:
    int minimax(GameState state,
        int depth,
        bool maximizing,
        int currentSide,
        int alpha,
        int beta);

    int evaluate(const GameState& state) const;

    std::vector<Move> generateMoves(const GameState& state,
        int side) const;
};

#endif
