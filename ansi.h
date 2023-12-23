#pragma once
#include <stdio.h>
#include <string>
#include <sstream>
#include <unistd.h>
#define BOARDSIZE 5
using namespace std;

string slither[] = {
    "\033[31m ███████╗\033[91m ██╗     \033[93m ██╗\033[32m ████████╗\033[36m ██╗  ██╗\033[34m ███████╗\033[35m ██████╗ ",
    "\033[31m ██╔════╝\033[91m ██║     \033[93m ██║\033[32m ╚══██╔══╝\033[36m ██║  ██║\033[34m ██╔════╝\033[35m ██╔══██╗ ",
    "\033[31m ███████╗\033[91m ██║     \033[93m ██║\033[32m    ██║   \033[36m ███████║\033[34m █████╗  \033[35m ██████╔╝ ",
    "\033[31m ╚════██║\033[91m ██║     \033[93m ██║\033[32m    ██║   \033[36m ██╔══██║\033[34m ██╔══╝  \033[35m ██╔══██╗ ",
    "\033[31m ███████║\033[91m ███████╗\033[93m ██║\033[32m    ██║   \033[36m ██║  ██║\033[34m ███████╗\033[35m ██║  ██║ ",
    "\033[31m ╚══════╝\033[91m ╚══════╝\033[93m ╚═╝\033[32m    ╚═╝   \033[36m ╚═╝  ╚═╝\033[34m ╚══════╝\033[35m ╚═╝  ╚═╝ "
};

// black white empty
string piece[] = {"\033[40m    \033[43m", "\033[47m    \033[43m", "    "};

string serv_cat[] = {
    "  |\\___/|",
    "  ) ^Y^ ( ",
    " =\\  ^  /=",
    "   )=*=(",
    " /       \\",
    " |       |",
    "/| |   | |\\",
    "\\| |   |_|/\\",
    "//_// _____/",
    "    \\_)"
};

string cli_cat[] = {
    "  |\\___/|",
    "  )     (",
    " =\\     /=",
    "   )===(",
    " /       \\",
    " |       |",
    "/   \033[32mYOU\033[30m   \\",
    "\\         /",
    " \\___  __/",
    "    ( (",
    "     ) )",
    "    (_("
};

string sleep_cat[] = {
    "  |\\      _,,,---,,_",
    "  /,`.-'`'    -.  ;-;;,_",
    " |,4-  ) )-,_..;\\ (  `'-'",
    "'---''(_/--'  `-'\\_)"
};

string playing_cat[] = {
"              _ |\\_",
"              \\` ..\\",
"         __,.-\" =__Y=",
"       .\"        )",
" _    /   ,    \\/\\_  _",
"((____|    )_-\\ \\_-`(_)",
"`-----'`-----` `--`",
};

string loading_cat[] = {
    "                         ,",
    "  ,-.       _,---._ __  / \\",
    " /  )    .-'       `./ /   \\",
    "(  (   ,'            `/    /|",
    " \\  `-\"             \\'\\   / |",
    "  `.              ,  \\ \\ /  |",
    "   /`.          ,'-`----Y   |",
    "  (            ;        |   '",
    "  |  ,-.    ,-'         |  /",
    "  |  | (   |            | /",
    "  )  |  \\  `.___________|/",
    "  `--'   `--'"
};

string list[] = {
    "     ______________________________",
    "   ./                             / \\",
    "   .|                            |   |",
    "   .|                            | _/",
    "   .|                            |",
    "   .|                            |",
    "   .|                            |",
    "   .|                            |",
    "   .|                            |",
    "   .|                            |",
    "   .|                            |",
    "   .|                            |",
    "   .|                            |",
    "   .|                            |",
    " ___|_________________________   |",
    ".\\                            \\  |",
    " .\\____________________________\\_/"
};

void init() {
    // set title name
    printf("\033]0;Slither\007");
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
    // reset gray background
    printf("\033[100m");
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
    // reset gray background
    printf("\033[100m");

    printf("\033[8H");
    for (const auto c: serv_cat) {
        printf("\033[0K\033[95C%s\n", c.c_str());
    }

    // set white background
    printf("\033[10H\033[47m");
    for (int i=0; i<8; i++) {
        printf("\033[36C");
        for (int j=0; j<44; j++) {
            printf(" ");
        }
        printf("\n");
    }
    printf("\033[11;81H > \n");
    // reset gray background
    printf("\033[100m");
    printf("\033[11H");
    return;
}

void printServMsg(string msg) {
    istringstream iss(msg);
    string line;

    // set white background
    printf("\033[47m");

    while (getline(iss, line)) {
        printf("\033[40C%s\n", line.c_str());
    }
    // reset gray background
    printf("\033[100m");
    // make cursor invisible
    printf("\033[?25l");
    return;
}

void printList(string msg) {
    // reset gray background
    printf("\033[100m");

    printf("\033[13H");
    for (const auto l: list) {
        printf("\033[80C%s\n", l.c_str());
    }
    istringstream iss(msg);
    string line;

    printf("\033[16H");
    while (getline(iss, line)) {
        printf("\033[86C%s\n", line.c_str());
    }
    return;
}

void printCli() {
    // reset gray background
    printf("\033[100m");

    printf("\033[18H");
    for (const auto c: cli_cat) {
        printf("\033[15C%s\n", c.c_str());
    }
    // set white background
    printf("\033[20H\033[47m");
    for (int i=0; i<3; i++) {
        printf("\033[36C");
        for (int j=0; j<20; j++) {
            printf(" ");
        }
        printf("\n");
    }
    printf("\033[21;34H < \n");

    // move cursor to input
    printf("\033[21;41H");
    // make cursor visible
    printf("\033[?25h");
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