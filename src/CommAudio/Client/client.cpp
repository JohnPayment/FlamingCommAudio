/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: client.cpp - Hold the code for the client side of the application.
--
-- PROGRAM: CommAudio
--
-- FUNCTIONS:
-- void WINAPI ClientMulticastThread()
-- DWORD WINAPI MicClientSessionThread(LPVOID lpParameter);
-- void StartClientMicSession(char * IPAddress); //start mic session client thread
-- int __stdcall SendRoutine(void* instance, void *user_data, libZPlay::TCallbackMessage message, unsigned int param1, unsigned int param2);
--
-- DATE: 2013/03/26
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Wright
--			 Luke Tao
--
-- PROGRAMMER: Jesse Wright
--			   Luke Tao
--
-- NOTES:
-- Holds the code that will run the server side of the application. The client has the option to send a request
-- to the server for a peer-to-peer microphone session.
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
	if((result = JoinMulticast(&recvSocket, "234.5.6.8")) == SOCKET_ERROR)
	{
		sprintf(output, "JoinMulticast error: %d\n", WSAGetLastError());
		MessageBox(NULL, output, "Error", NULL);
	}
	
	while(true)
	{
		//Blocking call to the socket. This is needed to keep the client in sync.
		if((BytesRead = recvfrom(recvSocket, recvBuf, RECV_MAX, 0, NULL, NULL)) > 0 )
		{
			firstframe.append(recvBuf, BytesRead);	
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

/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: StartClientMicSession
--
-- DATE: 2013/03/24
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Luke Tao
--
-- PROGRAMMER: Luke Tao
--
-- INTERFACE: void StartClientMicSession(char * IPAddress)
--										
--										 char * IPAddress - the server IP Address to send data to.
--
-- RETURNS: void.
--
-- NOTES: Function that creates the actual Microphone client session thread.
----------------------------------------------------------------------------------------------------------------------*/
void StartClientMicSession(char * IPAddress)
{
	HANDLE MicSessionHandle;
	DWORD threadID;

	if((MicSessionHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) MicClientSessionThread, (LPVOID) IPAddress, 0, &threadID)) == 0)
	{
		MessageBox(NULL, "Microphone session thread creation failed", NULL, MB_OK);
		return;
	}
}

/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: MicClientSessionThread
--
-- DATE: 2013/03/24
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Luke Tao
--
-- PROGRAMMER: Luke Tao
--
-- INTERFACE: DWORD WINAPI MicClientSessionThread(LPVOID lpParameter)
--
--												  LPVOID lpParameter - a server IP address passed in.
--
-- RETURNS: 0 on exit.
--
-- NOTES: The actual thread and processing for the microphone session client side.
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI MicClientSessionThread(LPVOID lpParameter)
{

	char * IPAddress = (char *) lpParameter;
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
		delete[] IPAddress;
	}
	server = SetDestinationAddr(IPAddress, PORT);
	//Bind the socket
	if((result = BindSocket(&sock, INADDR_ANY, PORT)) == SOCKET_ERROR)
	{
		sprintf(output, "BindSocket error: %d\n", WSAGetLastError());
		MessageBox(NULL, output, "Error", NULL);
		delete[] IPAddress;
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
		delete[] IPAddress;
		return 0;
	}

	// Open microphone
	result = mic->OpenFile("wavein://", sfAutodetect);
	if(result == 0) {
		sprintf(output, "Error: %s\n", mic->GetError());
		MessageBox(NULL, output, NULL, MB_OK);
		mic->Release();
		delete[] IPAddress;
		return 0;
	}
	
	// Microphone data is provided via callback message MsgWaveBuffer.
	mic->SetCallbackFunc(SendRoutine, (TCallbackMessage)(MsgWaveBuffer|MsgStop), NULL);

	// start getting microphone input.
	mic->Play();

	while(1) {
		
		int size = sizeof(server);

		// end microphone session if 'q' is pressed
		if(kbhit())
        {
            int a = getch();
            if(a == 'q' || a == 'Q')
                break; 
        }

		//receive microphone data from client socket
		if((recv_bytes = recvfrom(sock, buf, sizeof(buf), 0, (sockaddr*)&server, &size)) < 0)
		{
			int err = WSAGetLastError();
			if(err == 10054)
				MessageBox(NULL, "P2P Microphone session ended.", NULL, MB_OK);
			else
			{
				sprintf(output, "Get Last error %d\n", err);
				MessageBox(NULL, output, NULL, MB_OK);
			}
			
			break;
		}
		//Process and play microphone data
		speakers->PushDataToStream(buf, recv_bytes);
		speakers->Play();
	}
	delete[] IPAddress;
	speakers->Release();
	mic->Release();
	WSACleanup();
	return 0;

}

/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: SendRoutine
--
-- DATE: 2013/03/24
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Luke Tao
--
-- PROGRAMMER: Luke Tao
--
-- INTERFACE: int __stdcall SendRoutine(void* instance, void *user_data, libZPlay::TCallbackMessage message, unsigned int param1, unsigned int param2)
--										
--										void* instance - not used, parameter is for passing in ZPlay object.										
--										void* user_data - not used, parameter is for passing in number of bytes read from a sound file.
--										libZPlay::TCallbackMessage message - message passed in to be handled.
--										unsigned int param1 - audio buffer data to be sent.
--										unsigned int param2 - the size of the audio buffer data.										
--
-- RETURNS: Returns 1 on success, 2 when sending audio data to the server fails.
--
-- NOTES: This is the completion callback routine for sending microphone data to the server. Note that this callback
--        function is used from the "libZPlay" API library.
----------------------------------------------------------------------------------------------------------------------*/
int __stdcall SendRoutine(void* instance, void *user_data, libZPlay::TCallbackMessage message, unsigned int param1, unsigned int param2)
{
	if ( message == MsgStop )
		return closesocket(sock);

	//Push microphone data to the client
	if (sendto(sock, (const char *)param1, param2, 0, (const struct sockaddr*)&server, sizeof(server)) < 0)
			return 2;

	return 1;
}