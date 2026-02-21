#ifndef MINPLAYER_H
#define MINPLAYER_H

#include "Player.h"
#include "Rules.h"

class MinPlayer : public Player {
private:
    int side; // 0 = player1, 1 = player2

public:
    explicit MinPlayer(int side);
    Move chooseMove(const GameState& state) override;

private:
    int evaluateImmediateGain(const GameState& before, const GameState& after, int evaluatingSide) const;
    int estimateOpponentBestGain(const GameState& stateAfterMyMove) const;
};

#endif
