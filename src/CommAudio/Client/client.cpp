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
SOCKET sock;
SOCKADDR_IN server, client;
char buf[10000];
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
void StartMicSession()
{
	HANDLE MicSessionHandle;
	DWORD threadID;

	if((MicSessionHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) MicSessionThread, NULL, 0, &threadID)) == 0)
	{
		MessageBox(NULL, "Microphone session thread creation failed", NULL, MB_OK);
		return;
	}
}

DWORD WINAPI MicSessionThread()
{
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2,2);
	WSAStartup(wVersionRequested, &wsaData);
	int recv_bytes, result, i;
	char output[100];
	ZPlay * speakers = CreateZPlay();
	ZPlay *mic = CreateZPlay();

	//set destination address
	if((sock = NewUDPSocket()) == INVALID_SOCKET)
	{
		printf("SetDestAddr error: %d\n", WSAGetLastError());
		MessageBox(NULL, output, "Error", NULL);
	}
	server = SetDestinationAddr("localhost", 7000);
	//Bind the socket
	if((result = BindSocket(&sock, INADDR_ANY, 7000)) == SOCKET_ERROR)
	{
		sprintf(output, "BindSocket error: %d\n", WSAGetLastError());
		MessageBox(NULL, output, "Error", NULL);
	}

	speakers->SetSettings(sidSamplerate, 44100);// 44100 samples
	speakers->SetSettings(sidChannelNumber, 2);// 2 channel
	speakers->SetSettings(sidBitPerSample, 16);// 16 bit
	speakers->SetSettings(sidBigEndian, 1); // little endian

	// Remote microphone is a UDP "stream" of PCM data,
	result = speakers->OpenStream(1, 1, &i, 1, sfPCM); // we open the zplay stream without any real data, and start playback when we actually get input.
	if(result == 0) {
		sprintf(output, "Error: %s\n", speakers->GetError());
		MessageBox(NULL, output, NULL, MB_OK);
		speakers->Release();
		return 0;
	}

	// Open microphone
	result = mic->OpenFile("wavein://", sfAutodetect);
	if(result == 0) {
		sprintf(output, "Error: %s\n", mic->GetError());
		MessageBox(NULL, output, NULL, MB_OK);
		mic->Release();
		return 0;
	}
	
	// Microphone data is provided via callback message MsgWaveBuffer.
	mic->SetCallbackFunc(SendRoutine, (TCallbackMessage)(MsgWaveBuffer|MsgStop), NULL);

	// start getting microphone input.
	mic->Play();

	while(1) {
		
		int size = sizeof(server);
		
		if(kbhit())
        {
            int a = getch();
            if(a == 'q' || a == 'Q')
                break; // end program if Q key is pressed
        }


		// Simply send the entire microphone data buffer to server.
		if((recv_bytes = recvfrom(sock, buf, sizeof(buf), 0, (sockaddr*)&server, &size)) < 0)
		{
			int err = WSAGetLastError();
			if(err == 10054)
				MessageBox(NULL, "Connection reset by peer.", NULL, MB_OK);
			else
			{
				sprintf(output, "Get Last error %d\n", err);
				MessageBox(NULL, output, NULL, MB_OK);
			}
			break;
		}
		speakers->PushDataToStream(buf, recv_bytes);
		speakers->Play();
	}
	speakers->Release();
	mic->Release();
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