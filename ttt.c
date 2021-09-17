#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void draw_board(char* board) {
    for(int i = 0; i < 9; i++) {
        if(i % 3 == 0 && i != 0) {
            printf("\n");
        }
        printf("%c|", board[i]);
    }
    printf("\n");
    return;
}

char check_win(char* board, char turn) {
    if((board[0] == board[1] && board[1] == board[2] && board[2] == turn)
    || (board[3] == board[4] && board[4] == board[5] && board[3] == turn)
    || (board[6] == board[7] && board[7] == board[8] && board[6] == turn)) {
        return turn;
    }
    if((board[0] == board[3] && board[3] == board[6] && board[0] == turn)
    || (board[1] == board[4] && board[4] == board[7] && board[1] == turn)
    || (board[2] == board[5] && board[5] == board[8] && board[2] == turn)) {
        return turn;
    }
    if((board[0] == board[4] && board[4] == board[8] && board[0] == turn)
    || (board[2] == board[4] && board[4] == board[6] && board[2] == turn)) {
        return turn;
    }
    return 0;

}

int main(int argc, char** argv) {

    int is_host;
    int clientsocket_fd;

    if(argc == 2 && strcmp("host", argv[1]) == 0) {
        is_host = 1;
        printf("hosting...\n");
        struct sockaddr_in address;
        int serversocket_fd = socket(AF_INET, SOCK_STREAM, 0);
        int option = 1;

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(4444);

        setsockopt(serversocket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(int));
        bind(serversocket_fd, (struct sockaddr*) &address, sizeof(struct sockaddr_in));
        listen(serversocket_fd, 4);
        uint32_t addrlen = sizeof(struct sockaddr_in);
		clientsocket_fd = accept(serversocket_fd, (struct sockaddr*) &address, &addrlen);
    }
    else if(argc == 3) {
        is_host = 0;
        char* ip = argv[1];
        int port;
        sscanf(argv[2], "%d", &port);
        printf("joining %s on port %d\n", ip, port);
        struct sockaddr_in serv_addr;
        clientsocket_fd = socket(AF_INET, SOCK_STREAM, 0);

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        inet_pton(AF_INET, ip, &serv_addr.sin_addr);

        connect(clientsocket_fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    }
    else{
        return -1;
    }

    char symbols[2] = {'X', 'O'};
    int turn = symbols[0];
    int move = 0;
    char input[100];
    int loc = -1;
    char winner = '\0';
    char board[9];
    memset(board, ' ', 9);

    while(!winner && move < 9) {
        printf("Player %c to move...\n", turn);

        if(!((move + is_host) % 2)) {
            recv(clientsocket_fd, &loc, sizeof(int), 0);
            printf("Opponent has selected: %d\n", loc);
            board[loc] = turn;
            winner = check_win(board, turn);
            move++;
            turn = symbols[move % 2];
        }
        else {
            fgets(input, 100, stdin);
            if(sscanf(input, "%d", &loc) != 1) {
                printf("Invalid move\n");
                continue;
            }
            if(loc > 8 || loc < 0) {
                printf("Invalid move\n");
                continue;
            }
            if(board[loc] == ' ') {
                send(clientsocket_fd, &loc, sizeof(int), 0);
                board[loc] = turn;
                winner = check_win(board, turn);
                move++;
                turn = symbols[move % 2];
            }
            else {
                printf("Invalid move\n");
                continue;
            }
        }
        draw_board(board);
    }
    if(winner) {
        printf("%c wins!\n", winner);
    }
    else {
        printf("draw\n");
    }
    return 0;
}