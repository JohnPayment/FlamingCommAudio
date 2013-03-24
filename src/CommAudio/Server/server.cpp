/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: server.cpp - Hold the code for the server side of the application.
--
-- PROGRAM: CommAudio
--
-- FUNCTIONS:
-- void DisableLoopback(SOCKET *socketfd);
-- void RunMulticast();
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
-- The server will handle client requests via a TCP control channel, and from there will handle the clients requests
-- via threading. Clients will be able to Download/Upload audio files, listen to the current radio broadcast, and 
-- have a peer to peer microphone session.
----------------------------------------------------------------------------------------------------------------------*/

#include "server.h"
#pragma comment(lib, "ws2_32.lib")
using namespace std;
string achMCAddr		   = TIMECAST_ADDR;
u_long lMCAddr;
u_short nPort              = TIMECAST_PORT;
u_long  lTTL               = TIMECAST_TTL;
u_short nInterval          = TIMECAST_INTRVL;

/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: DisableLoopback
--
-- DATE: 2013/03/23
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Wright
--
-- PROGRAMMER: Jesse Wright
--
-- INTERFACE: void DisableLoopback(SOCKET *socketfd)
--
-- RETURNS: void
--
-- NOTES: Disables loopback on the socket discriptor passed in.
----------------------------------------------------------------------------------------------------------------------*/
void DisableLoopback(SOCKET *socketfd)
{
	bool fFlag = FALSE;
	int nRet;
	nRet = setsockopt(*socketfd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&fFlag, sizeof(fFlag));
	if (nRet == SOCKET_ERROR) 
	{
		printf ("setsockopt() IP_MULTICAST_LOOP failed, Err: %d\n", WSAGetLastError());
	}
}
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: RunMulticast
--
-- DATE: 2013/03/23
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Wright
--
-- PROGRAMMER: Jesse Wright
--
-- INTERFACE: void Run_Multicast()
--
-- RETURNS: void
--
-- NOTES: Main loop that will read from the audio file and broadcast to the multicast group.
----------------------------------------------------------------------------------------------------------------------*/
void RunMulticast()
{
	int nRet, i;
	BOOL  fFlag;
	SOCKADDR_IN stDstAddr;
    /* Multicast interface structure */
	SOCKET socketfd;
	WSADATA stWSAData;
	/* Init WinSock */
	nRet = WSAStartup(0x0202, &stWSAData);
	if (nRet) 
	{
		printf ("WSAStartup failed: %d\r\n", nRet);
		exit (1);
	}
	socketfd = NewUDPSocket();
	BindSocket(&socketfd, INADDR_ANY, 0);
	JoinMulticast(&socketfd, achMCAddr);

	/* Set IP TTL to traverse up to multiple routers */
	nRet = setsockopt(socketfd, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&lTTL, sizeof(lTTL));
	if (nRet == SOCKET_ERROR) 
	{
		printf ("setsockopt() IP_MULTICAST_TTL failed, Err: %d\n", WSAGetLastError());
	}
	DisableLoopback(&socketfd);
	stDstAddr = SetDestinationAddr(achMCAddr, nPort);

	closesocket(socketfd);
}