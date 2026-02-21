#include "MinimaxPlayer.h"
#include <limits>
#include <random>
#include "Rules.h"

MinimaxPlayer::MinimaxPlayer(int s, int depth)
    : side(s), maxDepth(depth) {
}


// =============================
// Move selection
// =============================

Move MinimaxPlayer::chooseMove(const GameState& state) {

    auto moves = generateMoves(state, side);

    if (moves.empty())
        return { -1, true }; // no move

    int bestScore = std::numeric_limits<int>::min();
    std::vector<Move> bestMoves;

    for (const auto& m : moves) {

        GameState next = state;

        // simulate exactly like Game loop
        bool gameOver = Rules::prepareTurn(next, side);
        if (gameOver) continue;

        Rules::applyMove(next, m.pitIndex, m.clockwise, side);

        int score = minimax(
            next,
            maxDepth - 1,
            false,                 // opponent turn
            1 - side,
            std::numeric_limits<int>::min(),  // alpha
            std::numeric_limits<int>::max()   // beta
        );

        if (score > bestScore) {
            bestScore = score;
            bestMoves.clear();
            bestMoves.push_back(m);
        }
        else if (score == bestScore) {
            bestMoves.push_back(m);
        }
    }

    // 🎲 RANDOM TIE BREAK
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, bestMoves.size() - 1);

    return bestMoves[dist(rng)];
}



// =============================
// Minimax with alpha-beta
// =============================
int MinimaxPlayer::minimax(GameState state,
    int depth,
    bool maximizing,
    int currentSide,
    int alpha,
    int beta) {

    if (depth == 0 || Rules::isTerminal(state))
        return evaluate(state);

    // handle redistribution phase
    if (Rules::prepareTurn(state, currentSide))
        return evaluate(state);

    std::vector<Move> moves = generateMoves(state, currentSide);

    if (moves.empty())
        return evaluate(state);

    if (maximizing) {

        int best = std::numeric_limits<int>::min();

        for (const Move& m : moves) {

            GameState nextState = state;
            Rules::applyMove(nextState, m.pitIndex, m.clockwise, currentSide);

            int score = minimax(
                nextState,
                depth - 1,
                false,
                1 - currentSide,
                alpha,
                beta
            );

            best = std::max(best, score);
            alpha = std::max(alpha, score);

            if (beta <= alpha)
                break;
        }
        return best;
    }
    else {

        int best = std::numeric_limits<int>::max();

        for (const Move& m : moves) {

            GameState nextState = state;
            Rules::applyMove(nextState, m.pitIndex, m.clockwise, currentSide);

            int score = minimax(
                nextState,
                depth - 1,
                true,
                1 - currentSide,
                alpha,
                beta
            );

            best = std::min(best, score);
            beta = std::min(beta, score);

            if (beta <= alpha)
                break;
        }
        return best;
    }
}


// =============================
// Evaluation function
// =============================
int MinimaxPlayer::evaluate(const GameState& state) const {

    int myScore = (side == 0) ? state.score1 : state.score2;
    int oppScore = (side == 0) ? state.score2 : state.score1;

    int myPits = 0;
    int oppPits = 0;

    if (side == 0) {
        for (int i = 0; i <= 4; i++) myPits += state.board[i];
        for (int i = 6; i <= 10; i++) oppPits += state.board[i];
    }
    else {
        for (int i = 6; i <= 10; i++) myPits += state.board[i];
        for (int i = 0; i <= 4; i++) oppPits += state.board[i];
    }

    return
        (myScore - oppScore) * 10 +
        (myPits - oppPits);
}


// =============================
// Move generation
// =============================
std::vector<Move> MinimaxPlayer::generateMoves( const GameState& state, int side) const {
    std::vector<Move> moves;

    if (side == 0) {
        for (int i = 0; i <= 4; i++) {
            if (Rules::isValidMove(state, i, side)) {
                moves.push_back({ i, true });
                moves.push_back({ i, false });
            }
        }
    }
    else {
        for (int i = 6; i <= 10; i++) {
            if (Rules::isValidMove(state, i, side)) {
                moves.push_back({ i, true });
                moves.push_back({ i, false });
            }
        }
    }

    return moves;
}
