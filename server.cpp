/*
    只有一個人在server裡時 他打C進thread後其他人就連不進server了
    thread的select會卡住新進的人
    進遊戲後還是會被main的select抓到input!!
    MAX_VIEWER是10還是8?
*/
#include    "slither.h"
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
#include    <stdio.h>

#define SERV_PORT 7122
#define MAXLINE 4096
#define LISTENQ 1024
#define SA struct sockaddr

using namespace std;

int MAX_CLIENT = 20;
int MAX_CHATROOM = 10;
int MAX_VIEWER = 8;
int cur_client = 0;
int cur_room = 0;
vector<pthread_t> my_thread(MAX_CHATROOM);
vector<vector<int>> players_fd(MAX_CHATROOM);
vector<vector<int>> viewers_fd(MAX_CHATROOM);
// client info
vector<int>                 connfd(MAX_CLIENT, -1);
vector<struct sockaddr_in>	cliaddr(MAX_CLIENT);
vector<socklen_t>			clilen(MAX_CLIENT, sizeof(cliaddr[0]));

void *game_room(void* room_id_void){
    struct timeval myTimeval;
    myTimeval.tv_sec = 5;  
    myTimeval.tv_usec = 0; 
    int                room_id = *(int*)room_id_void, n;
    unordered_map<int, string>     player_id;
    char                sendline[MAXLINE], recvline[MAXLINE];
    int                 maxfdp1;
	fd_set		        rset;

    // Call entire game
    State game;

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
        select(maxfdp1, &rset, NULL, NULL, &myTimeval);

        for(int i = 0;i<players_fd[room_id].size();i++){
            auto p = players_fd[room_id][i];

            if(FD_ISSET(p, &rset)){
                if(player_id.find(p) == player_id.end()){ // if the player is new
                    if(n = read(p, recvline, MAXLINE) <= 0) { // disconnect before entering a name
                        if (i == 0) {
                            printf("From %d: Disconnect!\n", p);
                            players_fd[room_id].clear();
                            cur_room--;
                            for (auto& x: players_fd[room_id]) {
                                std::vector<int>::iterator it = std::find(connfd.begin(), connfd.end(), -1);
                                int available = it - connfd.begin();
                                connfd[available] = x;
                            }
                            pthread_exit(NULL);
                        }
                        else {
                            printf("From %d: Disconnect!\n", p);
                            players_fd[room_id].erase(std::find(players_fd[room_id].begin(), players_fd[room_id].end(), p));
                            std::vector<int>::iterator it = std::find(connfd.begin(), connfd.end(), -1);
                            int available = it - connfd.begin();
                            connfd[available] = p;
                            continue;
                        }
                    }
                    recvline[n-1] = 0;

                    bool isDuplicate = false;
                    for (auto& id: player_id) {
                        if (id.second == string(recvline)){
                            write(p, "Duplicate\n", 10);
                            printf("To %d: Duplicate\n", p);
                            isDuplicate = true;
                            break;
                        }
                    }
                    if (isDuplicate) continue;

                    player_id[p] = string(recvline);
                    if(i == 0){ // if the player is the first player
                        write(p, "Waiting for the second player...\n", 33);
                        printf("To %d: Waiting for the second player...\n", p);
                    }
                    else{   // second player
                        write(p, "Game Start!\n", 12);
                        printf("To %d: Game Start!\n", p);
                        write(players_fd[room_id][0], "Your turn!\n", 11);
                    }
                }
                else{
                    if(n = read(p, recvline, MAXLINE) > 0) {
                        recvline[n-1] = 0;
                        int a1, a2, a3;
                        sscanf(recvline, "%d %d %d", &a1, &a2, &a3);
                        if(game.legal_actions.find(a1) != game.legal_actions.end()) {
                            game.apply_action(a1);
                            if(game.legal_actions.find(a2) != game.legal_actions.end()){
                                game.apply_action(a2);
                                if(game.legal_actions.find(a3) != game.legal_actions.end()){
                                    game.apply_action(a3);
                                    // send board info to others
                                }
                            }
                        }
                        // send "resend" to the player

                    }
                    else{
                        printf("From %d: Disconnect!\n", p);
                        // TODO : send message to the other player and determine who wins
                        players_fd[room_id].clear();
                        cur_room--;
                        for (auto& x: players_fd[room_id]) {
                            std::vector<int>::iterator it = std::find(connfd.begin(), connfd.end(), -1);
                            int available = it - connfd.begin();
                            connfd[available] = x;
                        }
                        pthread_exit(NULL);
                    }
                }
            }
        }

        

        for(int i = 0;i<viewers_fd[room_id].size();i++){
            auto p = viewers_fd[room_id][i];

            if(FD_ISSET(p, &rset)){
                if(player_id.find(p) == player_id.end()){ // if the player is new
                    if(n = read(p, recvline, MAXLINE) <= 0) { // disconnect before entering a name
                        printf("From %d: Disconnect!\n", p);
                        viewers_fd[room_id].erase(std::find(viewers_fd[room_id].begin(), viewers_fd[room_id].end(), p));
                        std::vector<int>::iterator it = std::find(connfd.begin(), connfd.end(), -1);
                        int available = it - connfd.begin();
                        connfd[available] = p;
                        continue;
                    }
                    recvline[n-1] = 0;
                    bool isDuplicate = false;
                    for (auto& id: player_id) {
                        if (id.second == string(recvline)){
                            write(p, "Duplicate!\n", 11);
                            printf("To %d: Duplicate!\n", p);
                            isDuplicate = true;
                            break;
                        }
                    }
                    if (isDuplicate) continue;

                    player_id[p] = string(recvline);
                    write(p, "A gentleman should keep silent while watching.\n", 47);
                    printf("To %d: A gentleman should keep silent while watching.\n", p);
                }
                else{
                    if(n = read(p, recvline, MAXLINE) > 0) {
                        recvline[n-1] = 0;
                    }
                    else{
                        printf("From %d: Disconnect!\n", p);
                        viewers_fd[room_id].erase(std::find(viewers_fd[room_id].begin(), viewers_fd[room_id].end(), p));
                        player_id.erase(p);
                        std::vector<int>::iterator it = std::find(connfd.begin(), connfd.end(), -1);
                        int available = it - connfd.begin();
                        connfd[available] = p;
                    }
                }
            }
        }
    }

}

int main(int argc, char **argv){
	int					listenfd;
    pid_t               childpid;
	struct sockaddr_in	servaddr;

    printf("\033]0;Slither Server\007");
    printf("\033[=3h");
    printf("\033[2J");

    printf("Server running...\n");


	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT + 3);

	bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	listen(listenfd, LISTENQ);

    // Signal(SIGCHLD, sig_chld);
    int                         maxfdp1, n;
	fd_set		                rset;

    for ( ; ; ) { // Deal with socket
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
            
            if(cur_client == MAX_CLIENT){ // Too many clients
                printf("Too many clients!\n");
            }
            else{ //setting
                cur_client++;
                std::vector<int>::iterator it = std::find(connfd.begin(), connfd.end(), -1);
                int available = it - connfd.begin();
                connfd[available] = accept(listenfd, (SA *) &cliaddr[available], &clilen[available]);
                write(connfd[available], "Connect successfully!\n", 22);
                printf("To %d: Connect successfully!\n", connfd[available]);
            }
        }

        // Deal with client with every cases
        for(int i = 0;i<MAX_CLIENT;i++){

            if(connfd[i] != -1 && FD_ISSET(connfd[i], &rset)){
                
                if((n = read(connfd[i], recvline, MAXLINE)) <= 0){ // Client close
                    printf("From %d: Disconnect!\n", connfd[i]);
                    connfd[i] = -1;
                    bzero(&cliaddr[i], sizeof(cliaddr[i]));
                    bzero(&clilen[i], sizeof(clilen[i]));
                    continue;
                }
                recvline[n] = 0;
                printf("Clientfd %d: %s", connfd[i], recvline);
                // Deal with client's input
                if(strcmp(recvline, "C\n") == 0){ // Create a room
                    if(cur_room >= MAX_CHATROOM){
                        write(connfd[i], "Too many game rooms!\n", 21);
                        printf("To %d: Too many game rooms!\n", connfd[i]);
                        continue;
                    }
                    else{
                        for(int room_id = 0;room_id < MAX_CHATROOM;room_id++){
                            if(players_fd[room_id].size() == 0) {
                                players_fd[room_id].push_back(connfd[i]);
                                pthread_create(&my_thread[room_id], NULL, game_room, (void*)&room_id);
                                cur_room++;

                                write(connfd[i], "OK\n", 3);
                                printf("To %d: OK\n", connfd[i]);
                                connfd[i] = -1;
                                break;
                            }
                        }
                    }
                }

                else if(strcmp(recvline, "E\n") == 0){ // Send room list
                    bzero(sendline, MAXLINE);
                    string room_list = "";
                    for(int room_id = 0;room_id<MAX_CHATROOM;room_id++){
                        if(players_fd[room_id].size() == 0) continue;
                        room_list += "   " + to_string(room_id) + "       " + to_string(players_fd[room_id].size()) + "/2      " + to_string(viewers_fd[room_id].size()) + "/8\n";
                    }
                    if (room_list.size() == 0) room_list = "Empty\n";
                    copy(room_list.begin(), room_list.end(), sendline);
                    write(connfd[i], sendline, strlen(sendline));
                    printf("To %d: %s", connfd[i], sendline);
                }

                else if(strcmp(recvline, "R\n") == 0){ // Enter a random room
                    bool enter = false;
                    for(int room_id = 0;room_id<MAX_CHATROOM;room_id++){
                        if(players_fd[room_id].size() == 1){
                            write(connfd[i], "Player\n", 7);
                            printf("To %d: Player\n", connfd[i]);
                            players_fd[room_id].push_back(connfd[i]);
                            enter = true;
                            connfd[i] = -1;
                            break;
                        }
                    }
                    if(!enter) {
                        write(connfd[i], "All rooms are full!\n", 20);
                        printf("To %d: All rooms are full!\n", connfd[i]);
                    }
                }

                else if(isdigit(recvline[0])){ // Enter the room, client should check if the number is 0-9 and is listed on the screen
                    int room_id = recvline[0] - '0';
                    if(players_fd[room_id].size() == 1){ // Enter a room with one player
                        write(connfd[i], "Player\n", 7);
                        printf("To %d: Player\n", connfd[i]);
                        players_fd[room_id].push_back(connfd[i]);
                        connfd[i] = -1;
                    }
                    else if(players_fd[room_id].size() == 2){ // Enter a room as a viewer
                        if(viewers_fd[room_id].size() >= MAX_VIEWER){
                            write(connfd[i], "Too many viewers!\n", 18);
                            printf("To %d: Too many viewers!\n", connfd[i]);
                            continue;
                        }
                        write(connfd[i], "Viewer\n", 7);
                        printf("To %d: Viewer\n", connfd[i]);
                        viewers_fd[room_id].push_back(connfd[i]);
                        connfd[i] = -1;
                    }
                    else{
                        write(connfd[i], "Invalid room ID!\n", 17);
                        printf("To %d: Invalid room ID!\n", connfd[i]);
                        continue;
                    }
                }
            }
        }
    }
}
