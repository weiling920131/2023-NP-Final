#include	"unp.h"
#include    <pthread.h>
#include    <iostream>
#include    <vector>
#include    <string>
#include    <algorithm>
#include    <cctype>

using namespace std;

int MAX_CLIENT = 20;
int MAX_CHATROOM = 10;
int MAX_VIEWER = 10;
int cur_room = 0;
pthread_t my_thread[MAXCHATROOM];
vector<vector<int>> players_fd(MAX_CHATROOM);
vector<vector<int>> viewers_fd(MAX_CHATROOM);

void
sig_chld(int signo)
{
        pid_t   pid;
        int             stat;

        while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0)
                ;
        return;
}

void *game_room(int room_id){
    
}

// void Lobby(int connfd){
//     int n;
//     char recvline[MAXLINE], sendline[MAXLINE];
//     // Let client choose to create or enter a room
//     Writen(connfd, "Type \"Create Room\" to create a new room\n\nType \"Enter Room\" to enter a random/the specific room.\n", 96);
//     n = Read(connfd, recvline, MAXLINE);
//     recvline[n] = 0;
//     if(strcmp(recvline, "Create Room\n") == 0){
//         if(cur_room >= MAXCHATROOM){
//             Writen(connfd, "Too many chat rooms!\n", 21);
//             continue;
//         }
//         else{
//             Writen(connfd, "Waiting for the second player...\n", 32);
//             players_fd[cur_room].push_back(connfd);
//             pthread_create(&my_thread[cur_room], NULL, game_room, cur_room);
//             cur_room++;
//         }
//     }
//     else if(strcmp(recvline, "Enter Room\n") == 0){
//         Writen(connfd, "Type \"random\" to enter a random room\n\nType (0 - 9) to enter the room\n", 69);
//         n = Read(connfd, recvline, MAXLINE);
//         recvline[n] = 0;
//         if(strcmp(recvline, "random\n") == 0){    //Random enter a room

//         }
//         else if (isdigit(recvline[0])){           // Is a number
//             int room_id = recvline[0] - '0';
//             if(room_id >= MAX_CHATROOM || room_id < 0){
//                 Writen(connfd, "Invalid room id!\n", 17);
//                 continue;
//             }
//             else if(players_fd[room_id].size() == 0){ // Enter an empty room
//                 Writen(connfd, "Empty room!\n", 12);
//                 continue;
//             }
//             else if(players_fd[room_id].size() == 1){ // Enter a room with one player
//                 Writen(connfd, "Waiting for the second player...\n", 32);
//                 players_fd[room_id].push_back(connfd);
//                 pthread_create(&my_thread[room_id], NULL, game_room, room_id);
//             }
//             else if(players_fd[room_id].size() == 2){ // Enter a room as a viewer
//                 Writen(connfd, "Room is full!\n", 14);
//                 continue;
//             }
//         }
//         else{

//         }
//     }
//     else{
//         Writen(connfd, "Invalid input!\n", 15);
//         continue;
//     }
// }

int main(int argc, char **argv){
	int					listenfd, cur_room = 0;
    pid_t               childpid;
	struct sockaddr_in	servaddr;

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT + 3);

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

    Signal(SIGCHLD, sig_chld);

    vector<int>                 connfd(MAX_CLIENT, -1);
    vector<struct sockaddr_in>	cliaddr(MAX_CLIENT);
    vector<socklen_t>			clilen(MAX_CLIENT, sizeof(cliaddr[0]));
    int                         maxfdp1, n;
	fd_set		                rset;

    for ( ; ; ) { //Deal with socket
        char recvline[MAXLINE], sendline[MAXLINE];

	    FD_ZERO(&rset);
        FD_SET(listenfd, &rset);
        int max = listenfd;
		for(int i = 0;i<MAX_CLIENT;i++) {
            if(connfd[i] != -1) {
                FD_SET(connfd[i], &rset);
                max = max(max, connfd[i]);
            }
        }
        maxfdp1 = max + 1;
		Select(maxfdp1, &rset, NULL, NULL, NULL);

        // Accept client 
        if(FD_ISSET(listenfd, &rset)){
            auto it = find(connfd.begin(), connfd.end(), -1);
            int available = distance(connfd.begin(), it); 
            
            if(available >= MAX_CLIENT){ // Too many clients
                printf("Too many clients!\n");
            }
            else{ //setting
                connfd[available] = Accept(listenfd, (SA *) &cliaddr[available], &clilen[available]);
            }
        }

        // Deal with client with every cases
        for(int i = 0;i<MAX_CLIENT;i++){
            if(connfd[i] != -1 && FD_ISSET(connfd[i], &rset)){
                n = Read(connfd[i], recvline, MAXLINE);
                
                // Client close
                if(n == 0){ 
                    connfd[i] = -1;
                    bzero(&cliaddr[i], sizeof(cliaddr[i]));
                    bzero(&clilen[i], sizeof(clilen[i]));
                    continue;
                }
                recvline[n] = 0;

                // Deal with client's input
                if(strcmp(recvline, "Create Room\n") == 0){
                    if(cur_room >= MAX_CHATROOM){
                        Writen(connfd[i], "Too many game rooms!\n", 21);
                        continue;
                    }
                    else{
                        Writen(connfd[i], "Waiting for the second player...\n", 32);
                        players_fd[cur_room].push_back(connfd[i]);
                        pthread_create(&my_thread[cur_room], NULL, game_room, cur_room);
                        cur_room++;
                    }
                }
                else if(strcmp(recvline, "Enter Room\n") == 0){ // Send room list
                    bzero(sendline, MAXLINE);
                    for(int room_id = 0;room_id<MAX_CHATROOM;room_id++){
                        if(players_fd[room_id].size() == 0) continue;
                        sprintf(sendline, "%s %d %d %d", sendline, room_id, players_fd[room_id].size(), viewers_fd[room_id].size());
                    }
                    Writen(connfd[i], sendline, strlen(sendline));
                }
                else if(strcmp(recvline, "random\n") == 0){
                    for(int room_id = 0;room_id<MAX_CHATROOM;room_id++){
                        if(players_fd[room_id].size() == 1){
                            Writen(connfd[i], "Game Start!", 11);
                            players_fd[room_id].push_back(connfd[i]);
                            break;
                        }
                    }
                }
                else if(isdigit(recvline[0])){ // Enter the room, client should check if the number is 0-9 and is listed on the screen
                    int room_id = recvline[0] - '0';
                    if(players_fd[room_id].size() == 1){ // Enter a room with one player
                        Writen(connfd[i], "Game Start!", 11);
                        players_fd[room_id].push_back(connfd[i]);
                    }
                    else if(players_fd[room_id].size() == 2){ // Enter a room as a viewer
                        if(viewers_fd[room_id].size() >= MAX_VIEWER){
                            Writen(connfd[i], "Too many viewers!\n", 19);
                            continue;
                        }
                        Writen(connfd[i], "A gentleman should keep silent while watching.\n", 47);
                        viewers_fd[room_id].push_back(connfd[i]);
                    }
                }
                else{
                    Writen(connfd[i], "Invalid input!\n", 15);
                    continue;
                }
            }
        }
    }

    // for(;;){
    //     int  connfd;
    //     struct sockaddr_in	cliaddr;
    //     socklen_t			clilen = sizeof(cliaddr);
    //     connfd = Accept(listenfd, (SA *) &cliaddr, &clilen);
    //     if(childpid = Fork() == 0){
    //         Close(listenfd);
    //         Lobby(connfd);
    //         exit(0);
    //     }
    //     close(connfd);
    // }
}
