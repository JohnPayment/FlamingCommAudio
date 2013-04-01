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
#include "libzplay.h"
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "libzplay.lib")
using namespace libZPlay;
using namespace std;
ZPlay* player;
string firstframe;
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
	bool firsttime = true;
	DWORD BytesRead = 0, BytesWritten, Flags = 0;
	int result, SenderAddrLen = sizeof(SenderAddr);
	buf.len = RECV_MAX;
	buf.buf = recvBuf;
	player = CreateZPlay();
	player->Play();
	
	
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
	
	while(true)
	{
		//Blocking call to the socket. This is needed to keep the client in sync.
		if((BytesRead = recvfrom(recvSocket, recvBuf, RECV_MAX, 0, NULL, NULL)) > 0 )
		{
			firstframe.append(recvSocket, BytesRead);	
		}
		//Wait for a valid MP3 packet
 		while(!player->OpenStream(1, 1, firstframe.data(), firstframe.size(), sfMp3))
		{
			
			result = WSARecvFrom(recvSocket, &buf, 1, &BytesRead, &Flags, &SenderAddr, &SenderAddrLen, &recvOv, UDPRoutine);
			if((result = WSAWaitForMultipleEvents(1, &recvOv.hEvent, TRUE, INFINITE, TRUE)) == WSA_WAIT_FAILED)
			{
			break;
			} else
			{
				firstframe.append(buf.buf, BytesRead);	
			}
			
		}
		player->Play();
		//Keep recieving data.
		while(true)
		{
			TStreamStatus status;
			player->GetStatus(&status);
			if(status.fPlay == 0)
			{
				MessageBox(NULL, "Song Over", "", NULL);
				break; // song over, go wait for a new song back at the top.
			}

			result = WSARecvFrom(recvSocket, &buf, 1, &BytesRead, &Flags, &SenderAddr, &SenderAddrLen, &recvOv, UDPRoutine);
			if((result = WSAWaitForMultipleEvents(1, &recvOv.hEvent, TRUE, INFINITE, TRUE)) == WSA_WAIT_FAILED)
			{
				break;
			} else
			{
				player->PushDataToStream(buf.buf, BytesRead);
			}

		}
	}
			
}