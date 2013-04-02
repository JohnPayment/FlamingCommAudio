/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: main.cpp - The entry point for the server side of the comm audio application.
--
-- PROGRAM: CommAudio
--
-- FUNCTIONS:
-- int main(int argc, char *argv[])
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
-- The main function simply creates threads that will be delegated jobs to handle the clients requests as they come in.
----------------------------------------------------------------------------------------------------------------------*/
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "server.h"
#include "TCPServer.h"

using namespace std;

void CALLBACK TCPRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);

int TCPMode;
ifstream readFile, songLibrary;
char fileName[DATA_BUFSIZE];
bool didWrite;

/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: main
--
-- DATE: 2013/03/23
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Wright
--
-- PROGRAMMER: Jesse Wright
--             John Payment
--
-- INTERFACE: int main (int argc, char *argv[])
--
-- RETURNS: int - 0 upon success
--
-- NOTES: Main entry point to the server.
----------------------------------------------------------------------------------------------------------------------*/
int main(int argc, char *argv[]) 
{
	MicServerSessionThread();
	
	TCPMode = 0;
	didWrite = false;

	TCPServer::get()->WorkerRoutine = TCPRoutine;
	TCPServer::get()->StartServer();
	TCPServer::get()->ListenForClients();

	RunMulticast();
	WSACleanup();

	return (0);
}

/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: TCPRoutine
--
-- DATE: 2013/03/28
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: John Payment
--
-- PROGRAMMER: John Payment
--
-- INTERFACE: void CALLBACK TCPRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
--                 DWORD Error
--                 DWORD BytesTransferred
--                 LPWSAOVERLAPPED Overlapped
--                 DWORD InFlags
--
-- RETURNS: void
--
-- NOTES: The worker function used by the Asyncronous TCP Server for the purpose of managing writing to and reading from the socket
----------------------------------------------------------------------------------------------------------------------*/
void CALLBACK TCPRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
	// Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
	LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION) Overlapped;
	char fileSizee[30];
	memset(fileSizee, 0, 30);
	int fileSize = 0, begin = 0;

	if(Error != 0)
	{
		printf("I/O operation failed with error %d\n", Error);
	}

	if(BytesTransferred == 0)
	{
		printf("Closing socket %d\n", SI->Socket);
	}

	if(Error != 0 || BytesTransferred == 0)
	{
		closesocket(SI->Socket);
		GlobalFree(SI);
		return;
	}

	// Check to see if the BytesRECV field equals zero. If this is so, then
	// this means a WSARecv call just completed so update the BytesRECV field
	// with the BytesTransferred value from the completed WSARecv() call.

	if(SI->BytesRECV == 0)
	{
		SI->BytesRECV = BytesTransferred;
		SI->BytesSEND = 0;
	} else
	{
		SI->BytesSEND += BytesTransferred;
	}

	switch(TCPMode)
	{
	case 0: // Default
		if(SI->Buffer[0] == FILE_TRANSFER)
		{
			// We want to send A list of names
			songLibrary.open("songs.txt");
			songLibrary.read(SI->Buffer, DATA_BUFSIZE);
			songLibrary.close();

			TCPMode = 2;
			didWrite = true;

			TCPServer::get()->writeToSocket(SI);
		}
		break;
	case 2: // get file name and Upload/Download
		strncpy(fileName, SI->Buffer, DATA_BUFSIZE);

		if(SI->Buffer[0] == START_TRANSFER)
		{
			readFile.open("test.mp3", std::ifstream::binary);
			begin = readFile.tellg();
			readFile.seekg(0, ios_base::end);
			fileSize = readFile.tellg();
			fileSize -= begin;
			readFile.seekg(0);
			sprintf(fileSizee, "%d", fileSize);
			strcpy(SI->Buffer, fileSizee);

			TCPMode = 4;
			didWrite = true;

			TCPServer::get()->writeToSocket(SI);
		} else if(SI->Buffer[0] == START_UPLOAD)
		{
			// Client Uploading File
			TCPMode = 5;
		}
		
		break;
	case 4:
		{
			readFile.read(SI->Buffer, DATA_BUFSIZE);
		
			if(!readFile.good())
			{
				TCPMode = 0;
			}
			didWrite = true;

			TCPServer::get()->writeToSocket(SI);
		}
		break;
	}

	if(!didWrite)
	{
		// Need to read to continue the routine
		TCPServer::get()->readFromSocket(SI);
	}

	didWrite = false;

}
