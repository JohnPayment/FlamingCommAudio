#ifndef UTILS_H
#define UTILS_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string>
#include <cstdlib>

#define BUFLEN 256

SOCKET NewUDPSocket();
SOCKET NewTCPSocket();
SOCKADDR_IN SetDestinationAddr(std::string address, int port);
void JoinMulticast(SOCKET *socketfd, std::string achMcAddr);
void BindSocket(SOCKET *socketfd, char* hostname, int port);
int ReadFromFile(HANDLE hFile, char* buffer);

#endif