#include "slither.h"
#include "ansi.h"

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

using namespace std;

void game(Player player) {
    State state;
    printBoard(state.get_board());

    // BLACK
    if (player == 0) {

    }
    // WHITE
    else if (player == 1) {

    }
    // Spectator
    else {

    }
    
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
		printf("Usage: ./client <IPaddress>");
        return 0;
    }

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT + 3);
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0) {
        printf("Connection error\n");
        return 0;
    }

    // Connect successfully!
    read(sockfd, recvline, MAXLINE);

    init();
    printServ();
    printServMsg("Welcome to Slither!"); printf("\n");
    printServMsg("Type \033[93mC\033[30m to create a new room.");
    printServMsg("Type \033[93mE\033[30m to enter a room.");

    printCli();
    int flag = 0;

    while (fgets(sendline, MAXLINE, stdin) != NULL) {
        if (flag == 0 && strcmp(sendline, "C\n") == 0) {
            write(sockfd, sendline, strlen(sendline));
            n = read(sockfd, recvline, MAXLINE);
            recvline[n] = 0;
            if (strcmp(recvline, "Too many game rooms!\n") == 0) {
                printServ();
                printServMsg(recvline); printf("\n");
                printServMsg("Type \033[93mB\033[30m to return.");
                flag = 2;
            }
            else {
                game(0);
            }
        }
        else if (flag == 0 && strcmp(sendline, "E\n") == 0) {
            write(sockfd, sendline, strlen(sendline));
            n = read(sockfd, recvline, MAXLINE);
            recvline[n] = 0;
            if (strcmp(recvline, "\n") == 0) {
                printServ();
                printServMsg("There is no avaliable room."); printf("\n");
                printServMsg("Please try again later");
                printServMsg("or create a new room yourself."); printf("\n");
                printServMsg("Type \033[93mB\033[30m to return.");
                flag = 2;
            }
            else {
                printServ();
                printServMsg("Type \033[93m0\033[30m - \033[93m9\033[30m to enter the room by \033[34mID\033[30m");
                printServMsg("or type \033[93mR\033[30m to enter a random room."); printf("\n");
                printServMsg("\033[34mRoom ID\033[30m  Players  Viewers");
                printServMsg(recvline); printf("\n");
                printServMsg("Type \033[93mB\033[30m to return.");
                flag = 1;
            }
        }
        else if (flag == 1 && (isdigit(sendline[0]) || strcmp(sendline, "R\n") == 0)) {
            write(sockfd, sendline, strlen(sendline));
            n = read(sockfd, recvline, MAXLINE);
            recvline[n] = 0;
            if (strcmp(recvline, "Game Start!\n") == 0) {
                game(1);
            }
            else if (strcmp(recvline, "A gentleman should keep silent while watching.\n") == 0) {
                game(2);
            }
            else {
                printServ();
                printServMsg(recvline); printf("\n");
                printServMsg("Type \033[93mB\033[30m to return.");
                flag = 2;
            }
        }
        else if (strcmp(sendline, "B\n") == 0) {
            flag = 0;
            printServ();
            printServMsg("Welcome to Slither!"); printf("\n");
            printServMsg("Type \033[93mC\033[30m to create a new room.");
            printServMsg("Type \033[93mE\033[30m to enter a room.");
        }
        printCli();
    }


    return 0;
}