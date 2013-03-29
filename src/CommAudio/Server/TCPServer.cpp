#include "TCPServer.h"

TCPServer* TCPServer::_Server = 0;

TCPServer* 
TCPServer::get()
{
	if(_Server == 0)
	{
		_Server = new TCPServer();
	}

	return _Server;
}

/*--------------------------------------------------------------------------------------------------------------------
-- FUNCTION: TCPServer
--
-- DATE: 2013/03/25
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: John Payment
--
-- PROGRAMMER: John Payment
--
-- INTERFACE: TCPServer()
--
-- RETURNS: N/A
--
-- NOTES: Constructor for TCPServer. Private for Singleton class
----------------------------------------------------------------------------------------------------------------------*/
TCPServer::TCPServer()
{

}

/*--------------------------------------------------------------------------------------------------------------------
-- FUNCTION: StartServer
--
-- DATE: 2013/03/25
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: John Payment
--
-- PROGRAMMER: John Payment
--
-- INTERFACE: void StartServer()
--
-- RETURNS: void
--
-- NOTES: Sets up everything the Server needs to be a server.
----------------------------------------------------------------------------------------------------------------------*/
void 
TCPServer::StartServer()
{
	WSADATA wsaData;
	INT Ret;

	// Start up Windows Garbage
	if((Ret = WSAStartup(0x0202, &wsaData)) != 0)
	{
		printf("WSAStartup failed with error %d\n", Ret);
		WSACleanup();
		return;
	}

	// Initialize Listening Socket
	if((_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) 
	{
		printf("Failed to get a socket %d\n", WSAGetLastError());
		return;
	}

	// Setup Socket Options
	_InternetAddress.sin_family = AF_INET;
	_InternetAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	_InternetAddress.sin_port = htons(PORT);

	// Bind Socket
	if(bind(_ListenSocket, (PSOCKADDR)&_InternetAddress, sizeof(_InternetAddress)) == SOCKET_ERROR)
	{
		printf("bind() failed with error %d\n", WSAGetLastError());
		return;
	}
}

/*--------------------------------------------------------------------------------------------------------------------
-- FUNCTION: ListenForClients
--
-- DATE: 2013/03/25
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: John Payment
--
-- PROGRAMMER: John Payment
--
-- INTERFACE: void ListenForClients()
--
-- RETURNS: void
--
-- NOTES: Listens for Clients who may want to connect and gives each its own worker thread.
----------------------------------------------------------------------------------------------------------------------*/
void 
TCPServer::ListenForClients()
{
	DWORD threadId;
	WSAEVENT acceptEvent;
	HANDLE threadHandle;

	// Listen for a maximum of 5 simultaneous clients
	if(listen(_ListenSocket, 5))
	{
		printf("listen() failed with error %d\n", WSAGetLastError());
		return;
	}

	// Setup Accept event
	if((acceptEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
	{
		printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
		return;
	}

	// Create the thread that will actually allow clients to connect as well as spawn worker threads to do the I/O work
	if((threadHandle = CreateThread(NULL, 0, WorkerThread, (LPVOID)acceptEvent, 0, &threadId)) == NULL)
	{
		printf("CreateThread failed with error %d\n", GetLastError());
		return;
	}

	// Loop FOREVER!
	while(true)
	{
		// Actually post accept events so the thread can handle them
		_AcceptSocket = accept(_ListenSocket, NULL, NULL);

		if(WSASetEvent(acceptEvent) == false)
		{
			printf("WSASetEvent failed with error %d\n", WSAGetLastError());
			return;
		}
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
-- INTERFACE: void readFromSocket(LPSOCKET_INFORMATION &SI)
--
-- RETURNS: void
--
-- NOTES: Reads data from the Socket and stores it in SI->DataBuf
----------------------------------------------------------------------------------------------------------------------*/
void 
TCPServer::readFromSocket(LPSOCKET_INFORMATION &SI)
{
	DWORD recieveBytes;
	DWORD flags = 0;

	SI->BytesRECV = 0;

	ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));

	SI->DataBuf.len = DATA_BUFSIZE;
	SI->DataBuf.buf = SI->Buffer;

	if(WSARecv(SI->Socket, &(SI->DataBuf), 1, &recieveBytes, &flags, &(SI->Overlapped), WorkerRoutine) == SOCKET_ERROR)
	{
		if(WSAGetLastError() != WSA_IO_PENDING )
		{
			printf("WSARecv() failed with error %d\n", WSAGetLastError());
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
-- INTERFACE: void writeToSocket(LPSOCKET_INFORMATION &SI)
--
-- RETURNS: void
--
-- NOTES: Writes data to the Socket from SI->DataBuf
----------------------------------------------------------------------------------------------------------------------*/
void 
TCPServer::writeToSocket(LPSOCKET_INFORMATION &SI)
{
	DWORD sendBytes;

	ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));

	SI->DataBuf.buf = SI->Buffer + SI->BytesSEND;
	SI->DataBuf.len = SI->BytesRECV - SI->BytesSEND;

	if(WSASend(SI->Socket, &(SI->DataBuf), 1, &sendBytes, 0, &(SI->Overlapped), WorkerRoutine) == SOCKET_ERROR)
	{
		if(WSAGetLastError() != WSA_IO_PENDING)
		{
			printf("WSASend() failed with error %d\n", WSAGetLastError());
		}
	}
}

SOCKET 
TCPServer::getSocket()
{
	return _AcceptSocket;
}

DWORD WINAPI WorkerThread(LPVOID lpParameter)
{
	DWORD flags = 0;
	DWORD index;
	DWORD bytesRecieved;

	LPSOCKET_INFORMATION SocketInfo;
	WSAEVENT eventArray[1];

	// Save the accept event in the event array.

	eventArray[0] = (WSAEVENT)lpParameter;

	while(true)
	{
		// Wait for accept() to signal an event and also process WorkerRoutine() returns.
		while(true)
		{
			index = WSAWaitForMultipleEvents(1, eventArray, false, WSA_INFINITE, true);

			if(index == WSA_WAIT_FAILED)
			{
				printf("WSAWaitForMultipleEvents failed with error %d\n", WSAGetLastError());
				return false;
			}

			if(index != WAIT_IO_COMPLETION)
			{
				// An accept() call event is ready - break the wait loop
				break;
			} 
		}

		WSAResetEvent(eventArray[index - WSA_WAIT_EVENT_0]);
   
		// Create a socket information structure to associate with the accepted socket.

		if((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
		{
			printf("GlobalAlloc() failed with error %d\n", GetLastError());
			return false;
		} 

		// Fill in the details of our accepted socket.

		SocketInfo->Socket = TCPServer::get()->getSocket();
		ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));  
		SocketInfo->BytesSEND = 0;
		SocketInfo->BytesRECV = 0;
		SocketInfo->DataBuf.len = DATA_BUFSIZE;
		SocketInfo->DataBuf.buf = SocketInfo->Buffer;

		if(WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &bytesRecieved, &flags, &(SocketInfo->Overlapped), TCPServer::get()->WorkerRoutine) == SOCKET_ERROR)
		{
			if(WSAGetLastError() != WSA_IO_PENDING)
			{
				printf("WSARecv() failed with error %d\n", WSAGetLastError());
				return false;
			}
		}

		printf("Socket %d connected\n", TCPServer::get()->getSocket());
	}

	return true;
}
