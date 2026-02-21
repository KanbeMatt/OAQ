#pragma once
#include "Player.h"
#include "Rules.h"
#include <memory>
#include <vector>

class MCTSPlayer : public Player {
public:
    MCTSPlayer(int side, int iterations = 2000, double c = 1.4);

    Move chooseMove(const GameState& state) override;

private:
    struct Node {
        GameState state;
        int sideToMove;
        Move moveFromParent;

        int visits = 0;
        double value = 0.0;   // accumulated reward

        Node* parent = nullptr;
        std::vector<Move> untriedMoves;
        std::vector<std::unique_ptr<Node>> children;
    };


    int side;
    int iterations;
    double exploration;
    int rolloutLimit = 40;
	double epsilon = 0.15; // for epsilon-greedy

    Node* select(Node* node);
    Node* expand(Node* node);
    double simulate(GameState state, int sideToMove);
    void backpropagate(Node* node, double result);

    std::vector<Move> generateMoves(const GameState& state, int side) const;

    double terminalReward(const GameState& state) const;
    double rolloutEvaluation(const GameState& state) const;
    double quickHeuristic(const GameState& before, const GameState& after) const;
};