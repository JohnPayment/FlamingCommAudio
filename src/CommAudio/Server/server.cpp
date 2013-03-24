#include "server.h"
#pragma comment(lib, "ws2_32.lib")

using namespace std;
string achMCAddr		   = TIMECAST_ADDR;
u_long lMCAddr;
u_short nPort              = TIMECAST_PORT;
u_long  lTTL               = TIMECAST_TTL;
u_short nInterval          = TIMECAST_INTRVL;


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
void run_multicast()
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
	disable_loopback(&socketfd);
	stDstAddr = SetDestinationAddr(achMCAddr, nPort);

	closesocket(socketfd);
}