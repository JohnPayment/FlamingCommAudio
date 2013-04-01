#ifndef TCP_CLIENT
#define TCP_CLIENT

/*--------------------------------------------------------------------------------------------------------------------
-- CLASS: TCPClient
--
-- FUNCTIONS: TCPClient(sockaddr_in server)
--            void StartClient()
--            void readFromSocket(char* buffer)
--            int writeToSocket(char* buffer)
--            void closeSocket()
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

#include <stdio.h>
#include <winsock2.h>
#include <errno.h>

#define SERVER_TCP_PORT 5150
#define BUFFER_SIZE     8192

#define FILE_TRANSFER "F"
#define START_TRANSFER "S"
#define START_UPLOAD "U"
#define MICROPHONE "M"

class TCPClient
{
public:
	TCPClient(sockaddr_in server);

	void StartClient();

	void readFromSocket(char* buffer);
	int writeToSocket(char* buffer);

	void closeSocket();
private:

	sockaddr_in _server;
	SOCKET _socket;
};

#endif