/*
Authors: Yuval Kuperberg 200832525
		   Yaniv Boneh 201444403
Project: 
Description: This project is responsible of handling the server side of the communication, including all the game options (player vs. server,
			 player vs. player), calculating the winner of each round, and manage the flow of the game.
*/

#include "Server_Comm.h"

int main(int argc, char *argv[])
{
	int pass_fail_flag = 0, check, i = 0;
	//time_t t;
	//srand((unsigned)time(&t));
	//for (i = 0; i < 1000; i++) {
	//	check = rand() % 5 + 1;
	//	printf("%d\n", check);
	//	if (check < 1 || check > 5)
	//		break;
	//}
	pass_fail_flag = MainServer(argv[1]);
}