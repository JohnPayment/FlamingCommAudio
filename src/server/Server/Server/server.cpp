/*
* TimeCastSrvr.c              (c) Bob Quinn              3/15/97
*
* Released to the public domain
*
* Description:
*  Sample multicast server (sender) application that multicasts the
*  system time for clients (receivers) to use to set their clocks.
*/
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#include "server.h"


#pragma comment(lib, "ws2_32.lib")

char achMCAddr[MAXADDRSTR] = TIMECAST_ADDR;
u_long lMCAddr;
u_short nPort              = TIMECAST_PORT;
u_long  lTTL               = TIMECAST_TTL;
u_short nInterval          = TIMECAST_INTRVL;
SYSTEMTIME stSysTime;

void show_usage(void) {
	printf("usage: TimeCastSrvr [-g multicast address][:port number]\n");
	printf("                    [-i interval]\n");
	printf("                    [-t IP time-to-live\n\n");
	printf("  multicast address: 224.0.0.0 to 239.255.255.255\n");
	printf("  port number:       server's port\n\n");
	printf("If none provided, default values are %s:%d.\n", TIMECAST_ADDR, TIMECAST_PORT);
	printf("  interval:        seconds between sends (default: 30)\n");
	printf("  IP time-to-live: number of router hops (default: 2)\n");
} 


/* Bind the socket
* 
* NOTE: Normally, we wouldn't need to call bind unless we were 
*  assigning a local port number explicitly (naming the socket), 
*  however Microsoft requires that a socket be bound before it 
*  can join a multicast group with setsockopt() IP_ADD_MEMBERSHIP 
*  (or fails w/ WSAEINVAL).
*/
int bind_socket(SOCKET *socketfd)
{
	int nRet;
	SOCKADDR_IN stLclAddr;
	*socketfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (*socketfd == INVALID_SOCKET) 
	{
		printf ("socket() failed, Err: %d\n", WSAGetLastError());
		exit(1);
	}

	stLclAddr.sin_family      = AF_INET; 
	stLclAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* any interface */
	stLclAddr.sin_port        = 0;                 /* any port */
	nRet = bind(*socketfd, (struct sockaddr*) &stLclAddr, sizeof(stLclAddr));

	if (nRet == SOCKET_ERROR) 
	{
		printf ("bind() port: %d failed, Err: %d\n", nPort, WSAGetLastError());
	}
}
int join_multicast(SOCKET *socketfd, char *achMcAddr)
{
	int nRet;
	struct ip_mreq stMreq;  
	stMreq.imr_multiaddr.s_addr = inet_addr(achMCAddr);
	stMreq.imr_interface.s_addr = INADDR_ANY;
	nRet = setsockopt(*socketfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&stMreq, sizeof(stMreq));
	if (nRet == SOCKET_ERROR) 
	{
		printf ("setsockopt() IP_ADD_MEMBERSHIP address %s failed, Err: %d\n", achMCAddr, WSAGetLastError());
	} 
	return 0;
}
int disable_loopback(SOCKET *socketfd)
{
	bool fFlag = FALSE;
	int nRet;
	nRet = setsockopt(*socketfd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&fFlag, sizeof(fFlag));
	if (nRet == SOCKET_ERROR) 
	{
		printf ("setsockopt() IP_MULTICAST_LOOP failed, Err: %d\n", WSAGetLastError());
	}
	return 0;
}
SOCKADDR_IN set_destination_addr()
{
	SOCKADDR_IN stDstAddr;
	/* Assign our destination address */
	stDstAddr.sin_family =      AF_INET;
	stDstAddr.sin_addr.s_addr = inet_addr(achMCAddr);
	stDstAddr.sin_port =        htons(nPort);

	return stDstAddr;
}
void run_multicast()
{
	int nRet, i;
	BOOL  fFlag;
	SOCKADDR_IN stDstAddr;
    /* Multicast interface structure */
	SOCKET socketfd;
	WSADATA stWSAData;

	printf("------------------------------------------------------\n");
	printf(" TimeCastSrvr - multicast time server   (c) Bob Quinn\n");
	printf("------------------------------------------------------\n");

	/* Init WinSock */
	nRet = WSAStartup(0x0202, &stWSAData);
	if (nRet) 
	{
		printf ("WSAStartup failed: %d\r\n", nRet);
		exit (1);
	}
	bind_socket(&socketfd);
	join_multicast(&socketfd, achMCAddr);

	/* Set IP TTL to traverse up to multiple routers */
	nRet = setsockopt(socketfd, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&lTTL, sizeof(lTTL));
	if (nRet == SOCKET_ERROR) 
	{
		printf ("setsockopt() IP_MULTICAST_TTL failed, Err: %d\n", WSAGetLastError());
	}
	disable_loopback(&socketfd);
	stDstAddr = set_destination_addr();

	for (;;) 
	{
		/* Get System (UTC) Time */
		GetSystemTime (&stSysTime);

		/* Send the time to our multicast group! */
		nRet = sendto(socketfd, (char *)&stSysTime, sizeof(stSysTime), 0, (struct sockaddr*)&stDstAddr, sizeof(stDstAddr));
		if (nRet < 0) 
		{
			printf ("sendto() failed, Error: %d\n", WSAGetLastError());
			exit(1);
		} else 
		{
			printf("Sent UTC Time %02d:%02d:%02d:%03d ",
				stSysTime.wHour, 
				stSysTime.wMinute, 
				stSysTime.wSecond,
				stSysTime.wMilliseconds);

			printf("Date: %02d-%02d-%02d to: %s:%d\n",
				stSysTime.wMonth, 
				stSysTime.wDay, 
				stSysTime.wYear, 
				inet_ntoa(stDstAddr.sin_addr), 
				ntohs(stDstAddr.sin_port));
		}
		Sleep(nInterval*1000);
	}
	closesocket(socketfd);
}