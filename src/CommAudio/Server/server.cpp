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
#pragma comment(lib, "libzplay.lib")
using namespace std;
using namespace libZPlay;
string achMCAddr		   = TIMECAST_ADDR;
u_long lMCAddr;
u_short nPort              = TIMECAST_PORT;
u_long  lTTL               = TIMECAST_TTL;
bool bQuit;
OVERLAPPED sendOv;
SOCKET sock;
struct	sockaddr_in server;
char buffer[10000];
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

void StartServerMicSession()
{
	HANDLE MicSessionHandle;
	DWORD threadID;

	if((MicSessionHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) MicServerSessionThread, NULL, 0, &threadID)) == 0)
	{
		MessageBox(NULL, "Microphone session thread creation failed", NULL, MB_OK);
		return;
	}
}

DWORD WINAPI MicServerSessionThread()
{
int	i, j, server_len, client_len, result, recv_bytes;
	WSADATA WSAData;
	WORD wVersionRequested = MAKEWORD (2,2);

	// Initialize the DLL with version Winsock 2.2
	WSAStartup(wVersionRequested, &WSAData);

	// Create a datagram socket
	sock = NewUDPSocket();
	if((result = BindSocket(&sock, INADDR_ANY, 5150)) == SOCKET_ERROR)
	{
		perror("Cannot bind socket");
		exit(1);
	}

	ZPlay *mic = CreateZPlay();
	ZPlay *speaker = CreateZPlay();

	speaker->SetSettings(sidSamplerate, 44100);// 44100 samples
    speaker->SetSettings(sidChannelNumber, 2);// 2 channel
    speaker->SetSettings(sidBitPerSample, 16);// 16 bit
    speaker->SetSettings(sidBigEndian, 1); // little endian
	

	if((result = speaker->OpenStream(1, 1, &i, 1, sfPCM)) == 0) // we open the zplay stream without any real data, and start playback when we actually get input.
    {
		printf("Error speaker: %s\n", speaker->GetError());
        speaker->Release();
        return 0;
    }

	/*if((result = mic->OpenFile("wavein://", sfAutodetect)) == 0) // open the mic
	{
		printf("Error microphone: %s\n", mic->GetError());
		mic->Release();
		return 0;
	}*/
	mic->SetCallbackFunc(SendRoutine, (TCallbackMessage) (MsgWaveBuffer|MsgStop), NULL);
	mic->Play();
	

	while(true)
	{
		if(kbhit())
        {
            int a = getch();
            if(a == 'q' || a == 'Q')
                break; // end program if Q key is pressed
        }

		int size = sizeof(server);
		if((recv_bytes = recvfrom(sock, buffer, sizeof(buffer), 0, (sockaddr*)&server, &size)) < 0)
		{
			int err = WSAGetLastError();
			if(err == 10054)
				printf("Connection reset by peer.\n");
			else
				printf("Get Last error %d\n", err);
			break;
		}
		
		printf("%d got %d \n", GetTickCount(), recv_bytes);
        speaker->PushDataToStream(buffer, recv_bytes);
        speaker->Play();
	
		//printf("Buffer: %s\n", buffer);
        // get stream status to check if song is still playing
        TStreamStatus status;
        speaker->GetStatus(&status);
        if(status.fPlay == 0)
			break; // exit checking loop
 
            TStreamTime pos;
            speaker->GetPosition(&pos);
            printf("Pos: %02u:%02u:%02u:%03u\r", pos.hms.hour, pos.hms.minute, pos.hms.second, pos.hms.millisecond);
	}
	speaker->Release();
	mic->Release();
	WSACleanup();
    return 0;
}

int __stdcall SendRoutine(void* instance, void *user_data, libZPlay::TCallbackMessage message, unsigned int param1, unsigned int param2)
{
	if ( message == MsgStop )
		return closesocket(sock);

	if (sendto(sock, (const char *)param1, param2, 0, (const struct sockaddr*)&server, sizeof(server)) < 0)
			return 2;

	return 1;
}
