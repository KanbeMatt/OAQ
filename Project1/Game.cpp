#include "Game.h"
#include <iostream>
#include <iomanip>

using namespace std;
const bool debug_text = false;

Game::Game(Player* p1, Player* p2)
    : player1(p1), player2(p2) {
}

int Game::nextIndex(int index, bool clockwise) const {
    if (clockwise)
        return (index + 1) % 12;
    else
        return (index - 1 + 12) % 12;
}

int Game::scatter(int startIndex, bool clockwise) {
    int current = startIndex;
    int times = 0;

    while (true) {
        int stones = state.board[current];

        if (stones == 0)
            break;

        state.board[current] = 0;

        int index = current;

        if (debug_text) std::cout << "[SCATTER] (" << ++times << "th time) " << stones << " stones scattered from pit " << current << " " << (clockwise ? "clockwise" : "counter-clockwise") << ".\n";

        // scatter stones
        while (stones > 0) {
            index = nextIndex(index, clockwise);
            state.board[index]++;
            stones--;
        }

        // check chaining condition:
        int next = nextIndex(index, clockwise);

        if (debug_text) printBoard();

        if (state.board[next] > 0 && next % 6 != 5) {
            current = next;   // continue scattering
        }
        else {
            return index;     // stop scattering phase
        }
    }

    return current;
}

bool Game::isValidMove(int startIndex, Player* player) const {
    if (startIndex < 0 || startIndex >= 12) return false;
    if (state.board[startIndex] == 0) return false;
    if (player == player1) {
        if (startIndex >= 0 && startIndex <= 4) return true;
        else return false;
    }
    else if (player == player2) {
        if (startIndex >= 6 && startIndex <= 10) return true;
        else return false;
    }
    return false;
}

bool Game::isAllPitsEmpty(Player* player) const {
    if (player == player1) {
        for (int i = 0; i <= 4; i++) {
            if (state.board[i] > 0) return false;
        }
    }
    else {
        for (int i = 6; i <= 10; i++) {
            if (state.board[i] > 0) return false;
        }
    }
    return true;
}

void Game::capture(int lastPosition, bool clockwise, Player* player) {

    while (true) {
        int next = nextIndex(lastPosition, clockwise);
        int nextNext = nextIndex(next, clockwise);

        // capture condition:
        // next pit empty
        // next-next pit has stones
        if (state.board[next] == 0 && state.board[nextNext] > 0) {
            if (nextNext != 5 && nextNext != 11) { // capturing a peasant
                if (player == player1) {
                    state.score1 += state.board[nextNext];
                    if (debug_text) std::cout << "[CAPTURE] Player 1 captures " << state.board[nextNext] << " stones from pit " << nextNext << ".\n";
                } else {
                    state.score2 += state.board[nextNext];
                    if (debug_text) std::cout << "[CAPTURE] Player 2 captures " << state.board[nextNext] << " stones from pit " << nextNext << ".\n";
                }

                state.board[nextNext] = 0;

                lastPosition = nextNext;  // continue capture chain
            }
			else { // capturing a quan
				if (state.moves < 4) break; // cannot capture quans in first 2 turns
				if (state.board[nextNext] < 5) break; // cannot capture quans if less than 5 stones
                if (player == player1) {
                    state.score1 += state.board[nextNext] + state.quan[nextNext / 6];
                    if (debug_text) std::cout << "[CAPTURE] Player 1 captures " << state.board[nextNext] + state.quan[nextNext / 6] << " stones and quan from pit " << nextNext << ".\n";
                } else {
                    state.score2 += state.board[nextNext] + state.quan[nextNext / 6];
                    if (debug_text) std::cout << "[CAPTURE] Player 2 captures " << state.board[nextNext] + state.quan[nextNext / 6] << " stones and quan from pit " << nextNext << ".\n";
                }
                state.board[nextNext] = 0;
				state.quan[nextNext / 6] = 0;
				lastPosition = nextNext;  // continue capture chain
            }
        }
        else {
            break;
        }
    }
}

void Game::redistribute(Player* player) {
    if (player == player1) {
        for (int i = 0; i <= 4; i++) {
            state.board[i]++;
        }
		state.score1 -= 5;
    }
    else {
        for (int i = 6; i <= 10; i++) {
			state.board[i]++;
        }
		state.score2 -= 5;
    }
}

void Game::applyMove(int startIndex, bool clockwise, Player* player) {

    if (!isValidMove(startIndex, player)) {
        if (debug_text) std::cout << "[ERROR] Invalid move.\n";
        return;
    }

    int lastPos = scatter(startIndex, clockwise);

    capture(lastPos, clockwise, player);

    state.moves++;
}


void Game::printBoard() const {
    std::cout << " (   | ";
    for (int i = 0; i < 5; i++)
        cout << setw(2) << state.board[i] << " | ";
    std::cout << "  )\n";

    std::cout << "( " << setw(2)
        << state.quan[1] + state.board[11]
        << " |------------------------| "
        << setw(2)
        << state.quan[0] + state.board[5]
        << " )\n";

    std::cout << " (   | ";
    for (int i = 10; i > 5; i--)
        std::cout << setw(2) << state.board[i] << " | ";
    std::cout << "  )\n";
	std::cout << "\n";
}

void Game::printStats() const {
	std::cout << "[STATS] Moves: " << state.moves << "  - P1 Score: " << state.score1 << "  - P2 Score: " << state.score2 << endl;
}

bool Game::isGameOver() {
	// 1. The two quans are empty
    if ((state.quan[0] + state.board[5] + state.quan[1] + state.board[11]) == 0) {
        if (debug_text) std::cout << "[GAME OVER] Both Quan is empty!\n";
        return true;
    }
	// 2. One player's pits are all empty and they cannot redistribute
    if (isAllPitsEmpty(player1)) {
        if (state.score1 < 5) { 
            if (debug_text) std::cout << "[GAME OVER] Player 1 can't redistribute!\n";
            return true; 
        }
        else { 
            if (debug_text) std::cout << "[ACTION] Player 1 redistributes!\n";
            redistribute(player1); 
        }
    }
    if (isAllPitsEmpty(player2)) {
        if (state.score2 < 5) { 
            if (debug_text) std::cout << "[GAME OVER] Player 2 can't redistribute!\n";
            return true; 
        }
        else {
            if (debug_text) std::cout << "[ACTION] Player 2 redistributes!\n";
            redistribute(player2);
        }
    }
    // 3. One player has more than 40 stones
	if (state.score1 > 40) {
        if (debug_text) std::cout << "[GAME OVER] Player 1 has more than 40 stones!\n";
        return true;
	}
    if (state.score2 > 40) {
        if (debug_text) std::cout << "[GAME OVER] Player 2 has more than 40 stones!\n";
        return true;
	}
	// 4. Move limit reached (optional)
    if (state.moves >= 200) {
        if (debug_text) std::cout << "[GAME OVER] Move limit reached!\n";
        return true;
	}
	return false;
}

void Game::run() {
    while (!isGameOver()) {

        printStats();
        std::cout << "\n";
        printBoard();
        Move m1 = player1->chooseMove(state);
        while (!isValidMove(m1.pitIndex, player1)) {
            std::cout << "[ERROR] Invalid move. Try again.\n";
            m1 = player1->chooseMove(state);
		}
        applyMove(m1.pitIndex, m1.clockwise, player1);
		std::cout << "-----------------------------------------------------------\n";

        if (isGameOver()) break;

        printStats();
        std::cout << "\n";
        printBoard();
        Move m2 = player2->chooseMove(state);
        while (!isValidMove(m2.pitIndex, player2)) {
            std::cout << "[ERROR] Invalid move. Try again.\n";
            m2 = player2->chooseMove(state);
        }
        applyMove(m2.pitIndex, m2.clockwise, player2);
		std::cout << "-----------------------------------------------------------\n";
    }

    std::cout << "\n[END] Game Over!";
    if (state.score1 > state.score2) {
        std::cout << " Player 1 wins!\n";
    }
    else if (state.score2 > state.score1) {
        std::cout << " Player 2 wins!\n";
    }
    else {
        std::cout << " It's a tie!\n";
	}
    printStats();
    std::cout << "\n";
    printBoard();
}

void Game::runSilent() {
    while (!isGameOver()) {

        Move m1 = player1->chooseMove(state);
        applyMove(m1.pitIndex, m1.clockwise, player1);

        if (isGameOver()) break;

        Move m2 = player2->chooseMove(state);
        applyMove(m2.pitIndex, m2.clockwise, player2);
    }
}

int Game::getScore1() const { return state.score1; }
int Game::getScore2() const { return state.score2; }
int Game::getMoves() const { return state.moves; }

