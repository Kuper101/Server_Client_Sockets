/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/*
 This file was written for instruction purposes for the
 course "Introduction to Systems Programming" at Tel-Aviv
 University, School of Electrical Engineering, Winter 2011,
 by Amnon Drory.
*/
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#ifndef SOCKET_EXAMPLE_SHARED_H
#define SOCKET_EXAMPLE_SHARED_H

#define SERVER_ADDRESS_STR "127.0.0.1"
#define SERVER_PORT 2345
#define TIME_OUT_IN_MSEC 15000
#define MAX_USERNAME_LENGTH 20
#define MAX_MESSAGE_LEN 26//The longest message length is 26 charecters
#define MAX_NUM_OF_PARAMETERS 4
#define MAX_LEN_OF_PARAMETERS 59//2 usernames = 40, 2 game moves = 8, 3 times ';'  = 3
#define MAX_PARAMETERS_LENGTH 20
#define MAX_MOVE_LEN 8
#define MESSAGE_LEN_AS_INT 2
#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )
#define ROCK "ROCK"
#define PAPER "PAPER"
#define SCISSORS "SCISSORS"
#define SPOCK "SPOCK"
#define LIZARD "LIZARD"


#endif // SOCKET_EXAMPLE_SHARED_H