#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include "server.h"

extern char achMCAddr[MAXADDRSTR];
extern u_long lMCAddr;
extern u_short nPort;   
extern u_long  lTTL   ;   
extern u_short nInterval ;
extern SYSTEMTIME stSysTime;

int main(int argc, char *argv[]) 
{
	run_multicast();
	WSACleanup();

	return (0);
}