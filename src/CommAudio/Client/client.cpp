/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: client.cpp - Hold the code for the client side of the application.
--
-- PROGRAM: CommAudio
--
-- FUNCTIONS:
-- void WINAPI ClientMulticastThread()
-- 
--
--
--
--
--
-- DATE: 2013/03/26
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Wright
--
-- PROGRAMMER: Jesse Wright
--
-- NOTES:
-- Holds the code that will run the server side of the application.
----------------------------------------------------------------------------------------------------------------------*/
#include "client.h"
#pragma comment(lib, "ws2_32.lib")

/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: ClientMulticastThread
--
-- DATE: 2013/03/26
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Wright
--
-- PROGRAMMER: Jesse Wright
--
-- INTERFACE: void WINAPI ClientMulticastThread()
--
-- RETURNS:  void
--
-- NOTES: Thread for joining multicast group and processing the data that is recieved.
----------------------------------------------------------------------------------------------------------------------*/
void WINAPI ClientMulticastThread()
{
	
	SOCKET recvSocket;
	WSADATA wsaData;
	WSABUF buf;
	WSAOVERLAPPED recvOv;
	char output[BUFLEN];
	char recvBuf[RECV_MAX];
	struct sockaddr SenderAddr;
	DWORD BytesRead, Flags = 0;
	int result, SenderAddrLen = sizeof(SenderAddr);
	buf.len = RECV_MAX;
	buf.buf = recvBuf;
	memset(recvBuf, 0, sizeof(recvBuf));
	ZeroMemory(&recvOv, sizeof(WSAOVERLAPPED));

	//Create WSAEvent for overlapped struct
	if((recvOv.hEvent = WSACreateEvent()) == NULL)
	{
		sprintf(output, "Overlapped hEvent error: %d\n", WSAGetLastError());
		MessageBox(NULL, output, "Error", NULL);
		WSACleanup();
		return;
	}
	//WSA initialize
	if((result = WSAStartup(0x0202, &wsaData)) !=0)
	{
		sprintf(output, "WSAStartup Error: %d\n", WSAGetLastError());
		MessageBox(NULL, output, "Error", NULL);
	}
	//Create New data gram socket
	if((recvSocket = NewUDPSocket()) == INVALID_SOCKET)
	{
		sprintf(output, "WSASocket error: %d\n", WSAGetLastError());
		MessageBox(NULL, output, "Error", NULL);
	}
	//set reuse adder on the socket
	if((result = SetReuseAddr(&recvSocket)) == SOCKET_ERROR)
	{
		sprintf(output, "SetReuseAddr error: %d\n", WSAGetLastError());
		MessageBox(NULL, output, "Error", NULL);
	}
	//Bind the socket
	if((result = BindSocket(&recvSocket, INADDR_ANY, 8910)) == SOCKET_ERROR)
	{
		sprintf(output, "BindSocket error: %d\n", WSAGetLastError());
		MessageBox(NULL, output, "Error", NULL);
	}
	//Join the Multicast Group
	if((result = JoinMulticast(&recvSocket, "234.5.6.7")) == SOCKET_ERROR)
	{
		sprintf(output, "JoinMulticast error: %d\n", WSAGetLastError());
		MessageBox(NULL, output, "Error", NULL);
	}

	// This is temporary
	HANDLE hFile = CreateFile("test.txt", GENERIC_WRITE, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return;

	while(TRUE)
	{
		WSARecvFrom(recvSocket, &buf, 1, &BytesRead, &Flags, &SenderAddr, &SenderAddrLen, &recvOv, UDPRoutine);
		if((result = WSAWaitForMultipleEvents(1, &recvOv.hEvent, TRUE, INFINITE, TRUE)) == WSA_WAIT_FAILED)
		{
			break;
		} else
		{
			// HERE IS WHERE wE PROCESS THE DATA
			WriteFile(hFile, buf.buf, strlen(buf.buf), &BytesRead, NULL);
		}
			
	}
}