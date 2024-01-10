#include "slither.h"
#include "ansi.h"
#include "readline.h"

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
#include <algorithm>
#include <unordered_map>

#define SERV_PORT 7122
#define MAXLINE 4096
#define LISTENQ 1024
#define SA struct sockaddr

using namespace std;

int sockfd, n;
char sendline[MAXLINE], recvline[MAXLINE];
pthread_t timer_thread;
bool timerStop;

void *timer(void *arg) {
    int m, s;
    for (int sec = 90; sec >= 0; sec--) {
        if (timerStop) pthread_exit(NULL);
        m = sec / 60;
        s = sec % 60;
        printf("\033[26;40H\033[0K\033[5;31m%02d : %02d\033[25;30m\n", m, s);
        sleep(1);
    }
    pthread_exit(NULL);
}

void game(Player player) {
	// fd_set rset;
    // int maxfdp1;
    // bool stdinEOF, peerExit;
    vector<string> playerID(2), viewerID;

    printSlither();
    printServ();
    printServMsg("Type your name to start the game.");
    printCli();
    bool inRoom = false;
    while (fgets(sendline, MAXLINE, stdin) != NULL) {
        write(sockfd, sendline, strlen(sendline));

        // Waiting... or Game start
        if ((n = readline(sockfd, recvline, MAXLINE)) <= 0) {
            printLoading();
            printf("\033[1A\033[0K\033[50CConnection error\n");
            return;
        }
        recvline[n] = 0;

        if (strcmp(recvline, "Duplicate\n") == 0) {
            printSlither();
            printServ();
            printServMsg("Type your name to start the game.");
            printServMsg("It's already used!\nPlease try another name.");
            printCli();
        }
        else {
            inRoom = true;
            break;
        }
    }
    if (!inRoom) return;

    State cur, m, p;

    if (player == 0) {
        printBoard(cur.get_board());
        printBoardPlayer();
        // Waiting for the second player...
        printf("%s", recvline);
    }

    // players' ID
    if ((n = readline(sockfd, recvline, MAXLINE)) <= 0) {
        printLoading();
        printf("\033[1A\033[0K\033[50CConnection error\n");
        return;
    }
    recvline[n] = 0;
    istringstream iss(recvline);
    iss >> playerID[0];
    iss >> playerID[1];

    // Someone's turn!
    if ((n = readline(sockfd, recvline, MAXLINE)) <= 0) {
        printLoading();
        printf("\033[1A\033[0K\033[50CConnection error\n");
        return;
    }
    recvline[n] = 0;
    printBoard(cur.get_board());

    // stdinEOF = false;
    // peerExit = false;
    // for( ; ; ){
    //     FD_ZERO(&rset);
	// 	maxfdp1 = 0;
    //     if (!stdinEOF) {
    //         FD_SET(STDIN_FILENO, &rset);
	// 		maxfdp1 = STDIN_FILENO;
	// 	}
	// 	if (!peerExit) {
	// 		FD_SET(sockfd, &rset);
	// 		maxfdp1 = max(maxfdp1, sockfd);
	// 	}
    //     maxfdp1++;
    //     Select(maxfdp1, &rset, NULL, NULL, NULL);

    // }
    while (1) {
        vector<Action> toMove, toPlace;
        // Players
        if (player == 0 || player == 1) {
            if (strcmp(recvline, "Your turn!\n") == 0) {
                printBoardPlayers(true, player, playerID);
                printf("Move _ to _ : ");

                timerStop = false;
                pthread_create(&timer_thread, NULL, timer, NULL);

                // move
                while (fgets(sendline, MAXLINE, stdin) != NULL) {
                    toMove = m.string_to_action(sendline);
                    if (toMove.size() != 2) {
                        printf("\033[28;40HIllegal action!\n");
                        printBoardPlayers(true, player, playerID);
                        printf("Move _ to _ : ");
                        continue;
                    }
                    bool illegal = false;
                    for (auto action: toMove) {
                        vector<Action> legalActions = m.legal_actions();
                        if (std::find(legalActions.begin(), legalActions.end(), action) != legalActions.end()) {
                            m.apply_action(action);
                        }
                        else {
                            printf("\033[28;40HIllegal action!\n");
                            printBoardPlayers(true, player, playerID);
                            printf("Move _ to _ : ");
                            illegal = true;
                            break;
                        }
                    }
                    if (illegal) {
                        m = cur;
                        continue;
                    }
                    printBoard(m.get_board());
                    printBoardPlayers(true, player, playerID);
                    printf("Place _ (or reset move): ");
                    break;
                }
                p = m;

                // place
                bool reset = false;
                while (fgets(sendline, MAXLINE, stdin) != NULL) {
                    if (strcmp(sendline, "reset\n") == 0) {
                        reset = true;
                        break;
                    }
                    toPlace = p.string_to_action(sendline);
                    if (toPlace.size() != 1) {
                        printf("\033[28;40HIllegal action!\n");
                        printBoardPlayers(true, player, playerID);
                        printf("Place _ (or reset move): ");
                        continue;
                    }
                    vector<Action> legalActions = p.legal_actions();
                    if (std::find(legalActions.begin(), legalActions.end(), toPlace[0]) != legalActions.end()) {
                        p.apply_action(toPlace[0]);
                    }
                    else {
                        printf("\033[28;40HIllegal action!\n");
                        printBoardPlayers(true, player, playerID);
                        printf("Place _ (or reset move): ");
                        p = m;
                        continue;
                    }
                    printBoard(p.get_board());
                    printBoardPlayers(true, player, playerID);
                    break;
                }
                if (reset) {
                    p = m = cur;
                    printBoard(cur.get_board());
                    continue;
                }
                sprintf(sendline, "%d %d %d\n", toMove[0], toMove[1], toPlace[0]);
                write(sockfd, sendline, strlen(sendline));
                timerStop = true;
                pthread_join(timer_thread, NULL);
            }
            else {
                printBoardPlayers(false, player, playerID);
                printf("%s", recvline);
            }
        }
        // Viewer
        else {

        }

        // update board
        if ((n = readline(sockfd, recvline, MAXLINE)) <= 0) {
            printLoading();
            printf("\033[1A\033[0K\033[50CConnection error\n");
            return;
        }
        recvline[n] = 0;
        cur.set_board(string(recvline));
        p = m = cur;
        printBoard(cur.get_board());

        // next turn
        if ((n = readline(sockfd, recvline, MAXLINE)) <= 0) {
            printLoading();
            printf("\033[1A\033[0K\033[50CConnection error\n");
            return;
        }
        recvline[n] = 0;
    }
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
    if ((n = readline(sockfd, recvline, MAXLINE)) <= 0) {
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
            if ((n = readline(sockfd, recvline, MAXLINE)) <= 0) {
                printLoading();
                printf("\033[1A\033[0K\033[50CConnection error\n");
                return 0;
            }
            recvline[n] = 0;
            if (strcmp(recvline, "Player1\n") == 0) {
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
            if ((n = readline(sockfd, recvline, MAXLINE)) <= 0) {
                printLoading();
                printf("\033[1A\033[0K\033[50CConnection error\n");
                return 0;
            }
            recvline[n] = 0;
            if (strcmp(recvline, "Player2\n") == 0) {
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