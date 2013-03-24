#include "utils.h"
using namespace std;

#pragma comment(lib, "ws2_32.lib")

SOCKET NewUDPSocket()
{
	SOCKET socketfd;
	socketfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socketfd == INVALID_SOCKET) 
	{
		printf ("socket() failed, Err: %d\n", WSAGetLastError());
		exit(1);
	}
	return socketfd;
}
SOCKET NewTCPSocket()
{
	SOCKET socketfd;
	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketfd == INVALID_SOCKET) 
	{
		printf ("socket() failed, Err: %d\n", WSAGetLastError());
		exit(1);
	}
	return socketfd;
}
SOCKADDR_IN SetDestinationAddr(string address, int port)
{
	SOCKADDR_IN stDstAddr;
	/* Assign our destination address */
	stDstAddr.sin_family 		= AF_INET;
	stDstAddr.sin_addr.s_addr 	= inet_addr(address.c_str());
	stDstAddr.sin_port 			= htons(port);

	return stDstAddr;
}
int JoinMulticast(SOCKET *socketfd, string achMcAddr)
{
	int nRet;
	struct ip_mreq stMreq;  
	stMreq.imr_multiaddr.s_addr = inet_addr(achMcAddr.c_str());
	stMreq.imr_interface.s_addr = INADDR_ANY;
	nRet = setsockopt(*socketfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&stMreq, sizeof(stMreq));
	if (nRet == SOCKET_ERROR) 
	{
		printf ("setsockopt() IP_ADD_MEMBERSHIP address %s failed, Err: %d\n", achMcAddr.c_str(), WSAGetLastError());
	} 
	return 0;
}
void BindSocket(SOCKET *socketfd, char* hostname, int port)
{
	int nRet;
	SOCKADDR_IN stLclAddr;

	stLclAddr.sin_family      = AF_INET;
	stLclAddr.sin_port        = port;                 /* any port */
	hostname == NULL ? stLclAddr.sin_addr.s_addr = htonl(INADDR_ANY) : stLclAddr.sin_addr.s_addr = htonl((u_long)hostname);
	nRet = bind(*socketfd, (struct sockaddr*) &stLclAddr, sizeof(stLclAddr));

	if (nRet == SOCKET_ERROR) 
	{
		printf ("bind() port: %d failed, Err: %d\n", port, WSAGetLastError());
	}
}
