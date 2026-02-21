#include "Rules.h"

namespace {

    int nextIndex(int index, bool clockwise) {
        return clockwise
            ? (index + 1) % 12
            : (index - 1 + 12) % 12;
    }

    int scatter(GameState& state, int startIndex, bool clockwise) {

        int current = startIndex;

        while (true) {

            int stones = state.board[current];

            if (stones == 0)
                break;

            state.board[current] = 0;

            int index = current;

            // Distribute stones
            while (stones > 0) {
                index = nextIndex(index, clockwise);
                state.board[index]++;
                stones--;
            }

            // Check chaining condition
            int next = nextIndex(index, clockwise);

            if (next != 5 && next != 11 && state.board[next] > 0) {
                current = next;
            } else {
                return index;
            }
        }

        return current;
    }

    bool isAllPitsEmpty(const GameState& state, int side) {
        if (side == 0) {
            for (int i = 0; i <= 4; i++) {
                if (state.board[i] > 0)
                    return false;
            }
        } else {
            for (int i = 6; i <= 10; i++) {
                if (state.board[i] > 0)
                    return false;
            }
        }
        return true;
	}

    void capture(GameState& state, int lastPosition, bool clockwise, int side) {

        while (true) {

            int next = nextIndex(lastPosition, clockwise);
            int nextNext = nextIndex(next, clockwise);

            // Standard capture condition
            if (state.board[next] == 0 && state.board[nextNext] > 0) {
                bool isQuan = (nextNext == 5 || nextNext == 11);

                if (!isQuan) {
                    if (side == 0)
                        state.score1 += state.board[nextNext];
                    else
                        state.score2 += state.board[nextNext];

                    state.board[nextNext] = 0;
                    lastPosition = nextNext;
                } else {
                    if (state.moves < 4)
                        break;
                    if (state.board[nextNext] < 5)
                        break;

                    int quanIndex = nextNext / 6;
                    int captured = state.board[nextNext] + state.quan[quanIndex];

                    if (side == 0)
                        state.score1 += captured;
                    else
                        state.score2 += captured;

                    state.board[nextNext] = 0;
                    state.quan[quanIndex] = 0;

                    lastPosition = nextNext;
                }
            }
            else {
                break;
            }
        }
    }

} // anonymous namespace



// ================================
// Public API
// ================================

void Rules::applyMove(GameState& state,
    int pit,
    bool clockwise,
    int side) {

    if (!isValidMove(state, pit, side))
        return;

    int last = scatter(state, pit, clockwise);

    capture(state, last, clockwise, side);

    state.moves++;
}

void Rules::redistribute(GameState& state, int side) {

    if (side == 0) {
        for (int i = 0; i <= 4; i++)
            state.board[i]++;

        state.score1 -= 5;
    }
    else {
        for (int i = 6; i <= 10; i++)
            state.board[i]++;

        state.score2 -= 5;
    }
}

bool Rules::isTerminal(const GameState& state) {

    if ((state.quan[0] + state.board[5] +
        state.quan[1] + state.board[11]) == 0)
        return true;

    if (state.score1 > 40)
        return true;

    if (state.score2 > 40)
        return true;

    if (state.moves >= 200)
        return true;

    return false;
}

bool Rules::prepareTurn(GameState& state, int side) {

    if (!isAllPitsEmpty(state, side))
        return false;

    if (side == 0) {
        if (state.score1 < 5)
            return true;   // cannot redistribute → game over
        redistribute(state, 0);
    }
    else {
        if (state.score2 < 5)
            return true;
        redistribute(state, 1);
    }

    return false;
}

bool Rules::isValidMove(const GameState& state, int startIndex, int side) {
    if (startIndex < 0 || startIndex >= 12)
        return false;
    if (state.board[startIndex] == 0)
        return false;
    // Do not allow scattering from Quan pits
    if (startIndex == 5 || startIndex == 11)
        return false;
    // Check side
    if (side == 0) {
        return (startIndex >= 0 && startIndex <= 4);
    }
    else {
        return (startIndex >= 6 && startIndex <= 10);
    }
}


