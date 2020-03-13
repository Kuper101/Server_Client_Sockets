/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/*
 This file was written for instruction purposes for the
 course "Introduction to Systems Programming" at Tel-Aviv
 University, School of Electrical Engineering.
Last updated by Amnon Drory, Winter 2011.
 */
 /*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <stdbool.h>
#include "client_comm.h"
#include "Socket_Shared.h"
#include "Socket_Send_Recv_Tools.h"



SOCKET m_socket;
bool game_over_menu_is_on = FALSE;
bool client_wants_to_quit = FALSE;
//HANDLE hThread[2];


void split_parameters_into_strings(char* string_to_split, char** parameters){
	int i = 0, param_index = 0;

	while (*string_to_split != '\0') {
		parameters[param_index++] = string_to_split;
		while ((*string_to_split != '\0') && (strchr(";",*string_to_split) == NULL)) {
			++string_to_split;
			if (*string_to_split == '\0') {
				return;
			}
		}
		*string_to_split = '\0';
		++string_to_split;
	}
}

static DWORD RecvDataThread(void)
{
	TransferResult_t RecvRes;
	while (!client_wants_to_quit){
		TCHAR *rcv_buffer = NULL;
		char *token = NULL, message_type[MAX_MESSAGE_LEN+1], parameters[MAX_LEN_OF_PARAMETERS+1], *parameter_list[MAX_NUM_OF_PARAMETERS];
		RecvRes = ReceiveString(&rcv_buffer, m_socket, TIME_OUT_IN_MSEC);
		if (RecvRes == TRNS_FAILED){
			printf("Socket error while trying to write data to socket\n");
			//return 0x555;
		}
		else if (RecvRes == TRNS_DISCONNECTED){
			printf("Server closed connection. Bye!\n");
			//return 0x555;
		}
		token = strtok(rcv_buffer, ":");
		strcpy(message_type, rcv_buffer);
		token = strtok(NULL, ":");
		if (token != NULL) {
			strcpy(parameters, token);
		}
		if (STRINGS_ARE_EQUAL(message_type, "SERVER_MAIN_MENU")) {
			printf("Choose what to do next:\n"
				"1. Play against another client\n"
				"2. Play against the server\n"
				"3. View the leaderboard\n"
				"4. Quit\n");
		}
		else if (STRINGS_ARE_EQUAL(message_type, "SERVER_PLAYER_MOVE_REQUEST")){
			printf("Choose a move from the list: Rock, Paper, Scissors, Lizard or Spock:\n");
		}
		else if (STRINGS_ARE_EQUAL(message_type, "SERVER_GAME_RESULTS")) {   //SERVER_GAME_RESULTS:Server;server_move;your_move;who_won
			split_parameters_into_strings(parameters, parameter_list);
			if (!STRINGS_ARE_EQUAL(parameter_list[3], "Tie")) {
				printf("You played: %s\n"
					"%s played: %s\n"
					"%s won!\n", parameter_list[2], parameter_list[0], parameter_list[1], parameter_list[3]);
			}
			else {
				printf("You played: %s\n"
					"%s played: %s\n", parameter_list[2], parameter_list[0], parameter_list[1]);
			}
		}
		else if (STRINGS_ARE_EQUAL(message_type, "SERVER_GAME_OVER_MENU")){
			game_over_menu_is_on = TRUE;
			printf("Choose what to do next:\n"
				"1. Play again\n"
				"2. Return to the main menu\n");
		}
		free(rcv_buffer);
	}
	return 0;
}

static DWORD SendDataThread(LPVOID lpParam)
{
	DWORD ret_val = 0;
	int i = 0, error_flag = 0;
	char input_buffer[MAX_MESSAGE_LEN], *send_buffer = NULL;
	while (TRUE)
	{
		gets_s(input_buffer, sizeof(input_buffer));
		if (strlen(input_buffer) > 1) {
			_strupr(input_buffer);
		}
		if (STRINGS_ARE_EQUAL(input_buffer, "1")) {
			if (!game_over_menu_is_on) {
				error_flag = send_message("CLIENT_VERSUS", NULL, &m_socket);
			}
			else {
				error_flag = send_message("CLIENT_REPLAY", NULL, &m_socket);
			}
		}
		else if (STRINGS_ARE_EQUAL(input_buffer, "2")){
			if (game_over_menu_is_on) {
				error_flag = send_message("CLIENT_MAIN_MENU", NULL, &m_socket);
				game_over_menu_is_on = FALSE;
			}
			else {
				error_flag = send_message("CLIENT_CPU", NULL, &m_socket);
			}
		}
		else if (STRINGS_ARE_EQUAL(input_buffer, "3"))
			error_flag = send_message("CLIENT_LEADERBOARD", NULL, &m_socket);
		else if (STRINGS_ARE_EQUAL(input_buffer, "4")) {
			client_wants_to_quit = TRUE;
			//CloseHandle(hThread[1]);
			error_flag = send_message("CLIENT_DISCONNECT", NULL, &m_socket);
			break;
		}
		else if (STRINGS_ARE_EQUAL(input_buffer, "ROCK") || STRINGS_ARE_EQUAL(input_buffer, "PAPER") ||
			STRINGS_ARE_EQUAL(input_buffer, "SCISSORS") || STRINGS_ARE_EQUAL(input_buffer, "LIZARD")
			|| STRINGS_ARE_EQUAL(input_buffer, "SPOCK")) {
			error_flag = send_message("CLIENT_PLAYER_MOVE", input_buffer, &m_socket);
		}
		if (error_flag == -1){
			printf("Socket error while trying to write data to socket\n");
			ret_val = 0x555;
			break;
		}
	}
	free(send_buffer);
	return ret_val;
}


int MainClient(char *server_ip, char *port_num_str, char *username)
{
	int client_input_for_error = 0, port_num = 0, error_flag = 0;
	char *end_ptr;
	char send_buffer[MAX_MESSAGE_LEN];
	TransferResult_t SendRes;
	TransferResult_t RecvRes;
	SOCKADDR_IN clientService;
	HANDLE hThread[2];

	// Initialize Winsock.
	WSADATA wsaData; //Create a WSADATA object called wsaData.
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
		printf("Error at WSAStartup()\n");
reconnecting:
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(server_ip); //Setting the IP address to connect to
	port_num = (int)strtol(port_num_str, &end_ptr, 10);
	clientService.sin_port = htons(port_num); //Setting the port to connect to.
	while (TRUE){
		if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
			printf("Failed connecting to server on %s:%s\n"
				"Choose what to do next:\n"
				"1. Try to reconnect\n"
				"2. Exit\n",
				server_ip, port_num_str);
			scanf_s("%d", &client_input_for_error);
			if (client_input_for_error == 1) {
				goto reconnecting;
			}
			else {
				goto client_cleanup_1;
			}
		}
		else {
			printf("Connected to server on %s:%s\n", server_ip, port_num_str);
			strcpy(send_buffer, "CLIENT_REQUEST:");
			strcat(send_buffer, username);
			SendRes = SendString(send_buffer, m_socket);
			if (SendRes == TRNS_FAILED){
				printf("Service socket error while writing, closing thread.\n");
				goto client_cleanup_2;
			}
			char* AcceptedStr = NULL;
			char* message_type = NULL;
			RecvRes = ReceiveString(&AcceptedStr, m_socket, TIME_OUT_IN_MSEC);
			if (RecvRes == TRNS_FAILED){
				printf("Socket error while trying to write data to socket\n");
				//return 0x555;
			}
			if (STRINGS_ARE_EQUAL(AcceptedStr, "SERVER_APPROVED")){
				break;
			}
			else if (STRINGS_ARE_EQUAL(AcceptedStr, "SERVER_DENIED")){
				printf("Server on %s:%s denied the connection request.\n", server_ip, port_num_str);
				goto client_cleanup_2;
			}
		}
	}
	hThread[0] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)SendDataThread,
		username,
		0,
		NULL
	);
	hThread[1] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)RecvDataThread,
		username,
		0,
		NULL
	);

	WaitForMultipleObjects(2, hThread, FALSE, INFINITE);

	TerminateThread(hThread[0], 0x555);
	TerminateThread(hThread[1], 0x555);

	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);
client_cleanup_2:
	if (closesocket(m_socket) == SOCKET_ERROR) {
		printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
		error_flag = -1;
	}
client_cleanup_1:
	if (WSACleanup() == SOCKET_ERROR) {
		printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
		error_flag = -1;
	}
	return error_flag;
}

