#ifndef SERVER_GAME_H
#define SERVER_GAME_H

#include <winsock2.h>
int server_game_handler(char *username, SOCKET *server_socket, HANDLE *mutexhandle);

typedef struct _ThreadInputs_t {
	HANDLE mutexhandle;
	SOCKET socket;

}ThreadInputs_t;

typedef struct _ClientsDetails_t {
	char player_user_name[100];
	HANDLE player_event;

}ClientsDetails_t;



ClientsDetails_t  First_player;
ClientsDetails_t  Second_player;







#endif