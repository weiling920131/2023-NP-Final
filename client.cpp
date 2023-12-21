#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <vector>

#define BOARDSIZE 5

using namespace std;

// black white empty
const char *piece[3] = {"\033[40m    \033[43m", "\033[47m    \033[43m", "    "};
vector<int> board(25, 2);

char s1[] = "\033[31m ███████╗\033[91m ██╗     \033[93m ██╗\033[32m ████████╗\033[36m ██╗  ██╗\033[34m ███████╗\033[35m ██████╗ ",
     s2[] = "\033[31m ██╔════╝\033[91m ██║     \033[93m ██║\033[32m ╚══██╔══╝\033[36m ██║  ██║\033[34m ██╔════╝\033[35m ██╔══██╗ ",
     s3[] = "\033[31m ███████╗\033[91m ██║     \033[93m ██║\033[32m    ██║   \033[36m ███████║\033[34m █████╗  \033[35m ██████╔╝ ",
     s4[] = "\033[31m ╚════██║\033[91m ██║     \033[93m ██║\033[32m    ██║   \033[36m ██╔══██║\033[34m ██╔══╝  \033[35m ██╔══██╗ ",
     s5[] = "\033[31m ███████║\033[91m ███████╗\033[93m ██║\033[32m    ██║   \033[36m ██║  ██║\033[34m ███████╗\033[35m ██║  ██║ ",
     s6[] = "\033[31m ╚══════╝\033[91m ╚══════╝\033[93m ╚═╝\033[32m    ╚═╝   \033[36m ╚═╝  ╚═╝\033[34m ╚══════╝\033[35m ╚═╝  ╚═╝ ";    

void init() {
    // set to 80*25 color mode
    printf("\033[=3h");
    // set screen size
    printf("\033[8;30;120t");
    // set bold, black font, gray background
    printf("\033[1;30;100m");
    // make cursor invisible
    printf("\033[?25l");
    return;
}

void printSlither() {
    // make cursor invisible
    printf("\033[?25l");
    // erase screen
    printf("\033[2J");
    // move cursor to line 2
    printf("\033[2H");
    // print SLITHER
    printf("\033[30C%s\n\033[30C%s\n\033[30C%s\n\033[30C%s\n\033[30C%s\n\033[30C%s\n",
            s1, s2, s3, s4, s5, s6);
    // reset black font
    printf("\033[30m");
    printf("\n");
    return;
}

void printSlitherMoving() {
    for (int n = 1; n <= 186; n++) {
        // erase screen
        printf("\033[2J");
        // move cursor to line 2
        printf("\033[2H");
        // print SLITHER
        printf("\033[30C%.*s\n\033[30C%.*s\n\033[30C%.*s\n\033[30C%.*s\n\033[30C%.*s\n\033[30C%.*s\n",
                n, s1, n, s2, n, s3, n, s4, n, s5, n, s6);
        // reset black font
        printf("\033[30m");
        usleep(10000);
    }
    printf("\n");
    return;
}

void printBoard(vector<int> board) {
    printSlither();

    // set yellow background
    printf("\033[43m");

    for (int i = 0, idx = 0; i < BOARDSIZE+1; i++, idx += BOARDSIZE) {
        // move cursor to middle
        printf("\033[36C ");
        for(int j = 0; j < BOARDSIZE+1; j++) {
            printf("+");

            if (j < BOARDSIZE) {
                printf("--------");
            }
        }

        printf(" \n\033[36C");

        if (i < BOARDSIZE) {
            printf(" ");
            for (int k = 0; k < BOARDSIZE; k++) {
                printf("|  %s  ", piece[board[idx+k]]);
            }
            printf("| \n\033[36C ");
            for (int k = 0; k < BOARDSIZE; k++) {
                printf("|  %s  ", piece[board[idx+k]]);

            }
            printf("| \n");
        }
    }
    printf("\n");
    // reset gray background
    printf("\033[100m");
    // move cursor to middle
    printf("\033[36C");
    // make cursor visible
    printf("\033[?25h");
    return;
}

int main() {
    init();
    printSlitherMoving();
    printBoard(board);
    
    int s, m, p;
    char input[10];
    while (1) {
        fgets(input, sizeof(input), stdin);
        if (sscanf(input, "%d %d %d", &s, &m, &p) != 3) {
            // move cursor to middle
            printf("\033[27;37H\033[0K");
            printf("illegal move!\n");
            // erase previous input
            printf("\033[26;37H\033[0K");
            continue;
        }
        if (s != -1 && m != -1) {
            if (board[s] == 0 && board[m] == 2) {
                board[s] = 2;
                board[m] = 0;
            }
            else {
                // move cursor to middle
                printf("\033[27;37H\033[0K");
                printf("illegal move!\n");
                // erase previous input
                printf("\033[26;37H\033[0K");
                continue;
            }
        }
        else if (s == -1 ^ m == -1) {
            // move cursor to middle
            printf("\033[27;37H\033[0K");
            printf("illegal move!\n");
            // erase previous input
            printf("\033[26;37H\033[0K");
            continue;
        }
        if (p != -1 && board[p] == 2) {
            board[p] = 0;
        }
        else {
            // move cursor to middle
            printf("\033[27;37H\033[0K");
            printf("illegal move!\n");
            // erase previous input
            printf("\033[26;37H\033[0K");
            continue;
        }
        printBoard(board);
    }

    return 0;
}