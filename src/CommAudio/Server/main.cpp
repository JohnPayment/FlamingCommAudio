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
ifstream readFile;
char fileName[DATA_BUFSIZE];

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
	TCPMode = 0;

	TCPServer::get()->WorkerRoutine = TCPRoutine;
	TCPServer::get()->StartServer();
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

	if(SI->BytesRECV > SI->BytesSEND)
	{
		switch(TCPMode)
		{
		case 0:
			
			return;
		case 1:
			// Send File Names
			break;
		case 2:
			// File Open. This has its own number because we only want to do it once
			// test.txt will need to be replaced with a file name received from the client later on
			readFile.open(fileName);
			if(readFile.fail())
			{
				break;
			} else
			{
				TCPMode = 3;
			}
		case 3:
			// File Transfer
			readFile.read(SI->Buffer, DATA_BUFSIZE);
			if(readFile.eof())
			{
				readFile.close();
				TCPMode = 0;
			}
			break;
		}
		TCPServer::get()->writeToSocket(SI);
	} else
	{
		TCPServer::get()->readFromSocket(SI);
		if(TCPMode == 1)
		{
			strncpy(fileName, SI->Buffer, DATA_BUFSIZE);
			TCPMode = 1;
		}

		if(!strcmp(SI->Buffer, FILE_TRANSFER))
		{
			// We want to send A list of names
			TCPMode = 1;
		} else if(!strcmp(SI->Buffer, MICROPHONE))
		{
			// Put Code for starting Microphone mode here
		}
	}
}
