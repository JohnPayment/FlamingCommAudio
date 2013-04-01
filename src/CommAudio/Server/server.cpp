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
bool bQuit;
OVERLAPPED sendOv;
void OpenWinFile(HANDLE* hFile, string name);
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
	SOCKET socketfd;
	WSADATA stWSAData;
	ifstream songFile;
	char buffer[BUFLEN];
	ZeroMemory(&sendOv, sizeof(OVERLAPPED));
	int currentSong = 1;
	string songName;
	HANDLE hFile;
	bool backToStart = false;

	memset(buffer, 0, BUFLEN);
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
	SetTimeToLive(socketfd, lTTL);
	DisableLoopback(&socketfd);
	stDstAddr = SetDestinationAddr(achMCAddr, nPort);

	_getch();
	while(true) // main loop for choosing song from library
	{
		songFile.open("songs.txt");
		
		for(int i = 0; i < currentSong; i++) // will loop until the current number
		{
			getline(songFile, songName);
			cout << songName << endl;
		}
		if(songFile.eof())
		{
			backToStart = true;
		}
		songFile.close();
		OpenWinFile(&hFile, songName);
		while((i= ReadFromFile(hFile, buffer))) // sending a song
		{
			UDPSend(socketfd, buffer, (struct sockaddr*) &stDstAddr, &sendOv, i);
			Sleep(20);
		}
		CloseHandle(hFile);
		if(backToStart)
		{
			currentSong = 1;
			backToStart = false;
		}else
		{
			currentSong++;
		}
	}
	

	closesocket(socketfd);
}
void OpenWinFile(HANDLE* hFile, string name)
{
	*hFile = CreateFile(name.c_str(), GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		printf("Error opening Song. Ernum: %d,", GetLastError);
	}
}
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: SetTimeToLive
--
-- DATE: 2013/03/24
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Wright
--
-- PROGRAMMER: Jesse Wright
--
-- INTERFACE: void SetTimeToLive(SOCKET s, u_long TTL)
--
-- RETURNS: void.
--
-- NOTES: Wrapper for setting the time to live on a socket.
----------------------------------------------------------------------------------------------------------------------*/
void SetTimeToLive(SOCKET s, u_long TTL)
{
	int nRet = setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&TTL, sizeof(TTL));
	if (nRet == SOCKET_ERROR) 
	{
		printf ("setsockopt() IP_MULTICAST_TTL failed, Err: %d\n", WSAGetLastError());
	}
}
