#include "MinPlayer.h"
#include <limits>
#include <random>
#include <vector>

MinPlayer::MinPlayer(int s) : side(s) {}

Move MinPlayer::chooseMove(const GameState& state) {
    int start = (side == 0) ? 0 : 6;
    int end = (side == 0) ? 4 : 10;

    std::vector<Move> bestMoves;
    int lowestOpponentGain = std::numeric_limits<int>::max();
    int bestOwnGain = std::numeric_limits<int>::min();

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

            const int opponentBestGain = estimateOpponentBestGain(next);
            const int ownImmediateGain = evaluateImmediateGain(state, next, side);

            if (opponentBestGain < lowestOpponentGain ||
                (opponentBestGain == lowestOpponentGain && ownImmediateGain > bestOwnGain)) {
                lowestOpponentGain = opponentBestGain;
                bestOwnGain = ownImmediateGain;
                bestMoves.clear();
                bestMoves.push_back({ pit, clockwise });
            }
            else if (opponentBestGain == lowestOpponentGain && ownImmediateGain == bestOwnGain) {
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

int MinPlayer::evaluateImmediateGain(const GameState& before, const GameState& after, int evaluatingSide) const {
    const int myBefore = (evaluatingSide == 0) ? before.score1 : before.score2;
    const int myAfter = (evaluatingSide == 0) ? after.score1 : after.score2;

    const int oppBefore = (evaluatingSide == 0) ? before.score2 : before.score1;
    const int oppAfter = (evaluatingSide == 0) ? after.score2 : after.score1;

    return (myAfter - myBefore) - (oppAfter - oppBefore);
}

int MinPlayer::estimateOpponentBestGain(const GameState& stateAfterMyMove) const {
    const int opponent = 1 - side;
    const int start = (opponent == 0) ? 0 : 6;
    const int end = (opponent == 0) ? 4 : 10;

    GameState preparedState = stateAfterMyMove;
    if (Rules::prepareTurn(preparedState, opponent)) {
        return std::numeric_limits<int>::max();
    }

    int bestGain = std::numeric_limits<int>::min();

    for (int pit = start; pit <= end; ++pit) {
        if (!Rules::isValidMove(preparedState, pit, opponent)) {
            continue;
        }

        for (bool clockwise : { true, false }) {
            GameState oppNext = preparedState;
            Rules::applyMove(oppNext, pit, clockwise, opponent);
            int gain = evaluateImmediateGain(preparedState, oppNext, opponent);
            bestGain = std::max(bestGain, gain);
        }
    }

    if (bestGain == std::numeric_limits<int>::min()) {
        return std::numeric_limits<int>::max();
    }

    return bestGain;
}
