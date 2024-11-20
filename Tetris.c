#include <stdio.h>

#define ROWS 20
#define COLS 10

static char board[ROWS][COLS];

void initializeBoard() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            board[i][j] = '.';  // 빈 공간을 '.'으로 표시
        }
    }
}

void printBoard() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            printf("%c ", board[i][j]);
        }
        printf("\n");
    }
}

void startTetris() {
    printf("Starting Tetris game...\n");

    initializeBoard();

    printBoard();
}
