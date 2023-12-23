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

void game() {
    State state;
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

    // Connect sucessfully
    read(sockfd, recvline, MAXLINE);

    init();
    printServ();
    printServMsg("Welcome to Slither!\n");
    printServMsg("Type \"Create Room\" to create a new room.");
    printServMsg("Type \"Enter Room\" to enter a room.\n");

    printCli();

    while (fgets(sendline, MAXLINE, stdin) != NULL) {
        if (strcmp(sendline, "Create Room\n") == 0) {
            write(sockfd, sendline, strlen(sendline));
            n = read(sockfd, recvline, MAXLINE);
            recvline[n] = 0;
            printf("\n%s", recvline);

        }
        else if (strcmp(sendline, "Enter Room\n") == 0) {
            if (write(sockfd, sendline, strlen(sendline)) < strlen(sendline)) {
                printf("%d\n", errno);
            }
            n = read(sockfd, recvline, MAXLINE);
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
        }
    }


    return 0;
}