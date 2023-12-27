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

int sockfd, n;
char sendline[MAXLINE], recvline[MAXLINE];

void game(Player player) {
    printSlither();
    printServ();
    printServMsg("Type your name to start the game.");
    printCli();
    bool inRoom = false;
    while (fgets(sendline, MAXLINE, stdin) != NULL) {
        write(sockfd, sendline, strlen(sendline));
        printLoading();
        if ((n = read(sockfd, recvline, MAXLINE)) <= 0) {
            printf("\033[1A\033[0K\033[50CConnection error\n");
            return;
        }
        recvline[n] = 0;

        if (strcmp(recvline, "Duplicate\n") == 0) {
            printSlither();
            printServ();
            printServMsg("Type your name to start the game.");
            printServMsg("It's already used! Please try another name.");
            printCli();
        }
        else {
            inRoom = true;
            break;
        }
    }
    if (!inRoom) return;

    State state;
    printBoard(state.get_board());

    while (1) {
        // BLACK
        if (player == 0) {
            if (strcmp(recvline, "Your turn!\n") == 0) {
                printBoardPlayers(true, player);
                while (fgets(sendline, MAXLINE, stdin) != NULL) {

                }
            }
            else if (strcmp(recvline, "Waiting for the second player...\n") == 0) {
                printBoardPlayer();
                printf("%s", recvline);
                // Your turn!
                if ((n = read(sockfd, recvline, MAXLINE)) <= 0) {
                    printLoading();
                    printf("\033[1A\033[0K\033[50CConnection error\n");
                    return;
                }
                recvline[n] = 0;
                continue;
            }
            else {
                printBoardPlayers(false, player);
            }
        }
        // WHITE
        else if (player == 1) {
            if (strcmp(recvline, "Your turn!\n") == 0) {
                printBoardPlayers(true, player);
            }
            else {
                printBoardPlayers(false, player);
            }
        }
        // Viewer
        else {

        }

        // update board
        if ((n = read(sockfd, recvline, MAXLINE)) <= 0) {
            printLoading();
            printf("\033[1A\033[0K\033[50CConnection error\n");
            return;
        }
        recvline[n] = 0;
        // state.set_board();
        // printBoard();
        // next turn
        if ((n = read(sockfd, recvline, MAXLINE)) <= 0) {
            printLoading();
            printf("\033[1A\033[0K\033[50CConnection error\n");
            return;
        }
        recvline[n] = 0;
    }
    // vector<Action> play;
    // char input[10];
    // while (1) {
    //     // erase previous input
    //     printf("\033[26;37H\033[0K");
    //     fgets(input, sizeof(input), stdin);
    //     // move cursor to middle
    //     printf("\033[27;37H\033[0K");

    //     play = state.string_to_action(input);
    //     if (play.size() != 3) {
    //         printf("illegal move!\n");
    //         continue;
    //     }
    //     else {
    //         printf("%d %d %d\n", play[0], play[1], play[2]);
    //     }
    //     // printBoard(state.get_board());
    // }
}

int main(int argc, char **argv) {
	struct sockaddr_in	servaddr;

	if (argc != 2) {
		printf("Usage: ./client <IPaddress>");
        return 0;
    }

    init();
    printLoading();

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT + 3);
	inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0) {
        printf("\033[1A\033[0K\033[50CConnection error\n");
        return 0;
    }

    // Connect successfully!
    if ((n = read(sockfd, recvline, MAXLINE)) <= 0) {
        printf("\033[1A\033[0K\033[50CConnection error\n");
        return 0;
    }

    printSlither();
    printServ();
    printServMsg("Welcome to Slither!"); printf("\n");
    printServMsg("Type \033[32mC\033[30m to create a new room.\nType \033[32mE\033[30m to enter a room.");

    printCli();
    // 0: C or E; 1: 0-9 or R; 2: deadend;
    int flag = 0;

    while (fgets(sendline, MAXLINE, stdin) != NULL) {
        if (flag == 0 && strcmp(sendline, "C\n") == 0) {
            write(sockfd, sendline, strlen(sendline));
            if ((n = read(sockfd, recvline, MAXLINE)) <= 0) {
                printLoading();
                printf("\033[1A\033[0K\033[50CConnection error\n");
                return 0;
            }
            recvline[n] = 0;
            if (strcmp(recvline, "OK\n") == 0) {
                game(0);
                if (n <= 0) return 0;
            }
            else {
                printServ();
                printServMsg(recvline); printf("\n");
                printServMsg("Type \033[32mB\033[30m to return.");
                flag = 2;
            }
        }
        else if (flag == 0 && strcmp(sendline, "E\n") == 0) {
            write(sockfd, sendline, strlen(sendline));
            if ((n = read(sockfd, recvline, MAXLINE)) <= 0) {
                printLoading();
                printf("\033[1A\033[0K\033[50CConnection error\n");
                return 0;
            }
            recvline[n] = 0;
            if (strcmp(recvline, "Empty\n") == 0) {
                printServ();
                printServMsg("There is no avaliable room."); printf("\n");
                printServMsg("Please try again later\nor create a new room yourself."); printf("\n");
                printServMsg("Type \033[32mB\033[30m to return.");
                flag = 2;
            }
            else {
                printServ();
                printServMsg("Type \033[32m0\033[30m - \033[32m9\033[30m to enter the room by \033[34mID\033[30m");
                printServMsg("or type \033[32mR\033[30m to enter a random room."); printf("\n");
                printServMsg("Type \033[32mB\033[30m to return.");

                string roomlist = "\033[34mRoom ID\033[30m  Players  Viewers\n" + string(recvline);
                printList(roomlist);
                flag = 1;
            }
        }
        else if (flag == 1 && ((isdigit(sendline[0]) && sendline[1] == '\n') || strcmp(sendline, "R\n") == 0)) {
            write(sockfd, sendline, strlen(sendline));
            if ((n = read(sockfd, recvline, MAXLINE)) <= 0) {
                printLoading();
                printf("\033[1A\033[0K\033[50CConnection error\n");
                return 0;
            }
            recvline[n] = 0;
            if (strcmp(recvline, "Player\n") == 0) {
                game(1);
                if (n <= 0) return 0;
            }
            else if (strcmp(recvline, "Viewer\n") == 0) {
                game(2);
                if (n <= 0) return 0;
            }
            else {
                printSlither();
                printServ();
                printServMsg(recvline); printf("\n");
                printServMsg("Type \033[32mB\033[30m to return.");
                flag = 2;
            }
        }
        else if (strcmp(sendline, "B\n") == 0) {
            flag = 0;

            printSlither();
            printServ();
            printServMsg("Welcome to Slither!"); printf("\n");
            printServMsg("Type \033[32mC\033[30m to create a new room.\nType \033[32mE\033[30m to enter a room.");
        }
        printCli();
    }

    return 0;
}