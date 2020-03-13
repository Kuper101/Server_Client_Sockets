 #define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <winsock2.h>
#include <time.h>
#include "Socket_Send_Recv_Tools.h"
#include "Server_Comm.h"
#include "Socket_Shared.h"
#include "Server_Game.h"

HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
ThreadInputs_t ThreadInputs[NUM_OF_WORKER_THREADS];

static int FindFirstUnusedThreadSlot();
static void CleanupWorkerThreads();
DWORD WINAPI  ServiceThread(SOCKET *t_socket);


int MainServer(char *port_num_str)
{
	int Ind, bindRes, ListenRes, port_num, error_flag = 0;
	char *end_ptr;
	SOCKET MainSocket = INVALID_SOCKET;
	unsigned long Address;
	SOCKADDR_IN service;
	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	LPCTSTR MutexName = ("MutexName");
	HANDLE MutexHandle;

	MutexHandle = CreateMutex(			//Create mutex to secure writing into game file
		NULL,							// default security attributes
		FALSE,							 // initially not owned
		MutexName);
	if (StartupRes != NO_ERROR){
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());                                
		return -1;
	}  
	MainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (MainSocket == INVALID_SOCKET){
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		error_flag = -1;
		goto server_cleanup_1;
	}
	Address = inet_addr(SERVER_ADDRESS_STR);
	if (Address == INADDR_NONE){
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			SERVER_ADDRESS_STR);
		error_flag = -1;
		goto server_cleanup_2;
	}
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = Address;
	port_num = (int)strtol(port_num_str, &end_ptr, 10);
	service.sin_port = htons(port_num); 
	bindRes = bind(MainSocket, (SOCKADDR*)&service, sizeof(service));
	if (bindRes == SOCKET_ERROR){
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		error_flag = -1;
		goto server_cleanup_2;
	}
	ListenRes = listen(MainSocket, SOMAXCONN);
	if (ListenRes == SOCKET_ERROR){
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		error_flag = -1;
		goto server_cleanup_2;
	}
	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
		ThreadHandles[Ind] = NULL;
	printf("Waiting for a client to connect...\n");
	while (TRUE){
		SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET){
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			error_flag = -1;
			goto server_cleanup_3;
		}
		printf("Server: Client Connected.\n");
		Ind = FindFirstUnusedThreadSlot();
		if (Ind == NUM_OF_WORKER_THREADS){
			printf("No slots available for client, dropping the connection.\n");
			closesocket(AcceptSocket); //Closing the socket, dropping the connection.
		}
		else{
			ThreadInputs[Ind].socket = AcceptSocket; // shallow copy: don't close 
			
											  // AcceptSocket, instead close 
											  // ThreadInputs[Ind] when the
											  // time comes.
			ThreadInputs[Ind].mutexhandle = MutexHandle; // shallow copy: don't close 
			ThreadHandles[Ind] = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)ServiceThread,
				&(ThreadInputs[Ind]),
				0,
				NULL
			);
		}
	}

server_cleanup_3:

	CleanupWorkerThreads();

server_cleanup_2:
	if (closesocket(MainSocket) == SOCKET_ERROR) {
		printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
		error_flag = -1;
	}
server_cleanup_1:
	if (WSACleanup() == SOCKET_ERROR) {
			printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
			error_flag = -1;
	}
	return error_flag;
}

static int FindFirstUnusedThreadSlot(){
	int Ind;

	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
	{
		if (ThreadHandles[Ind] == NULL)
			break;
		else
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(ThreadHandles[Ind], 0);

			if (Res == WAIT_OBJECT_0) // this thread finished running
			{
				CloseHandle(ThreadHandles[Ind]);
				ThreadHandles[Ind] = NULL;
				break;
			}
		}
	}
	return Ind;
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

static void CleanupWorkerThreads()
{
	int Ind;

	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
	{
		if (ThreadHandles[Ind] != NULL)
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(ThreadHandles[Ind], INFINITE);

			if (Res == WAIT_OBJECT_0)
			{
				closesocket(ThreadInputs[Ind].socket);
				CloseHandle(ThreadHandles[Ind]);
				ThreadHandles[Ind] = NULL;
				break;
			}
			else
			{
			printf("Waiting for thread failed. Ending program\n");
			return;
			}
		}
	}
}


DWORD WINAPI ServiceThread(LPVOID lpParam)
{
	DWORD error_flag = 0;
	SOCKET* t_socket;
	char username[MAX_USERNAME_LENGTH];
	char *token = NULL, message_type[MAX_MESSAGE_LEN+1];
	TCHAR *rcv_buffer = NULL;
	ThreadInputs_t* pThreadArgs;
	pThreadArgs = (ThreadInputs_t*)lpParam;
	TransferResult_t RecvRes;
	HANDLE mutexhandle;
	mutexhandle = pThreadArgs->mutexhandle;
	t_socket = &(pThreadArgs->socket);
	RecvRes = ReceiveString(&rcv_buffer, *t_socket, TIME_OUT_IN_MSEC);
	if (RecvRes == TRNS_FAILED){	
		printf("Service socket error occured while reading, closing thread.\n");
		closesocket(*t_socket);
		error_flag = -1;
		goto exit;
	}
	else if (RecvRes == TRNS_DISCONNECTED){
		printf("Connection error occured while reading, closing thread.\n");
		error_flag = -1;
		goto exit;
	}
	token = strtok(rcv_buffer, ":"); //extract message type
	strcpy(message_type, token);
	token = strtok(NULL, ":"); //extract username
	strcpy(username, token);
	//while (TRUE){
		if (RecvRes == TRNS_FAILED){
			printf("Service socket error while reading, closing thread.\n");
			error_flag = -1;
			goto exit;
		}
		else if (RecvRes == TRNS_DISCONNECTED){
			printf("Connection closed while reading, closing thread.\n");
			closesocket(*t_socket);
			error_flag = -1;
			goto exit;
		}
		if (STRINGS_ARE_EQUAL(message_type, "CLIENT_REQUEST")) {
			error_flag = send_message("SERVER_APPROVED", NULL, t_socket);
			if (error_flag == -1) {
				printf("Service socket error while writing, closing thread.\n");
				goto exit;
			}
			error_flag = server_game_handler(username, t_socket, &mutexhandle);
			if (error_flag == -1) {
				printf("Service socket error while writing, closing thread.\n");
				goto exit;
			}
		}
	//}
exit:
	closesocket(*t_socket);
	return 0;
}
