#include "Game.h"
#include "HumanPlayer.h"
#include "RandomPlayer.h"
#include "MinimaxPlayer.h"
#include "MCTSPlayer.h"
#include "MaxPlayer.h"
#include "MinPlayer.h"
#include <chrono>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <chrono>

void runSingleGame();
void runSimulation(int numGames);
void runAllSimulations(int numGames);

const std::string OutputFile = "results.txt";

struct AgentSpec {
    std::string name;
    std::function<Player*(int)> create;
};

struct MatchSummary {
    std::string p1Name;
    std::string p2Name;
    int games = 0;
    int p1Wins = 0;
    int p2Wins = 0;
    int draws = 0;
    int totalMoves = 0;
    long long totalTimeMs = 0;
};

int main() {

    const int NUM_GAMES = 1000;

    int mode;

    std::cout << "Select mode:\n";
    std::cout << "1. Single Game\n";
    std::cout << "2. Simulation\n";
    std::cout << "3. Run all agent vs agent simulations\n";
    std::cout << "Choice: ";
    std::cin >> mode;

    if (mode == 1) {
        runSingleGame();
    }
    else if (mode == 2) {
        runSimulation(NUM_GAMES);
    }
    else if (mode == 3) {
        runAllSimulations(NUM_GAMES);
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

static MatchSummary runMatchupSimulation(
    const AgentSpec& p1Agent,
    const AgentSpec& p2Agent,
    int numGames,
    const std::string& outputFile) {

    std::ofstream out(outputFile);

    MatchSummary summary;
    summary.p1Name = p1Agent.name;
    summary.p2Name = p2Agent.name;
    summary.games = numGames;

    if (!out.is_open()) {
        std::cout << "Failed to open " << outputFile << "\n";
        return summary;
    }

    using clock = std::chrono::steady_clock;
    const auto simulationStart = clock::now();

    for (int i = 1; i <= numGames; ++i) {
        if (i % 10 == 0) {
            std::cout << p1Agent.name << " vs " << p2Agent.name << " | game " << i << "\n";
        }

        const auto gameStart = clock::now();

        std::unique_ptr<Player> p1(p1Agent.create(0));
        std::unique_ptr<Player> p2(p2Agent.create(1));

        Game game(p1.get(), p2.get());
        game.runSilent();

        const int s1 = game.getScore1();
        const int s2 = game.getScore2();
        const int moves = game.getMoves();

        const auto gameEnd = clock::now();
        const auto gameDurationMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(gameEnd - gameStart).count();

        summary.totalMoves += moves;

        if (s1 > s2) summary.p1Wins++;
        else if (s2 > s1) summary.p2Wins++;
        else summary.draws++;

        out << "Game " << i
            << " | P1: " << s1
            << " | P2: " << s2
            << " | Moves: " << moves
            << " | Time(ms): " << gameDurationMs
            << "\n";
    }

    const auto simulationEnd = clock::now();
    summary.totalTimeMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(simulationEnd - simulationStart).count();

    out << "\nSUMMARY\n";
    out << "P1 Wins: " << summary.p1Wins << "\n";
    out << "P2 Wins: " << summary.p2Wins << "\n";
    out << "Draws: " << summary.draws << "\n";
    out << "Average Moves per Game: "
        << static_cast<double>(summary.totalMoves) / numGames << "\n";
    out << "Total Simulation Time(ms): " << summary.totalTimeMs << "\n";
    out << "Average Time per Game(ms): "
        << static_cast<double>(summary.totalTimeMs) / numGames << "\n";

    return summary;
}

void runSimulation(int numGames) {
    const AgentSpec p1{ "MCTS", [](int side) { return new MCTSPlayer(side); } };
    const AgentSpec p2{ "Min", [](int side) { return new MinPlayer(side); } };

    runMatchupSimulation(p1, p2, numGames, OutputFile);
    std::cout << "Simulation complete. Results written to " << OutputFile << "\n";
}

void runAllSimulations(int numGames) {
    const std::vector<AgentSpec> agents = {
        { "Random", [](int side) { return new RandomPlayer(side); } },
        { "Minimax", [](int side) { return new MinimaxPlayer(side); } },
        { "MCTS", [](int side) { return new MCTSPlayer(side); } },
        { "Max", [](int side) { return new MaxPlayer(side); } },
        { "Min", [](int side) { return new MinPlayer(side); } }
    };

    std::vector<MatchSummary> allSummaries;

    for (const AgentSpec& p1 : agents) {
        for (const AgentSpec& p2 : agents) {
            const std::string fileName = p1.name + "Vs" + p2.name + ".txt";
            allSummaries.push_back(runMatchupSimulation(p1, p2, numGames, fileName));
        }
    }

    std::ofstream resultsOut("results.txt");
    if (!resultsOut.is_open()) {
        std::cout << "Failed to open results.txt\n";
        return;
    }

    int p1Wins = 0;
    int p2Wins = 0;
    int draws = 0;
    int totalMoves = 0;

    using clock = std::chrono::steady_clock;
    const auto simulationStart = clock::now();

    for (int i = 1; i <= numGames; ++i) {
        if (i % 10 == 0) std::cout << i << std::endl;

        const auto gameStart = clock::now();

        Player* p1 = new MinimaxPlayer(0);
        Player* p2 = new RandomPlayer(1);

        Game game(p1, p2);
        game.runSilent();

        int s1 = game.getScore1();
        int s2 = game.getScore2();
        int moves = game.getMoves();
        const auto gameEnd = clock::now();
        const auto gameDurationMs = std::chrono::duration_cast<std::chrono::milliseconds>(gameEnd - gameStart).count();

        totalMoves += moves;

        if (s1 > s2) p1Wins++;
        else if (s2 > s1) p2Wins++;
        else draws++;

        resultsOut << "Game " << i
            << " | P1: " << s1
            << " | P2: " << s2
            << " | Moves: " << moves
            << " | Time(ms): " << gameDurationMs
            << "\n";
    }

    resultsOut << "\nSUMMARY\n";
    resultsOut << "P1 Wins: " << p1Wins << "\n";
    resultsOut << "P2 Wins: " << p2Wins << "\n";
    resultsOut << "Draws: " << draws << "\n";
    resultsOut << "Average Moves per Game: "
        << static_cast<double>(totalMoves) / numGames << "\n";

    const auto simulationEnd = clock::now();
    const auto totalDurationMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(simulationEnd - simulationStart).count();

    resultsOut << "Total Simulation Time(ms): " << totalDurationMs << "\n";
    resultsOut << "Average Time per Game(ms): "
        << static_cast<double>(totalDurationMs) / numGames << "\n";

    resultsOut.close();

    std::cout << "Simulation complete. Results written to results.txt\n";
}
