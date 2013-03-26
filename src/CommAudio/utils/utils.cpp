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
-- NOTES: Wrapper for creating a new overlapped UDP socket. 
----------------------------------------------------------------------------------------------------------------------*/
SOCKET NewUDPSocket()
{
	SOCKET socketfd;
	socketfd = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, 0, 0, 0);
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
-- NOTES: Wrapper for creating a new overlapped TCP socket.
----------------------------------------------------------------------------------------------------------------------*/
SOCKET NewTCPSocket()
{
	SOCKET socketfd;
	socketfd = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);
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
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: ReadFromFile
--
-- DATE: 2013/03/24
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Wright
--
-- PROGRAMMER: Jesse Wright
--
-- INTERFACE: int ReadFromFile(HANDLE hFile, char* buffer)
--
-- RETURNS: int - number of bytes read
--
-- NOTES: Wrapper for reading from a file.
----------------------------------------------------------------------------------------------------------------------*/
int ReadFromFile(HANDLE hFile, char* buffer)
{
	DWORD BytesRead;
	DWORD BytesToRead = BUFLEN - 1;
	if(!ReadFile(hFile, buffer, BytesToRead, &BytesRead, NULL))
	{
		printf("ReadFile() error. Err: %d\n", GetLastError());
	}

	return BytesRead;
}
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: UDPSend
--
-- DATE: 2013/03/24
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Wright
--
-- PROGRAMMER: Jesse Wright
--
-- INTERFACE: int UDPSend(SOCKET s, const char* buf, const struct sockaddr *dest)
--
-- RETURNS: int - number of bytes sent
--
-- NOTES: Wrapper for sending the buffer data to a UDP Socket
----------------------------------------------------------------------------------------------------------------------*/
void UDPSend(SOCKET s, char* buf, const struct sockaddr *dest, OVERLAPPED *sendOv)
{
	WSABUF buffer;
	buffer.buf = buf;
	buffer.len = strlen(buf);
	assert(WSASendTo(s, &buffer, 1, NULL, 0, dest, sizeof(struct sockaddr), sendOv, UDPSendComplete)  == 0 || WSAGetLastError() == WSA_IO_PENDING);
	memset(buf, 0, BUFLEN);
}
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: UDPSendComplete
--
-- DATE: 2013/03/24
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Wright
--
-- PROGRAMMER: Jesse Wright
--
-- INTERFACE: void CALLBACK UDPSendComplete(DWORD dwError, DWORD dwTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
--
-- RETURNS: void.
--
-- NOTES: CompletionRoutine for sending via a UDP socket. Just checks for errors.
----------------------------------------------------------------------------------------------------------------------*/
void CALLBACK UDPSendComplete(DWORD dwError, DWORD dwTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	assert(dwError == 0);
}
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: UDPRecvComplete
--
-- DATE: 2013/03/24
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Wright
--
-- PROGRAMMER: Jesse Wright
--
-- INTERFACE: void CALLBACK UDPRecvComplete(DWORD dwError, DWORD dwTransferred, OVERLAPPED *lpOverlapped, DWORD dwFlags)
--
-- RETURNS: void.
--
-- NOTES: CompletionRoutine for recieving via a UDP socket. Checks bytes trasnferred and posts another WSARecvFrom.
----------------------------------------------------------------------------------------------------------------------*/
void CALLBACK UDPRecvComplete(DWORD dwError, DWORD dwTransferred, OVERLAPPED *lpOverlapped, DWORD dwFlags)
{
	recvData *data = (struct recvData*)lpOverlapped->hEvent;
	static DWORD dwTotalTransferred;
	int addrLen = sizeof(sockaddr);

	if(dwTransferred == 0)
	{
		data->bQuit = true;
		return;
	}

	dwTotalTransferred += dwTransferred;
	WSABUF buf;
	buf.buf = &data->recvBuffer[dwTotalTransferred];
	buf.len = RECV_MAX - dwTotalTransferred;

	assert(WSARecvFrom(*(data->sock), &buf, 1, &dwTransferred, &dwFlags, data->dest, &addrLen, lpOverlapped, UDPRecvComplete) == 0 || WSAGetLastError() == WSA_IO_PENDING);
}
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: UDPRead
--
-- DATE: 2013/03/24
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Wright
--
-- PROGRAMMER: Jesse Wright
--
-- INTERFACE: void UDPRead(OVERLAPPED *recvOv)
--
-- RETURNS: int - number of bytes read
--
-- NOTES: The overlapped structures "hEvent" will hold a pointer to a recvData struct and we will get the socket,
-- recvBuffer, among other things from there. Must be set before this function is called.
----------------------------------------------------------------------------------------------------------------------*/
void UDPRead(OVERLAPPED *recvOv)
{
	recvData *data = (recvData*) recvOv->hEvent;
	int addrLen = sizeof(struct sockaddr);
	DWORD dwTransferred;

	WSABUF buffer;
	buffer.buf = data->recvBuffer;
	buffer.len = RECV_MAX;

	assert(WSARecvFrom(*(data->sock), &buffer, 1, &dwTransferred, 0, data->dest, &addrLen, recvOv, UDPRecvComplete) == 0 || WSAGetLastError() == WSA_IO_PENDING);

}
