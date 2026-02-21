#include "Game.h"
#include "HumanPlayer.h"
#include "RandomPlayer.h"
#include "MinimaxPlayer.h"
#include "MCTSPlayer.h"
#include "MaxPlayer.h"
#include "MinPlayer.h"
#include <fstream>
#include <iostream>

void runSingleGame();
void runSimulation(int numGames);

const std::string OutputFile = "results.txt";

int main() {

    const int NUM_GAMES = 1000;

    int mode;

    std::cout << "Select mode:\n";
    std::cout << "1. Single Game\n";
    std::cout << "2. Simulation\n";
    std::cout << "Choice: ";
    std::cin >> mode;

    if (mode == 1) {
        runSingleGame();
    }
    else if (mode == 2) {
        runSimulation(NUM_GAMES);
    }
    else {
        std::cout << "Invalid mode.\n";
    }

    return 0;
}


void runSingleGame() {

    // Choose configuration here
    Player* p1 = new HumanPlayer(0);
    Player* p2 = new MCTSPlayer(1);

    Game game(p1, p2);
    game.run();

    delete p1;
    delete p2;
}

void runSimulation(int numGames) {

    std::ofstream out(OutputFile);

    if (!out.is_open()) {
        std::cout << "Failed to open results.txt\n";
        return;
    }

    int p1Wins = 0;
    int p2Wins = 0;
    int draws = 0;
    int totalMoves = 0;

    for (int i = 1; i <= numGames; ++i) {
        if (i % 10 == 0) std::cout << i << std::endl;

        Player* p1 = new RandomPlayer(0);
        Player* p2 = new MinimaxPlayer(1,3);

        Game game(p1, p2);
        game.runSilent();

        int s1 = game.getScore1();
        int s2 = game.getScore2();
        int moves = game.getMoves();

        totalMoves += moves;

        if (s1 > s2) p1Wins++;
        else if (s2 > s1) p2Wins++;
        else draws++;

        out << "Game " << i
            << " | P1: " << s1
            << " | P2: " << s2
            << " | Moves: " << moves
            << "\n";

        delete p1;
        delete p2;
    }

    out << "\nSUMMARY\n";
    out << "P1 Wins: " << p1Wins << "\n";
    out << "P2 Wins: " << p2Wins << "\n";
    out << "Draws: " << draws << "\n";
    out << "Average Moves per Game: "
        << static_cast<double>(totalMoves) / numGames << "\n";

    out.close();

    std::cout << "Simulation complete. Results written to results.txt\n";
}
