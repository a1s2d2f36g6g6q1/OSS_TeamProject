#include <stdio.h>
#include "Tetris/Tetris.h"
#include "2048/2048.h"
#include "BrickBreaker/BrickBreaker.h"
#include "Minesweeper/MineSweeper.h"

void displayMenu() {
    printf("Select a game to play:\n");
    printf("1. Tetris\n");
    printf("2. 2048\n");
    printf("3. Brick Breaker\n");
    printf("4. Minesweeper\n");
    printf("0. Exit\n");
}

int main() {
    int choice;
    do {
        displayMenu();
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
        case 1:
            printf("Starting Tetris...\n");
            startTetris();
            break;
        case 2:
            printf("Starting 2048...\n");
            start2048();
            break;
        case 3:
            printf("Starting Brick Breaker...\n");
            startBrickbreaker();
            break;
        case 4:
            printf("Starting Minesweeper...\n");
            startMinesweeper();
            break;
        case 0:
            printf("Exiting the game selection.\n");
            break;
        default:
            printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 0);

    return 0;
}