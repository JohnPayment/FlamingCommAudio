#ifndef UTILS_H
#define UTILS_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string>
#include <cstdlib>
#include <conio.h>
#include <cassert>

#define BUFLEN 1024
#define RECV_MAX 1024
//This struct will be assigned to the OVerlapped struct 
//hEvent parameter for use in the Completion Routine.
typedef struct recvData
{
	bool bQuit;
	char recvBuffer[RECV_MAX];
	SOCKET *sock;
	sockaddr* dest;
};

SOCKET NewUDPSocket();
SOCKET NewTCPSocket();
SOCKADDR_IN SetDestinationAddr(std::string address, int port);
void JoinMulticast(SOCKET *socketfd, std::string achMcAddr);
void BindSocket(SOCKET *socketfd, char* hostname, int port);
int ReadFromFile(HANDLE hFile, char* buffer);
void CALLBACK UDPSendComplete(DWORD dwError, DWORD dwTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);
void UDPSend(SOCKET s, char* buf, const struct sockaddr *dest, OVERLAPPED *sendOv);
void CALLBACK UDPRecvComplete(DWORD dwError, DWORD dwTransferred, OVERLAPPED *lpOverlapped, DWORD dwFlags);
void UDPRead(OVERLAPPED *recvOv);

#endif