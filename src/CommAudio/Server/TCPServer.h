#ifndef TCP_SERVER
#define TCP_SERVER

/*--------------------------------------------------------------------------------------------------------------------
-- CLASS: TCPServer
--
-- FUNCTIONS: TCPServer* get()
--            TCPServer()
--            void StartServer()
--            void ListenForClients()
--            void readFromSocket(LPSOCKET_INFORMATION &SI)
--            void writeToSocket(LPSOCKET_INFORMATION &SI)
--            SOCKET getSocket()
--
-- DATE: 2013/03/25
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: John Payment
--
-- PROGRAMMER: John Payment
--
-- NOTES: This is a TCP client for sending and recieving data.
-- 
----------------------------------------------------------------------------------------------------------------------*/

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>

#define PORT 5150
#define DATA_BUFSIZE 8192

typedef struct _SOCKET_INFORMATION 
{
	OVERLAPPED Overlapped;
	SOCKET Socket;
	CHAR Buffer[DATA_BUFSIZE];
	WSABUF DataBuf;
	DWORD BytesSEND;
	DWORD BytesRECV;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;


class TCPServer
{
private:
	TCPServer();

public:
	static TCPServer* get();

	void StartServer();
	void ListenForClients();

	void readFromSocket(LPSOCKET_INFORMATION &SI);
	void writeToSocket(LPSOCKET_INFORMATION &SI);

	SOCKET getSocket();

	LPWSAOVERLAPPED_COMPLETION_ROUTINE WorkerRoutine;
private:
	static TCPServer* _Server;

	SOCKET _AcceptSocket;
	SOCKET _ListenSocket;
	SOCKADDR_IN _InternetAddress;
};

DWORD WINAPI WorkerThread(LPVOID lpParameter);

#endif