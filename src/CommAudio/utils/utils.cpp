/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: server.cpp - Hold the code for the server side of the application.
--
-- PROGRAM: CommAudio
--
-- FUNCTIONS:
-- SOCKET NewUDPSocket();
-- SOCKET NewTCPSocket();
-- SOCKADDR_IN SetDestinationAddr(std::string address, int port);
-- void JoinMulticast(SOCKET *socketfd, std::string achMcAddr);
-- void BindSocket(SOCKET *socketfd, char* hostname, int port);
--
-- DATE: 2013/03/23
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Wright
--
-- PROGRAMMER: Jesse Wright
--
-- NOTES:
-- Holds many wrapper functions that will be used by the client and server. 
----------------------------------------------------------------------------------------------------------------------*/
#include "utils.h"
using namespace std;
#pragma comment(lib, "ws2_32.lib")
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: NewUDPSocket
--
-- DATE: 2013/03/23
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Wright
--
-- PROGRAMMER: Jesse Wright
--
-- INTERFACE: SOCKET NewUDPSocket()
--
-- RETURNS: SOCKET- the new socket descriptor
--
-- NOTES: Wrapper for creating a UDP socket. 
----------------------------------------------------------------------------------------------------------------------*/
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
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: NewTCPSocket
--
-- DATE: 2013/03/23
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Wright
--
-- PROGRAMMER: Jesse Wright
--
-- INTERFACE: SOCKET NewTCPSocket()
--
-- RETURNS: SOCKET - the new socket descriptor
--
-- NOTES: Wrapper for creating a new TCP socket.
----------------------------------------------------------------------------------------------------------------------*/
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
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: SetDestinationAddr
--
-- DATE: 2013/03/23
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Wright
--
-- PROGRAMMER: Jesse Wright
--
-- INTERFACE: SOCKADDR_IN SetDestinationAddr(string address, int port)
--
-- RETURNS: SOCKADDR_IN - struct for the destination address.
--
-- NOTES: Wrapper for setting the destination address we will be sending to.
----------------------------------------------------------------------------------------------------------------------*/
SOCKADDR_IN SetDestinationAddr(string address, int port)
{
	SOCKADDR_IN stDstAddr;
	/* Assign our destination address */
	stDstAddr.sin_family 		= AF_INET;
	stDstAddr.sin_addr.s_addr 	= inet_addr(address.c_str());
	stDstAddr.sin_port 			= htons(port);

	return stDstAddr;
}
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: JoinMulticast
--
-- DATE: 2013/03/23
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Wright
--
-- PROGRAMMER: Jesse Wright
--
-- INTERFACE: int JoinMulticast(SOCKET *socketfd, string achMcAddr)
--
-- RETURNS: void.
--
-- NOTES: Wrapper for joining a multicast group.
----------------------------------------------------------------------------------------------------------------------*/
void JoinMulticast(SOCKET *socketfd, string achMcAddr)
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
}
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: BindSocket
--
-- DATE: 2013/03/23
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Wright
--
-- PROGRAMMER: Jesse Wright
--
-- INTERFACE: void BindSocket(SOCKET *socketfd, char* hostname, int port)
--
-- RETURNS: void.
--
-- NOTES: Wrapper for binding a socket to an address.
----------------------------------------------------------------------------------------------------------------------*/
void BindSocket(SOCKET *socketfd, char* hostname, int port)
{
	int nRet;
	SOCKADDR_IN stLclAddr;

	stLclAddr.sin_family      = AF_INET;
	stLclAddr.sin_port        = port;
	hostname == NULL ? stLclAddr.sin_addr.s_addr = htonl(INADDR_ANY) : stLclAddr.sin_addr.s_addr = inet_addr(hostname);
	nRet = bind(*socketfd, (struct sockaddr*) &stLclAddr, sizeof(stLclAddr));

	if (nRet == SOCKET_ERROR) 
	{
		printf ("bind() port: %d failed, Err: %d\n", port, WSAGetLastError());
	}
}