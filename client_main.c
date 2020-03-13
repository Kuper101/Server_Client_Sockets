/*
Authors: Yuval Kuperberg 200832525
		   Yaniv Boneh 201444403
Project:
Description: This project is responsible of handling the client side of the communication, including printing the menues for the client,
			 printing the results of each game and handling with errors in each step of the process.
*/

#include "client_comm.h"
#include <stdio.h>


int main(int argc, char *argv[])
{
	int error_flag = 0;
	error_flag = MainClient(argv[1], argv[2], argv[3]);
	return error_flag;
}