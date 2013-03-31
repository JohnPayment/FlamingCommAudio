#include "TCPClient.h"

/*--------------------------------------------------------------------------------------------------------------------
-- FUNCTION: TCPClient
--
-- DATE: 2013/03/25
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: John Payment
--
-- PROGRAMMER: John Payment
--
-- INTERFACE: TCPClient(sockaddr_in server)
--
-- RETURNS: N/A
--
-- NOTES: Constructor for TCPClient. Takes a sockaddr_in.
----------------------------------------------------------------------------------------------------------------------*/
TCPClient::TCPClient(sockaddr_in server)
:_server(server)
{

}

/*--------------------------------------------------------------------------------------------------------------------
-- FUNCTION: StartClient
--
-- DATE: 2013/03/25
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: John Payment
--
-- PROGRAMMER: John Payment
--
-- INTERFACE: void StartClient()
--
-- RETURNS: void
--
-- NOTES: Sets up everything the client needs including setting up the socket and connecting to the server.
----------------------------------------------------------------------------------------------------------------------*/
void 
TCPClient::StartClient()
{
	int err;
	WSADATA WSAData;
	WORD wVersionRequested;

	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &WSAData);

	//No usable DLL
	if(err != 0)
	{
		fprintf(stderr, "DLL not found!\n");
		return;
	}

	// Create the socket
	if((_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Cannot create socket");
		return;
	}

	// Connecting to the server
	if(connect(_socket, (struct sockaddr *)&_server, sizeof(_server)) == -1)
	{
		fprintf(stderr, "Can't connect to server\n");
		perror("connect");
		return;
	}
}

/*--------------------------------------------------------------------------------------------------------------------
-- FUNCTION: readFromSocket
--
-- DATE: 2013/03/25
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: John Payment
--
-- PROGRAMMER: John Payment
--
-- INTERFACE: void readFromSocket(char* buffer)
--                 char* buffer - A pointer to a character buffer. It must be at least as large as BUFFER_SIZE
--
-- RETURNS: void
--
-- NOTES: Reads from the socket to buffer.
----------------------------------------------------------------------------------------------------------------------*/
void 
TCPClient::readFromSocket(char* buffer)
{
	int n = 0;
	int bufferCapacity = BUFFER_SIZE;

	// client makes repeated calls to recv until no more data is expected to arrive.
	while((n = recv(_socket, buffer, bufferCapacity, 0)) < BUFFER_SIZE)
	{
		DWORD temp = GetLastError();
		buffer += n;
		bufferCapacity -= n;
		if(n == 0)
		{
			break;
		}
	}
}

/*--------------------------------------------------------------------------------------------------------------------
-- FUNCTION: writeToSocket
--
-- DATE: 2013/03/25
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: John Payment
--
-- PROGRAMMER: John Payment
--
-- INTERFACE: void writeToSocket(char* buffer)
--                 char* buffer - A pointer to a character buffer. It must be at least as large as BUFFER_SIZE
--
-- RETURNS: returns the error message from the4 send call.
--
-- NOTES: Writes to the socket from buffer.
----------------------------------------------------------------------------------------------------------------------*/
int 
TCPClient::writeToSocket(char* buffer)
{
	return send(_socket, buffer, BUFFER_SIZE, 0);
}

/*--------------------------------------------------------------------------------------------------------------------
-- FUNCTION: closeSocket
--
-- DATE: 2013/03/25
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: John Payment
--
-- PROGRAMMER: John Payment
--
-- INTERFACE: void closeSocket()
--
-- RETURNS: void
--
-- NOTES: Closes the socket and cleans up the WSA.
----------------------------------------------------------------------------------------------------------------------*/
void
TCPClient::closeSocket()
{
	closesocket(_socket);
	WSACleanup();
}
