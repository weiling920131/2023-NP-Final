// #include	"../unpv13e/lib/unp.h"
#include    <sys/wait.h>
#include    <sys/types.h>
#include    <sys/socket.h>
#include    <netinet/in.h>
#include    <arpa/inet.h>
#include    <unistd.h>
#include    <pthread.h>
#include    <iostream>
#include    <vector>
#include    <string>
#include    <algorithm>
#include    <cctype>
#include    <cstring>
#include    <unordered_map>

#define SERV_PORT 7122
#define MAXLINE 4096
#define LISTENQ 1024
#define SA struct sockaddr

using namespace std;

int MAX_CLIENT = 20;
int MAX_CHATROOM = 10;
int MAX_VIEWER = 10;
int cur_room = 0;
// pthread_t my_thread[MAX_CHATROOM];
vector<pthread_t> my_thread(MAX_CHATROOM);
vector<vector<int>> players_fd(MAX_CHATROOM);
vector<vector<int>> viewers_fd(MAX_CHATROOM);

void *game_room(void* room_id_void){
    int                room_id = *(int*)room_id_void;
    unordered_map<int, string>     player_id;
    char                send[MAXLINE], recv[MAXLINE];
    int                 maxfdp1;
	fd_set		        rset;

    for( ; ; ){
        int Max = 0;
        FD_ZERO(&rset);
        for(auto &p:players_fd[room_id]) {
            FD_SET(p, &rset);
            Max = max(Max, p);
        }
        for(auto &v:viewers_fd[room_id]) {
            FD_SET(v, &rset);
            Max = max(Max, v);
        }
        maxfdp1 = Max + 1;
        select(maxfdp1, &rset, NULL, NULL, NULL);

        

    }

}

// void Lobby(int connfd){
//     int n;
//     char recvline[MAXLINE], sendline[MAXLINE];
//     // Let client choose to create or enter a room
//     write(connfd, "Type \"Create Room\" to create a new room\n\nType \"Enter Room\" to enter a random/the specific room.\n", 96);
//     n = Read(connfd, recvline, MAXLINE);
//     recvline[n] = 0;
//     if(strcmp(recvline, "Create Room\n") == 0){
//         if(cur_room >= MAXCHATROOM){
//             write(connfd, "Too many chat rooms!\n", 21);
//             continue;
//         }
//         else{
//             write(connfd, "Waiting for the second player...\n", 32);
//             players_fd[cur_room].push_back(connfd);
//             pthread_create(&my_thread[cur_room], NULL, game_room, cur_room);
//             cur_room++;
//         }
//     }
//     else if(strcmp(recvline, "Enter Room\n") == 0){
//         write(connfd, "Type \"random\" to enter a random room\n\nType (0 - 9) to enter the room\n", 69);
//         n = Read(connfd, recvline, MAXLINE);
//         recvline[n] = 0;
//         if(strcmp(recvline, "random\n") == 0){    //Random enter a room

//         }
//         else if (isdigit(recvline[0])){           // Is a number
//             int room_id = recvline[0] - '0';
//             if(room_id >= MAX_CHATROOM || room_id < 0){
//                 write(connfd, "Invalid room id!\n", 17);
//                 continue;
//             }
//             else if(players_fd[room_id].size() == 0){ // Enter an empty room
//                 write(connfd, "Empty room!\n", 12);
//                 continue;
//             }
//             else if(players_fd[room_id].size() == 1){ // Enter a room with one player
//                 write(connfd, "Waiting for the second player...\n", 32);
//                 players_fd[room_id].push_back(connfd);
//                 pthread_create(&my_thread[room_id], NULL, game_room, room_id);
//             }
//             else if(players_fd[room_id].size() == 2){ // Enter a room as a viewer
//                 write(connfd, "Room is full!\n", 14);
//                 continue;
//             }
//         }
//         else{

//         }
//     }
//     else{
//         write(connfd, "Invalid input!\n", 15);
//         continue;
//     }
// }

int main(int argc, char **argv){
	int					listenfd, cur_room = 0;
    pid_t               childpid;
	struct sockaddr_in	servaddr;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT + 3);

	bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	listen(listenfd, LISTENQ);

    // Signal(SIGCHLD, sig_chld);
    vector<int>                 connfd(MAX_CLIENT, -1);
    vector<struct sockaddr_in>	cliaddr(MAX_CLIENT);
    vector<socklen_t>			clilen(MAX_CLIENT, sizeof(cliaddr[0]));
    int                         maxfdp1, n;
	fd_set		                rset;

    for ( ; ; ) { //Deal with socket
        char recvline[MAXLINE], sendline[MAXLINE];

	    FD_ZERO(&rset);
        FD_SET(listenfd, &rset);
        int Max = listenfd;
		for(int i = 0;i<MAX_CLIENT;i++) {
            if(connfd[i] != -1) {
                FD_SET(connfd[i], &rset);
                Max = max(Max, connfd[i]);
            }
        }
        maxfdp1 = Max + 1;
		select(maxfdp1, &rset, NULL, NULL, NULL);

        // Accept client 
        if(FD_ISSET(listenfd, &rset)){
            std::vector<int>::iterator it = std::find(connfd.begin(), connfd.end(), -1);
            int available = distance(connfd.begin(), it); 
            
            if(available >= MAX_CLIENT){ // Too many clients
                printf("Too many clients!\n");
            }
            else{ //setting
                connfd[available] = accept(listenfd, (SA *) &cliaddr[available], &clilen[available]);
            }
        }

        // Deal with client with every cases
        for(int i = 0;i<MAX_CLIENT;i++){
            if(connfd[i] != -1 && FD_ISSET(connfd[i], &rset)){
                n = read(connfd[i], recvline, MAXLINE);
                
                if(n == 0){ // Client close
                    connfd[i] = -1;
                    bzero(&cliaddr[i], sizeof(cliaddr[i]));
                    bzero(&clilen[i], sizeof(clilen[i]));
                    continue;
                }
                recvline[n] = 0;

                // Deal with client's input
                if(strcmp(recvline, "Create Room\n") == 0){ // Create a room
                    if(cur_room >= MAX_CHATROOM){
                        write(connfd[i], "Too many game rooms!\n", 21);
                        continue;
                    }
                    else{
                        for(int room_id = 0;room_id < MAX_CHATROOM;room_id++){
                            if(players_fd[room_id].size() == 0)
                                write(connfd[i], "Waiting for the second player...\n", 32);
                                players_fd[room_id].push_back(connfd[i]);
                                pthread_create(&my_thread[room_id], NULL, game_room, (void*)&room_id);
                                cur_room++;
                                break;
                        }
                    }
                }

                else if(strcmp(recvline, "Enter Room\n") == 0){ // Send room list
                    bzero(sendline, MAXLINE);
                    string room_list = "";
                    for(int room_id = 0;room_id<MAX_CHATROOM;room_id++){
                        if(players_fd[room_id].size() == 0) continue;
                        room_list += to_string(room_id) + " " + to_string(players_fd[room_id].size()) + " " + to_string(viewers_fd[room_id].size()) + " ";
                    }
                    room_list += "\n";
                    // sendline = room_list.c_str();
                    copy(room_list.begin(), room_list.end(), sendline);
                    sendline[room_list.size()] = 0;
                    write(connfd[i], sendline, strlen(sendline));
                }

                else if(strcmp(recvline, "random\n") == 0){ // Enter a random room
                    for(int room_id = 0;room_id<MAX_CHATROOM;room_id++){
                        if(players_fd[room_id].size() == 1){
                            write(connfd[i], "Game Start!", 11);
                            players_fd[room_id].push_back(connfd[i]);
                            break;
                        }
                    }
                }

                else if(isdigit(recvline[0])){ // Enter the room, client should check if the number is 0-9 and is listed on the screen
                    int room_id = recvline[0] - '0';
                    if(players_fd[room_id].size() == 1){ // Enter a room with one player
                        write(connfd[i], "Game Start!", 11);
                        players_fd[room_id].push_back(connfd[i]);
                    }
                    else if(players_fd[room_id].size() == 2){ // Enter a room as a viewer
                        if(viewers_fd[room_id].size() >= MAX_VIEWER){
                            write(connfd[i], "Too many viewers!\n", 19);
                            continue;
                        }
                        write(connfd[i], "A gentleman should keep silent while watching.\n", 47);
                        viewers_fd[room_id].push_back(connfd[i]);
                    }
                }

                else{
                    write(connfd[i], "Invalid input!\n", 15);
                    continue;
                }
            }
        }
    }
}
