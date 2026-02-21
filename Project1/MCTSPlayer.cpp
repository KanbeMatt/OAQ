#include "MCTSPlayer.h"
#include <cmath>
#include <random>
#include <algorithm>

static std::mt19937 rng(std::random_device{}());

MCTSPlayer::MCTSPlayer(int s, int it, double c)
    : side(s), iterations(it), exploration(c) {
}


// ============================
// Move selection
// ============================
Move MCTSPlayer::chooseMove(const GameState& rootState) {

    Node root;
    root.state = rootState;
    root.sideToMove = side;
    root.untriedMoves = generateMoves(rootState, side);

    for (int i = 0; i < iterations; ++i) {

        Node* node = select(&root);

        if (!Rules::isTerminal(node->state))
            node = expand(node);

        double result = simulate(node->state, node->sideToMove);

        backpropagate(node, result);
    }

    // choose most visited child
    Node* best = nullptr;
    int bestVisits = -1;

    for (auto& child : root.children) {
        if (child->visits > bestVisits) {
            bestVisits = child->visits;
            best = child.get();
        }
    }

    return best ? best->moveFromParent : Move{ -1, true };
}


// ============================
// Selection (UCT)
// ============================
MCTSPlayer::Node* MCTSPlayer::select(Node* node) {

    while (node->untriedMoves.empty() && !node->children.empty()) {

        Node* best = nullptr;
        double bestUCT = -1e18;

        for (auto& child : node->children) {

            double exploit = child->value / (child->visits + 1e-6);
            double explore = exploration *
                std::sqrt(std::log(node->visits + 1) / (child->visits + 1e-6));

            double uct = exploit + explore;

            if (uct > bestUCT) {
                bestUCT = uct;
                best = child.get();
            }
        }

        node = best;
    }

    return node;
}


// ============================
// Expansion
// ============================
MCTSPlayer::Node* MCTSPlayer::expand(Node* node) {

    auto moves = generateMoves(node->state, node->sideToMove);
    if (moves.empty()) return node;

    std::uniform_real_distribution<double> prob(0.0, 1.0);

    Move chosenMove;

    // 🎲 Exploration
    if (prob(rng) < epsilon) {

        chosenMove = moves[rng() % moves.size()];

    }
    // 🧠 Greedy choice
    else {

        double bestScore = -1e9;
        Move bestMove = moves[0];

        for (const Move& m : moves) {

            GameState next = node->state;

            if (!Rules::prepareTurn(next, node->sideToMove))
                Rules::applyMove(next, m.pitIndex, m.clockwise, node->sideToMove);

            double h = quickHeuristic(node->state, next);

            if (h > bestScore) {
                bestScore = h;
                bestMove = m;
            }
        }

        chosenMove = bestMove;
    }

    GameState next = node->state;

    if (!Rules::prepareTurn(next, node->sideToMove))
        Rules::applyMove(next, chosenMove.pitIndex, chosenMove.clockwise, node->sideToMove);

    auto child = std::make_unique<Node>();
    child->state = next;
    child->sideToMove = 1 - node->sideToMove;
    child->moveFromParent = chosenMove;
    child->parent = node;

    node->children.push_back(std::move(child));
    return node->children.back().get();
}


// ============================
// Simulation (rollout)
// ============================
double MCTSPlayer::simulate(GameState state, int sideToMove) {

    int current = sideToMove;
    int depth = 0;

    while (!Rules::isTerminal(state) && depth < rolloutLimit) {

        if (Rules::prepareTurn(state, current))
            break;

        auto moves = generateMoves(state, current);
        if (moves.empty()) break;

        std::uniform_int_distribution<int> dist(0, moves.size() - 1);
        Move m = moves[dist(rng)];

        Rules::applyMove(state, m.pitIndex, m.clockwise, current);

        current = 1 - current;
        depth++;
    }

    if (Rules::isTerminal(state))
        return terminalReward(state);

    return rolloutEvaluation(state);
}


// ============================
// Backpropagation
// ============================
void MCTSPlayer::backpropagate(Node* node, double result) {
    while (node) {
        node->visits++;
        node->value += result;   // NO sign flip
        node = node->parent;
    }
}


// ============================
// Move generation
// ============================
std::vector<Move> MCTSPlayer::generateMoves(
    const GameState& state,
    int s) const {

    std::vector<Move> moves;

    int start = (s == 0) ? 0 : 6;
    int end = (s == 0) ? 4 : 10;

    for (int i = start; i <= end; ++i) {
        if (Rules::isValidMove(state, i, s)) {
            moves.push_back({ i, true });
            moves.push_back({ i, false });
        }
    }
    return moves;
}


// ============================
// Terminal reward
// ============================
double MCTSPlayer::terminalReward(const GameState& state) const {

    int myScore = (side == 0) ? state.score1 : state.score2;
    int oppScore = (side == 0) ? state.score2 : state.score1;

    return (myScore - oppScore) / 50.0;
}


// ============================
// Rollout evaluation
// ============================
double MCTSPlayer::rolloutEvaluation(const GameState& state) const {

    int myScore = (side == 0) ? state.score1 : state.score2;
    int oppScore = (side == 0) ? state.score2 : state.score1;

    int myPits = 0, oppPits = 0;

    if (side == 0) {
        for (int i = 0; i <= 4; ++i) myPits += state.board[i];
        for (int i = 6; i <= 10; ++i) oppPits += state.board[i];
    }
    else {
        for (int i = 6; i <= 10; ++i) myPits += state.board[i];
        for (int i = 0; i <= 4; ++i) oppPits += state.board[i];
    }

    double score =
        (myScore - oppScore) +
        0.2 * (myPits - oppPits);

    return score / 50.0;
}

double MCTSPlayer::quickHeuristic(const GameState& before,
    const GameState& after) const {

    int myBefore = (side == 0) ? before.score1 : before.score2;
    int myAfter = (side == 0) ? after.score1 : after.score2;

    int oppBefore = (side == 0) ? before.score2 : before.score1;
    int oppAfter = (side == 0) ? after.score2 : after.score1;

    return (myAfter - myBefore) - (oppAfter - oppBefore);
}