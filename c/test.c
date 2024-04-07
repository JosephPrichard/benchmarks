//
// Created by Joseph on 4/6/2024.
//

#include "puzzle.h"

int main() {

    board_t board = {2,7, 4, 6, 0, 1, 3, 8, 5};
    long long h = hash_board(board, 9);
    if (h < 0) {
        printf("Expected to be valid hash, got %lld", h);
    }

    board_t board1 = {2,7, 4, 6, 0, 1, 3, 8, 5, 10, 11, 12, 13, 14, 15};
    h = hash_board(board1, 16);
    if (h < 0) {
        printf("Expected to be valid hash, got %lld", h);
    }
}