#pragma once
#include <string>
#include <unistd.h>
#define BOARDSIZE 5
using namespace std;

string slither[6] = {
    "\033[31m ███████╗\033[91m ██╗     \033[93m ██╗\033[32m ████████╗\033[36m ██╗  ██╗\033[34m ███████╗\033[35m ██████╗ ",
    "\033[31m ██╔════╝\033[91m ██║     \033[93m ██║\033[32m ╚══██╔══╝\033[36m ██║  ██║\033[34m ██╔════╝\033[35m ██╔══██╗ ",
    "\033[31m ███████╗\033[91m ██║     \033[93m ██║\033[32m    ██║   \033[36m ███████║\033[34m █████╗  \033[35m ██████╔╝ ",
    "\033[31m ╚════██║\033[91m ██║     \033[93m ██║\033[32m    ██║   \033[36m ██╔══██║\033[34m ██╔══╝  \033[35m ██╔══██╗ ",
    "\033[31m ███████║\033[91m ███████╗\033[93m ██║\033[32m    ██║   \033[36m ██║  ██║\033[34m ███████╗\033[35m ██║  ██║ ",
    "\033[31m ╚══════╝\033[91m ╚══════╝\033[93m ╚═╝\033[32m    ╚═╝   \033[36m ╚═╝  ╚═╝\033[34m ╚══════╝\033[35m ╚═╝  ╚═╝ "
};

// black white empty
string piece[3] = {"\033[40m    \033[43m", "\033[47m    \033[43m", "    "};

string serv_cat[10] = {
    "  |\\___/|",
    " =) ^Y^ (=",
    "  \\  ^  /",
    "   )=*=(",
    " /       \\",
    " |       |",
    "/| |   | |\\",
    "\\| |   |_|/\\",
    "//_// _____/",
    "    \\_)"
};

string cli_cat[12] = {
    "  |\\___/|",
    "  )     (",
    " =\\     /=",
    "   )===(",
    " /       \\",
    " |       |",
    "/   YOU   \\",
    "\\         /",
    " \\___  __/",
    "    ( (",
    "     ) )",
    "    (_("
};

void init() {
    // set to 80*25 color mode
    printf("\033[=3h");
    // set screen size
    printf("\033[8;30;120t");
    // set bold, black font, gray background
    printf("\033[1;30;100m");
    // make cursor invisible
    printf("\033[?25l");

    // print moving SLITHER
    for (int n = 1; n <= 186; n++) {
        // erase screen
        printf("\033[2J");
        // move cursor to line 2
        printf("\033[2H");
        // print SLITHER
        for (const auto s: slither) {
            printf("\033[30C%.*s\n", n, s.c_str());
        }
        // reset black font
        printf("\033[30m");
        usleep(10000);
    }
    printf("\n");
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
    for (const auto s: slither) {
        printf("\033[30C%s\n", s.c_str());
    }
    // reset black font
    printf("\033[30m");
    printf("\n");
    return;
}

void printServ() {
    for (const auto c: serv_cat) {
        printf("\033[95C%s\n", c.c_str());
    }
    printf("\033[11;90H>\n");
    printf("\033[11H");
}

void printServMsg(string msg) {
    printf("\033[40C%s\n", msg.c_str());
}

void printCli() {
    printf("\033[18H");
    for (const auto c: cli_cat) {
        printf("\033[20C%s\n", c.c_str());
    }
    printf("\033[20;25H>\n");
}

void printCliMsg(string msg) {
    printf("\033[40C%s\n", msg.c_str());
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
        printf(" \n\033[34C");
        if (i < BOARDSIZE) {
            printf("\033[100m%d \033[43m", BOARDSIZE-i);
            printf(" ");
            for (int k = 0; k < BOARDSIZE; k++) {
                printf("|  %s  ", piece[board[idx+k]].c_str());
            }
            printf("| \n\033[36C ");
            for (int k = 0; k < BOARDSIZE; k++) {
                printf("|  %s  ", piece[board[idx+k]].c_str());
            }
            printf("| \n");
        }
    }
    // reset gray background
    printf("\033[100m  ");
    printf("     A        B        C        D        E\n");
    // move cursor to middle
    printf("\033[36C");
    // make cursor visible
    printf("\033[?25h");
    return;
}