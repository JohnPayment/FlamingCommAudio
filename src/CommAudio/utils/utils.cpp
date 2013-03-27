/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: utils.cpp - Hold the code that is used in both the server and client applications to avoid duplicate
--							code.
--
-- PROGRAM: CommAudio
--
-- FUNCTIONS:
-- SOCKET NewUDPSocket();
-- SOCKET NewTCPSocket();
-- SOCKADDR_IN SetDestinationAddr(std::string address, int port);
-- int JoinMulticast(SOCKET *socketfd, std::string achMcAddr);
-- int BindSocket(SOCKET *socketfd, char* hostname, int port);
-- int SetReuseAddr(SOCKET* socketfd);
-- void CALLBACK UDPRoutine(DWORD dwError, DWORD dwTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);
-- void UDPSend(SOCKET s, char* buf, const struct sockaddr *dest, OVERLAPPED *sendOv);
-- int ReadFromFile(HANDLE hFile, char* buffer);
--
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
	socketfd = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (socketfd == INVALID_SOCKET) 
	{
		printf ("socket() failed, Err: %d\n", WSAGetLastError());
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
	socketfd = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (socketfd == INVALID_SOCKET) 
	{
		printf ("socket() failed, Err: %d\n", WSAGetLastError());
	}
	return socketfd;
}
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: SetDestinationAddr
--
-- DATE: 2013/03/23
--
-- REVISIONS: (Date and Description)
--			2013/03/26 - Jesse Wright:
--			Changed function to use a hostent struct so we can also set addr for TCP connection on the client side
--
-- DESIGNER: Jesse Wright
--
-- PROGRAMMER: Jesse Wright
--
-- INTERFACE: SOCKADDR_IN SetDestinationAddr(string address, int port)
--
-- RETURNS: SOCKADDR_IN - struct for the destination address.
--
-- NOTES: Wrapper for setting the address we'll be sending to (UDP) or connecting to (TCP).
----------------------------------------------------------------------------------------------------------------------*/
SOCKADDR_IN SetDestinationAddr(string address, int port)
{
	SOCKADDR_IN addr;
	struct hostent *hp;

	memset((char *)&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family 		= AF_INET;
	addr.sin_port 			= htons(port);
	if ((hp = gethostbyname(address.c_str())) == NULL)
	{
		printf("Unknown server address\n");
	}
	// Copy the server address
	memcpy((char *)&addr.sin_addr, hp->h_addr, hp->h_length);

	return addr;
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
	return nRet;
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
int BindSocket(SOCKET *socketfd, char* hostname, int port)
{
	int nRet;
	SOCKADDR_IN stLclAddr;

	stLclAddr.sin_family      = AF_INET;
	stLclAddr.sin_port        = htons(port);
	hostname == NULL ? stLclAddr.sin_addr.s_addr = htonl(INADDR_ANY) : stLclAddr.sin_addr.s_addr = inet_addr(hostname);
	nRet = bind(*socketfd, (struct sockaddr*) &stLclAddr, sizeof(stLclAddr));
	if (nRet == SOCKET_ERROR) 
	{
		printf ("bind() port: %d failed, Err: %d\n", port, WSAGetLastError());
	}
	return nRet;
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
	assert(WSASendTo(s, &buffer, 1, NULL, 0, dest, sizeof(struct sockaddr), sendOv, UDPRoutine)  == 0 || WSAGetLastError() == WSA_IO_PENDING);
	memset(buf, 0, BUFLEN);
}
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: UDPRoutine
--
-- DATE: 2013/03/24
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Wright
--
-- PROGRAMMER: Jesse Wright
--
-- INTERFACE: void CALLBACK UDPRoutine(DWORD dwError, DWORD dwTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
--
-- RETURNS: void.
--
-- NOTES: CompletionRoutine for sending/recieving via a UDP socket. Just checks for errors. Since UDP is either all
-- other nothing, we don't need to do any processing in this. We will simply error check.
----------------------------------------------------------------------------------------------------------------------*/
void CALLBACK UDPRoutine(DWORD dwError, DWORD dwTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	assert(dwError == 0);
}
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: SetReuseAddr
--
-- DATE: 2013/03/26
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Wright
--
-- PROGRAMMER: Jesse Wright
--
-- INTERFACE: int SetReuseAddr(SOCKET* socketfd)
--
-- RETURNS: int - result of setsockopt.
--
-- NOTES: Sets the reuse addr option on the socket so we can use the port again once the application has ended.
----------------------------------------------------------------------------------------------------------------------*/
int SetReuseAddr(SOCKET* socketfd)
{
	int result;
	bool flag = true;
	if((result = setsockopt(*socketfd, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag))) == SOCKET_ERROR)
	{
		printf("setsockopt error: %d\n", WSAGetLastError());
	}
	return result;
}
