#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include "server.h"

int main(int argc, char *argv[]) 
{
	run_multicast();
	WSACleanup();

	return (0);
}

