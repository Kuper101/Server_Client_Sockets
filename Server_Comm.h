#ifndef SERVER_COMM_H
#define SERVER_COMM_H
#include <stdio.h>
#include <stdbool.h>
#include <winsock2.h>


#define NUM_OF_WORKER_THREADS 2
#define MAX_LOOPS 3
#define SEND_STR_SIZE 35

int MainServer(char *port_num);



#endif // 
