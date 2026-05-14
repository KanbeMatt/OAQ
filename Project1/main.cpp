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
void runMCTSParameterSweep(int roundsPerConfig);

const std::string OutputFile = "MCTSVsMin.txt";

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
    long long totalScoreDiff = 0;
};

struct MCTSConfig {
    int iterations;
    double exploration;
    int rolloutLimit;
    double epsilon;
};

struct ParameterSweepSummary {
    MCTSConfig config;
    int games = 0;
    int mctsWins = 0;
    int randomWins = 0;
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
    std::cout << "4. MCTS parameter sweep (alternating first player)\n";
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
    else if (mode == 4) {
        runMCTSParameterSweep(100);
    }
    else {
        std::cout << "Invalid mode.\n";
    }

    return 0;
}


void runSingleGame() {

    // Choose configuration here
    Player* p1 = new HumanPlayer(0);
    Player* p2 = new RandomPlayer(1);

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
		summary.totalScoreDiff += (s1 - s2);
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

    resultsOut << std::left
        << std::setw(12) << "P1"
        << std::setw(12) << "P2"
        << std::setw(10) << "P1 Win%"
        << std::setw(10) << "P2 Win%"
        << std::setw(10) << "Draw%"
        << std::setw(14) << "Avg Score Diff"
        << std::setw(12) << "Avg Moves"
        << std::setw(12) << "Avg ms"
        << "File"
        << "\n";

    resultsOut << std::string(100, '-') << "\n";
    resultsOut << std::fixed << std::setprecision(2);

    for (const MatchSummary& summary : allSummaries) {
        const double p1WinPct = 100.0 * summary.p1Wins / summary.games;
        const double p2WinPct = 100.0 * summary.p2Wins / summary.games;
        const double drawPct = 100.0 * summary.draws / summary.games;
        const double avgMoves = static_cast<double>(summary.totalMoves) / summary.games;
        const double avgMs = static_cast<double>(summary.totalTimeMs) / summary.games;
        const double avgScoreDiff = static_cast<double>(summary.totalScoreDiff) / summary.games;

        resultsOut << std::left
            << std::setw(12) << summary.p1Name
            << std::setw(12) << summary.p2Name
            << std::setw(10) << p1WinPct
            << std::setw(10) << p2WinPct
            << std::setw(10) << drawPct
            << std::setw(14) << avgScoreDiff
            << std::setw(12) << avgMoves
            << std::setw(12) << avgMs
            << (summary.p1Name + "Vs" + summary.p2Name + ".txt")
            << "\n";
    }

    std::cout << "Simulation complete. Results written to results.txt\n";
}

void runMCTSParameterSweep(int roundsPerConfig) {
    const std::vector<int> iterationsList = { 500, 1000, 2000, 4000 };
    const std::vector<double> explorationList = { 1.0, 1.2, 1.4, 1.8 };
    const std::vector<int> rolloutLimitList = { 20, 40, 60 };
    const std::vector<double> epsilonList = { 0.05, 0.15, 0.30 };

    std::vector<ParameterSweepSummary> summaries;
    summaries.reserve(iterationsList.size() * explorationList.size() * rolloutLimitList.size() * epsilonList.size());

    using clock = std::chrono::steady_clock;

    for (int iters : iterationsList) {
        for (double c : explorationList) {
            for (int rolloutLimit : rolloutLimitList) {
                for (double eps : epsilonList) {
                    ParameterSweepSummary summary;
                    summary.config = { iters, c, rolloutLimit, eps };
                    summary.games = roundsPerConfig;

                    std::cout << "Testing config"
                        << " | iters=" << iters
                        << " | c=" << c
                        << " | rollout=" << rolloutLimit
                        << " | epsilon=" << eps
                        << "\n";

                    for (int gameIndex = 0; gameIndex < roundsPerConfig; ++gameIndex) {
                        const bool mctsFirst = (gameIndex % 2 == 0);

                        std::unique_ptr<Player> p1(
                            mctsFirst
                            ? static_cast<Player*>(new MCTSPlayer(0, iters, c, rolloutLimit, eps))
                            : static_cast<Player*>(new MinPlayer(0)));

                        std::unique_ptr<Player> p2(
                            mctsFirst
                            ? static_cast<Player*>(new MinPlayer(1))
                            : static_cast<Player*>(new MCTSPlayer(1, iters, c, rolloutLimit, eps)));

                        const auto gameStart = clock::now();
                        Game game(p1.get(), p2.get());
                        game.runSilent();
                        const auto gameEnd = clock::now();

                        summary.totalTimeMs += std::chrono::duration_cast<std::chrono::milliseconds>(gameEnd - gameStart).count();
                        summary.totalMoves += game.getMoves();

                        const int scoreP1 = game.getScore1();
                        const int scoreP2 = game.getScore2();

                        if (scoreP1 == scoreP2) {
                            summary.draws++;
                        }
                        else {
                            const bool p1Won = scoreP1 > scoreP2;
                            const bool mctsWon = (mctsFirst && p1Won) || (!mctsFirst && !p1Won);
                            if (mctsWon) summary.mctsWins++;
                            else summary.randomWins++;
                        }
                    }

                    summaries.push_back(summary);
                }
            }
        }
    }

    std::sort(summaries.begin(), summaries.end(), [](const ParameterSweepSummary& a, const ParameterSweepSummary& b) {
        if (a.mctsWins != b.mctsWins) return a.mctsWins > b.mctsWins;
        return a.totalTimeMs < b.totalTimeMs;
        });

    std::ofstream out("MCTSParameterSweep.txt");
    if (!out.is_open()) {
        std::cout << "Failed to open MCTSParameterSweep.txt\n";
        return;
    }

    out << std::fixed << std::setprecision(3);
    out << "Rounds per config: " << roundsPerConfig << " (first player alternates every game)\n\n";
    out << std::left
        << std::setw(10) << "Iters"
        << std::setw(10) << "C"
        << std::setw(10) << "Rollout"
        << std::setw(10) << "Eps"
        << std::setw(12) << "MCTS Win%"
        << std::setw(12) << "Rnd Win%"
        << std::setw(10) << "Draw%"
        << std::setw(12) << "AvgMoves"
        << std::setw(12) << "Avg ms"
        << "\n";
    out << std::string(100, '-') << "\n";

    for (const auto& s : summaries) {
        const double mctsWinPct = 100.0 * s.mctsWins / s.games;
        const double randomWinPct = 100.0 * s.randomWins / s.games;
        const double drawPct = 100.0 * s.draws / s.games;
        const double avgMoves = static_cast<double>(s.totalMoves) / s.games;
        const double avgMs = static_cast<double>(s.totalTimeMs) / s.games;

        out << std::left
            << std::setw(10) << s.config.iterations
            << std::setw(10) << s.config.exploration
            << std::setw(10) << s.config.rolloutLimit
            << std::setw(10) << s.config.epsilon
            << std::setw(12) << mctsWinPct
            << std::setw(12) << randomWinPct
            << std::setw(10) << drawPct
            << std::setw(12) << avgMoves
            << std::setw(12) << avgMs
            << "\n";
    }

    if (!summaries.empty()) {
        const auto& best = summaries.front();
        std::cout << "Best config by wins: "
            << "iters=" << best.config.iterations
            << ", c=" << best.config.exploration
            << ", rollout=" << best.config.rolloutLimit
            << ", epsilon=" << best.config.epsilon
            << " | wins=" << best.mctsWins << "/" << best.games
            << "\n";
    }

    std::cout << "Parameter sweep complete. Results written to MCTSParameterSweep.txt\n";
}
