/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: main.cpp - The entry point for the client side of the comm audio application.
--
-- PROGRAM: CommAudio
--
-- FUNCTIONS: int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
--            LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM)
--            void writeFileFromNetwork(char* fileName, TCPClient* client)
--
-- DATE: 2013/03/23
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: John Payment
--
-- PROGRAMMER: John Payment
--
-- NOTES:
-- The main function simply creates window that will be taking use imput to determine what jobs should be run.
----------------------------------------------------------------------------------------------------------------------*/
#include "client.h"
#include "TCPClient.h"
#include "resource.h"
#include <iostream>
#include <fstream>

using namespace std;

#define IPSIZE 16

void writeFileFromNetwork(char* fileName, TCPClient* client);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

char response[BUFFER_SIZE];
char outputLine[BUFFER_SIZE];
TCPClient* tcp;
int fileSize;

/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: WinMain
--
-- DATE: 2013/03/23
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: John Payment
--
-- PROGRAMMER: John Payment
--
-- INTERFACE: int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
--
-- RETURNS: int - 0 upon success
--
-- NOTES: Main entry point to the client.
----------------------------------------------------------------------------------------------------------------------*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	static TCHAR szAppName[] = TEXT("HelloWin");
	HWND         hwnd;
	MSG          msg;
	WNDCLASS     wndclass;

	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = WndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground =(HBRUSH) GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);
	wndclass.lpszClassName = szAppName;

	if(!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("This program requires Windows NT!"), 
					szAppName, MB_ICONERROR);
		return 0;
	}
     
	hwnd = CreateWindow(szAppName,                  // window class name
	                    TEXT("The Hello Program"), // window caption
	                    WS_OVERLAPPEDWINDOW,        // window style
	                    CW_USEDEFAULT,              // initial x position
	                    CW_USEDEFAULT,              // initial y position
	                    CW_USEDEFAULT,              // initial x size
	                    CW_USEDEFAULT,              // initial y size
	                    NULL,                       // parent window handle
	                    NULL,                       // window menu handle
	                    hInstance,                  // program instance handle
	                    NULL);                     // creation parameters
    
	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);
     
	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: WndProc
--
-- DATE: 2013/03/23
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: John Payment
--
-- PROGRAMMER: John Payment
--
-- INTERFACE: LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
--
-- RETURNS: The result of the call. usually 0.
--
-- NOTES: manages the window's messages.
----------------------------------------------------------------------------------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC         hdc;
	static HWND IPBox;
	static HWND FileNameBox;
	int BytesRead = 0;
	PAINTSTRUCT ps;
	RECT        rect;
    SOCKET UDPSocket;
	SOCKADDR_IN socketAddr;
	WSAOVERLAPPED Overlapped;


	switch(message)
	{
	case WM_CREATE:
		CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE) ClientMulticastThread, NULL, 0, NULL);

		IPBox =       CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), 
		                             WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT, 100, 30, 300, 20, 
		                             hwnd, (HMENU)5, ((LPCREATESTRUCT) lParam)->hInstance, NULL);
		FileNameBox = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), 
			                         WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT, 100, 60, 300, 20, 
									 hwnd, (HMENU)5, ((LPCREATESTRUCT) lParam)->hInstance, NULL);

		return 0;
          
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);

		GetClientRect(hwnd, &rect);

		RECT lables;
		lables.left = 10;
		lables.right = 90;
		lables.top = 30;
		lables.bottom = 50;
		DrawText(hdc, TEXT("IP"), -1, &lables, DT_SINGLELINE | DT_RIGHT | DT_VCENTER);

		lables.top = 60;
		lables.bottom = 80;
		DrawText(hdc, TEXT("File Name"), -1, &lables, DT_SINGLELINE | DT_RIGHT | DT_VCENTER);

		lables.top = 100;
		lables.bottom = 300;
		lables.right = 300;
		DrawText(hdc, outputLine, -1, &lables,  DT_LEFT | DT_VCENTER);
		  
		EndPaint(hwnd, &ps);
		return 0;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case ID_CONNECTMENU_RADIO: // Start Radio Mode. Not currently used
			//Close P2P
			//CLose FileTransfer
			break;
		case ID_CONNECTMENU_P2P: // Start Microphone Mode
			{
				char * ip = new char[IPSIZE];
				GetWindowText(IPBox, ip, IPSIZE);
				
				//start p2p udp session
				//runs the session unitl
				StartMicSession(ip);

			}
			break;
		case ID_CONNECTMENU_FILETRANSFER: // Start TCP Mode
			{
				if(tcp != NULL)
				{
					delete tcp;
				}
				char ip[IPSIZE];
				//Close P2P
				//Close Radio
				GetWindowText(IPBox, ip, IPSIZE);
				tcp = new TCPClient(SetDestinationAddr(ip, SERVER_TCP_PORT));

				tcp->StartClient();
				tcp->writeToSocket(FILE_TRANSFER);

				BytesRead = tcp->readFromSocket(outputLine);
				if(BytesRead > 0)
				{
					InvalidateRect(hwnd, NULL, true);
					UpdateWindow(hwnd);
				}
			}
			break;
		case ID_POST_FILENAME: // DOWNLOAD (TCP Mode)
			if(tcp != NULL)
			{
				char temp[BUFFER_SIZE] =  START_TRANSFER;
				char fileName[BUFFER_SIZE];
				
				GetWindowText(FileNameBox, fileName, BUFFER_SIZE);
				strcat(temp, fileName);
				// Sending fileName to server
				tcp->writeToSocket(temp);

				// Storing response
				tcp->readFromSocket(response);

				fileSize = atoi(response);
				writeFileFromNetwork(fileName, tcp);
			}
			break;
		case ID_POST_UPLOAD: // UPLOAD (TCP Mode)  - Not currently used.
			if(tcp != NULL)
			{
				char temp[BUFFER_SIZE] =  START_TRANSFER;
				char fileName[BUFFER_SIZE];
				
				GetWindowText(FileNameBox, fileName, BUFFER_SIZE);
				strcat(temp, fileName);
				// Sending fileName to server
				tcp->writeToSocket(temp);

				// Read From File
				// Write to Socket
			}
			break;
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: writeFileFromNetwork
--
-- DATE: 2013/03/28
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: John Payment
--
-- PROGRAMMER: John Payment
--
-- INTERFACE: void writeFileFromNetwork(char* fileName, TCPClient* client)
--                 char* fileName - the name of the file to be opened
--                 TCPClient* Pointer to the client that we are streaming data from
--
-- RETURNS: void
--
-- NOTES: This function reads binary data from a TCP SOCKET and writes it to a file until it has reached a predefined file size.
----------------------------------------------------------------------------------------------------------------------*/
void writeFileFromNetwork(char* fileName, TCPClient* client)
{
	ofstream file(fileName, std::ofstream::binary);
	char fileChunk[BUFFER_SIZE];
	memset(fileChunk, 0, BUFFER_SIZE);
	int n = 0, totalRecv = 0;

	while(totalRecv < fileSize)
	{
		n = client->readFromSocket(fileChunk);
		if(n + totalRecv  > fileSize)
		{
			n = fileSize - totalRecv;
		} else if(n == -1)
		{
			break;
		}
		file.write(fileChunk, n);
		totalRecv += n;
	}

	file.close();
}
