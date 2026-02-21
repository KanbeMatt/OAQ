#ifndef HUMANPLAYER_H
#define HUMANPLAYER_H

#include "Player.h"

class HumanPlayer : public Player {
private:
    int playerId; // 0 or 1

public:
    HumanPlayer(int id);
    Move chooseMove(const GameState& state) override;
};

#endif
