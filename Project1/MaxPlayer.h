#ifndef MAXPLAYER_H
#define MAXPLAYER_H

#include "Player.h"
#include "Rules.h"

class MaxPlayer : public Player {
private:
    int side; // 0 = player1, 1 = player2

public:
    explicit MaxPlayer(int side);
    Move chooseMove(const GameState& state) override;

private:
    int evaluateGreedyGain(const GameState& before, const GameState& after) const;
};

#endif
