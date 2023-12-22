#include "slither.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <iostream>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <cstring>

#define SERV_PORT 7122
#define MAXLINE 4096
#define LISTENQ 1024
#define SA struct sockaddr

#define BOARDSIZE 5

using namespace std;

// black white empty
const string piece[3] = {"\033[40m    \033[43m", "\033[47m    \033[43m", "    "};

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

        printf(" \n\033[34C");

        if (i < BOARDSIZE) {
            printf("\033[100m%d \033[43m", 5-i);
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
    printf("     A         B        C        D        E\n");
    // move cursor to middle
    printf("\033[36C");
    // make cursor visible
    printf("\033[?25h");
    return;
}

void game() {
    State state;
    init();
    printSlitherMoving();
    printBoard(state.get_board());
    
    vector<Action> play;
    char input[10];
    while (1) {
        // erase previous input
        printf("\033[26;37H\033[0K");
        fgets(input, sizeof(input), stdin);
        // move cursor to middle
        printf("\033[27;37H\033[0K");

        play = state.string_to_action(input);
        if (play.size() != 3) {
            printf("illegal move!\n");
            continue;
        }
        else {
            printf("%d %d %d\n", play[0], play[1], play[2]);
        }
        // printBoard(state.get_board());
    }
}

int main(int argc, char **argv) {
    int					sockfd, n;
	struct sockaddr_in	servaddr;
    char sendline[MAXLINE], recvline[MAXLINE];

	if (argc != 2) {
		printf("usage: ./client <IPaddress>");
        return 0;
    }

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT+3);
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0) {
        printf("QQ\n");
    }

    printf("Type \"Create Room\" to create a new room\nType \"Enter Room\" to enter a random/the specific room.\n\n");
    while (fgets(sendline, MAXLINE, stdin) != NULL) {
        if (strcmp(sendline, "Create Room\n") == 0) {
            write(sockfd, sendline, strlen(sendline));
            n = read(sockfd, recvline, MAXLINE);
            recvline[n] = 0;
            printf("\n%s", recvline);

        }
        else if (strcmp(sendline, "Enter Room\n") == 0) {
            printf("0\n");
            printf("socket: %d sendline: %s, strlen: %d\n", sockfd, sendline, strlen(sendline));
            if (write(sockfd, sendline, strlen(sendline)) < strlen(sendline)) {
                printf("%d\n", errno);
            }
            printf("1\n");
            n = read(sockfd, recvline, MAXLINE);
            printf("2\n");
            recvline[n] = 0;
            // if (strcmp(recvline, "(There is no room avaliable. Please try again later or create a new room yourself.)\n") != 0) {
            //     printf("\nType \"random\" to enter a random room\nType (0 - 9) to enter the room\n");
            //     printf("%s", recvline);
            // }
            // else {
                printf("%s", recvline);
                fgets(sendline, MAXLINE, stdin);
                write(sockfd, sendline, strlen(sendline));
                n = read(sockfd, recvline, MAXLINE);
                recvline[n] = 0;
                printf("%s", recvline);
            // }
        }
        else {
            printf("3\n");
        }
    }


    return 0;
}