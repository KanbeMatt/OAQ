#include "MaxPlayer.h"
#include <limits>
#include <random>
#include <vector>

MaxPlayer::MaxPlayer(int s) : side(s) {}

Move MaxPlayer::chooseMove(const GameState& state) {
    int start = (side == 0) ? 0 : 6;
    int end = (side == 0) ? 4 : 10;

    std::vector<Move> bestMoves;
    int bestGain = std::numeric_limits<int>::min();

    for (int pit = start; pit <= end; ++pit) {
        if (!Rules::isValidMove(state, pit, side)) {
            continue;
        }

        for (bool clockwise : { true, false }) {
            GameState next = state;

            if (Rules::prepareTurn(next, side)) {
                continue;
            }

            Rules::applyMove(next, pit, clockwise, side);

            int gain = evaluateGreedyGain(state, next);

            if (gain > bestGain) {
                bestGain = gain;
                bestMoves.clear();
                bestMoves.push_back({ pit, clockwise });
            }
            else if (gain == bestGain) {
                bestMoves.push_back({ pit, clockwise });
            }
        }
    }

    if (bestMoves.empty()) {
        return { start, true };
    }

    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, static_cast<int>(bestMoves.size()) - 1);

    return bestMoves[dist(rng)];
}

int MaxPlayer::evaluateGreedyGain(const GameState& before, const GameState& after) const {
    int myBefore = (side == 0) ? before.score1 : before.score2;
    int myAfter = (side == 0) ? after.score1 : after.score2;

    int oppBefore = (side == 0) ? before.score2 : before.score1;
    int oppAfter = (side == 0) ? after.score2 : after.score1;

    int myPitsBefore = 0;
    int myPitsAfter = 0;

    if (side == 0) {
        for (int i = 0; i <= 4; ++i) {
            myPitsBefore += before.board[i];
            myPitsAfter += after.board[i];
        }
    }
    else {
        for (int i = 6; i <= 10; ++i) {
            myPitsBefore += before.board[i];
            myPitsAfter += after.board[i];
        }
    }

    const int immediateScoreGain = (myAfter - myBefore) - (oppAfter - oppBefore);
    const int pitRetention = myPitsAfter - myPitsBefore;

    return immediateScoreGain * 10 + pitRetention;
}
