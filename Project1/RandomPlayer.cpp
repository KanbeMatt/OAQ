#include "RandomPlayer.h"
#include <iostream>
#include <vector>

RandomPlayer::RandomPlayer(int s) : side(s) {
    std::random_device rd;
    rng = std::mt19937(rd());
}

Move RandomPlayer::chooseMove(const GameState& state) {

    std::vector<int> validPits;

    // Determine pit range
    int start = (side == 0) ? 0 : 6;
    int end = (side == 0) ? 4 : 10;

    // Collect non-empty pits
    for (int i = start; i <= end; ++i) {
        if (state.board[i] > 0)
            validPits.push_back(i);
    }

    // Safety check (should not happen if restore rule implemented)
    if (validPits.empty()) {
        return { start, true }; // fallback
    }

    // Random pit
    std::uniform_int_distribution<> pitDist(0, validPits.size() - 1);
    int chosenPit = validPits[pitDist(rng)];

    // Random direction
    std::uniform_int_distribution<> dirDist(0, 1);
    bool clockwise = dirDist(rng);

    // std::cout << "RandomPlayer (side " << side << ") chooses pit " << chosenPit << " and direction " << (clockwise ? "clockwise\n" : "counter-clockwise\n");
    return { chosenPit, clockwise };
}

