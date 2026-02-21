#include "HumanPlayer.h"
#include <iostream>

using namespace std;

HumanPlayer::HumanPlayer(int id) : playerId(id) {}

Move HumanPlayer::chooseMove(const GameState& state) {
    int choice, direction;

    if (playerId == 0)
        cout << "Player 1 choose pit (0-4): ";
    else
        cout << "Player 2 choose pit (6-10): ";

    cin >> choice;

    cout << "Direction (0 = clockwise, 1 = counter): ";
    cin >> direction;

    return { choice, direction == 0 };
}
