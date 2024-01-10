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
pthread_t timerThread;
bool timerStop, timesup;

void *timer(void *arg) {
    timesup = false;
    int m, s;
    for (int sec = 90; sec >= 0; sec--) {
        if (timerStop) pthread_exit(NULL);
        m = sec / 60;
        s = sec % 60;
        printf("\033]0;%02d:%02d\007\n", m, s);
        sleep(1);
    }
    timesup = true;
    printf("\033]0;Slither\007\n");
    pthread_exit(NULL);
}

void game(Player player) {
	fd_set rset;
    int maxfdp1, flag;
    bool inRoom, hasSpace;
    vector<string> playerID(2), viewerID;
    State cur, m, p;
    vector<Action> toMove, toPlace;

    struct timeval myTimeval;
    myTimeval.tv_sec = 3;  
    myTimeval.tv_usec = 0;

    printSlither();
    printServ();
    printServMsg("Type your name to start the game.");
    printCli();
    // input ID
    while (fgets(sendline, MAXLINE, stdin) != NULL) {
        inRoom = false;
        hasSpace = false;
        for (int i=0; i<strlen(sendline); i++) {
            if (sendline[i] == ' ') {
                printSlither();
                printServ();
                printServMsg("Type your name to start the game.");
                printServMsg("Your name cannot contain spaces!\nPlease try another name.");
                printCli();
                hasSpace = true;
                break;
            }
        }
        if (hasSpace) continue;
        write(sockfd, sendline, strlen(sendline));

        // Waiting for the second player...
        if ((n = readline(sockfd, recvline, MAXLINE)) <= 0) {
            printLoading();
            printf("\033[1A\033[0K\033[50CConnection error\n");
            printf("\033[29H");
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

    // Waiting for the second player...
    if (player == 0 || player == 1) {
        printBoard(cur.get_board());
        printBoardPlayer();
        printf("%s", recvline);
    }
    // board
    else {
        cur.set_board(string(recvline));
    }

    // players' ID
    if ((n = readline(sockfd, recvline, MAXLINE)) <= 0) {
        printLoading();
        printf("\033[1A\033[0K\033[50CConnection error\n");
        printf("\033[29H");
        return;
    }
    recvline[n] = 0;
    istringstream iss(recvline);
    iss >> playerID[0];
    iss >> playerID[1];

    string  blackTurn = playerID[0] + "'s turn!\n", 
            whiteTurn = playerID[1] + "'s turn!\n";

    flag = 0;
    for ( ; ; ) {
        FD_ZERO(&rset);
		maxfdp1 = 0;
        FD_SET(sockfd, &rset);
        maxfdp1 = sockfd;
        FD_SET(STDIN_FILENO, &rset);
        maxfdp1 = max(maxfdp1, STDIN_FILENO);
		
        maxfdp1++;
        select(maxfdp1, &rset, NULL, NULL, &myTimeval);

        if (FD_ISSET(sockfd, &rset)) {  /* socket is readable */
            if ((n = readline(sockfd, recvline, MAXLINE)) <= 0) {
                printLoading();
                printf("\033[1A\033[0K\033[50CConnection error\n");
                printf("\033[29H");
                return;
            }
            recvline[n] = 0;

            if (strlen(recvline) > strlen("win!\n") && string(recvline).substr(strlen(recvline) - strlen("win!\n")) == "win!\n") {
                if (flag == 1 || flag == 2) {
                    timerStop = true;
                    pthread_join(timerThread, NULL);
                }
                printBoard(cur.get_board());
                printBoardPlayers(true, 0, playerID);
                printf("%s", recvline);
                sleep(2);
                return;
            }
			// Players
            if (player == 0 || player == 1) {
                if (strcmp(recvline, "Your turn!\n") == 0) {
                    printBoard(cur.get_board());
                    printBoardPlayers(true, player, playerID);
                    printf("Move _ to _ : ");

                    timerStop = false;
                    timesup = false;
                    pthread_create(&timerThread, NULL, timer, NULL);
                    flag = 1;
                }
                // other's turn
                else if (flag == 0){
                    printBoard(cur.get_board());
                    printBoardPlayers(false, player, playerID);
                    printf("%s", recvline);
                    flag = 3;
                }
                // update board
                else if (flag == 3) {
                    cur.set_board(string(recvline));
                    p = m = cur;
                    flag = 0;
                }
            }
            // Viewers
            else {
                // black's turn
                if (strcmp(recvline, blackTurn.c_str()) == 0) {
                    printBoard(cur.get_board());
                    printBoardPlayers(true, 0, playerID);
                    printf("A gentleman should keep silent while watching. ");
                    flag = 3;
                }
                else if (strcmp(recvline, whiteTurn.c_str()) == 0) {
                    printBoard(cur.get_board());
                    printBoardPlayers(false, 0, playerID);
                    printf("A gentleman should keep silent while watching. ");
                    flag = 3;
                }
                // update board
                else if (flag == 3) {
                    cur.set_board(string(recvline));
                    p = m = cur;
                    flag = 0;
                }
            }
        }

        if (timesup) {
            timesup = false;
            printf("\033[28;40HTime's up!\n");
            cur.apply_action(25);
            cur.apply_action(25);
            sprintf(sendline, "%d %d %d\n", 25, 25, cur.legal_actions()[0]);
            write(sockfd, sendline, strlen(sendline));
            flag = 3;
        }
		
        if (FD_ISSET(STDIN_FILENO, &rset)) {  /* input is readable */
            if (fgets(sendline, MAXLINE, stdin) != NULL) {
                // Players
                if (player == 0 || player == 1) {
                    if (strcmp(sendline, "exit\n") == 0) {
                        write(sockfd, sendline, strlen(sendline));
                        continue;
                    }
                    // move
                    if (flag == 1) {
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
                        printf("Place _ : ");
                        p = m;
                        flag = 2;
                    }
                    // place
                    else if (flag == 2) {
                        if (strcmp(sendline, "reset\n") == 0) {
                            p = m = cur;
                            printBoard(cur.get_board());
                            printBoardPlayers(true, player, playerID);
                            printf("Move _ to _ : ");
                            flag = 1;
                            continue;
                        }
                        toPlace = p.string_to_action(sendline);
                        if (toPlace.size() != 1) {
                            printf("\033[28;40HIllegal action!\n");
                            printBoardPlayers(true, player, playerID);
                            printf("Place _ : ");
                            continue;
                        }
                        vector<Action> legalActions = p.legal_actions();
                        if (std::find(legalActions.begin(), legalActions.end(), toPlace[0]) != legalActions.end()) {
                            p.apply_action(toPlace[0]);
                        }
                        else {
                            printf("\033[28;40HIllegal action!\n");
                            printBoardPlayers(true, player, playerID);
                            printf("Place _ : ");
                            p = m;
                            continue;
                        }
                        printBoard(p.get_board());
                        printBoardPlayers(true, player, playerID);
                        sprintf(sendline, "%d %d %d\n", toMove[0], toMove[1], toPlace[0]);
                        write(sockfd, sendline, strlen(sendline));
                        timerStop = true;
                        pthread_join(timerThread, NULL);
                        flag = 3;
                    }
                }
                // Viewers
                else {
                    if (strcmp(sendline, "exit\n") == 0) {
                        write(sockfd, sendline, strlen(sendline));
                        return;
                    }
                }
            }
            else {
                return;
            }
        }
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
        printf("\033[29H");
        return 0;
    }

    // Connect successfully!
    if ((n = readline(sockfd, recvline, MAXLINE)) <= 0) {
        printf("\033[1A\033[0K\033[50CConnection error\n");
        printf("\033[29H");
        return 0;
    }

    printSlither();
    printServ();
    printServMsg("Welcome to Slither!"); printf("\n");
    printServMsg("Type \033[32mC\033[30m to create a new room.\nType \033[32mE\033[30m to enter a room."); printf("\n");
    printServMsg("Type \033[32mexit\033[30m to quit the game.");

    printCli();
    // 0: C or E; 1: 0-9 or R; 2: deadend;
    int flag = 0;

    while (fgets(sendline, MAXLINE, stdin) != NULL) {
        if (flag == 0 && strcmp(sendline, "C\n") == 0) {
            write(sockfd, sendline, strlen(sendline));
            if ((n = readline(sockfd, recvline, MAXLINE)) <= 0) {
                printLoading();
                printf("\033[1A\033[0K\033[50CConnection error\n");
                printf("\033[29H");
                return 0;
            }
            recvline[n] = 0;
            if (strcmp(recvline, "Player1\n") == 0) {
                game(0);
                if (n <= 0) return 0;
                else {
                    flag = 0;
                    printSlither();
                    printServ();
                    printServMsg("Welcome to Slither!"); printf("\n");
                    printServMsg("Type \033[32mC\033[30m to create a new room.\nType \033[32mE\033[30m to enter a room."); printf("\n");
                    printServMsg("Type \033[32mexit\033[30m to quit the game.");
                }
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
                printf("\033[29H");
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
                printf("\033[29H");
                return 0;
            }
            recvline[n] = 0;
            if (strcmp(recvline, "Player2\n") == 0) {
                game(1);
                if (n <= 0) return 0;
                else {
                    flag = 0;
                    printSlither();
                    printServ();
                    printServMsg("Welcome to Slither!"); printf("\n");
                    printServMsg("Type \033[32mC\033[30m to create a new room.\nType \033[32mE\033[30m to enter a room."); printf("\n");
                    printServMsg("Type \033[32mexit\033[30m to quit the game.");
                }
            }
            else if (strcmp(recvline, "Viewer\n") == 0) {
                game(2);
                if (n <= 0) return 0;
                else {
                    flag = 0;
                    printSlither();
                    printServ();
                    printServMsg("Welcome to Slither!"); printf("\n");
                    printServMsg("Type \033[32mC\033[30m to create a new room.\nType \033[32mE\033[30m to enter a room."); printf("\n");
                    printServMsg("Type \033[32mexit\033[30m to quit the game.");
                }
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
            printServMsg("Type \033[32mC\033[30m to create a new room.\nType \033[32mE\033[30m to enter a room."); printf("\n");
            printServMsg("Type \033[32mexit\033[30m to quit the game.");
        }
        else if (strcmp(sendline, "exit\n") == 0) {
            break;
        }
        printCli();
    }

    printSlither();
    printServ();
    printServMsg("Good bye!"); printf("\n");
    printServMsg("Hope to see you again!");
    printf("\033[29H");

    return 0;
}