/*------------------------------------------------------------

	This is only a place holder so that the client will build without error.

--------------------------------------------------------------*/
#include "client.h"
#include "TCPClient.h"
#include "resource.h"
#include <iostream>
#include <fstream>

using namespace std;

#define IPSIZE 16

void writeFileFromNetwork(char* fileName, TCPClient* client);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

char outputLine[BUFFER_SIZE];
TCPClient* tcp;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
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
		lables.bottom = 290;
		DrawText(hdc, outputLine, -1, &lables,  DT_CENTER | DT_VCENTER);
		  
		EndPaint(hwnd, &ps);
		return 0;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case ID_CONNECTMENU_RADIO:
			//Close P2P
			//CLose FileTransfer
			break;
		case ID_CONNECTMENU_P2P:
			{
				if(tcp != NULL)
				{
					delete tcp;
				}

				char ip[IPSIZE];
				GetWindowText(IPBox, ip, IPSIZE);
				tcp = new TCPClient(SetDestinationAddr(ip, 5150));
				tcp->StartClient();
				tcp->writeToSocket(MICROPHONE);
				//Close Radio
				//CLose FileTransfer
				//send tcp request to server
				
				//start p2p udp session
				//runs the session unitl
				StartMicSession();

			}
			break;
		case ID_CONNECTMENU_FILETRANSFER:
			{
				if(tcp != NULL)
				{
					delete tcp;
				}
				char ip[IPSIZE];
				//Close P2P
				//Close Radio
				GetWindowText(IPBox, ip, IPSIZE);
				tcp = new TCPClient(SetDestinationAddr(ip, 5150));

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
		case ID_POST_FILENAME: // DOWNLOAD
			if(tcp != NULL)
			{
				tcp->writeToSocket(START_TRANSFER);
				char fileName[BUFFER_SIZE];
				char response[BUFFER_SIZE];
				GetWindowText(FileNameBox, fileName, BUFFER_SIZE);
				// Sending fileName to server
				tcp->writeToSocket(fileName);

				// Storing response
				tcp->readFromSocket(response);
				if(!strcmp(response, START_TRANSFER))
				{
					// Start writing file
				}
			}
			break;
		case ID_POST_UPLOAD:
			if(tcp != NULL)
			{
				tcp->writeToSocket(START_UPLOAD);
				char fileName[BUFFER_SIZE];
				char data[BUFFER_SIZE];
				GetWindowText(FileNameBox, fileName, BUFFER_SIZE);
				// Sending fileName to server
				tcp->writeToSocket(fileName);

				ifstream readFile;
				readFile.open(fileName);

				// File Transfer
				while(true)
				{
					readFile.read(data, BUFFER_SIZE);
					tcp->writeToSocket(data);

					if(readFile.eof())
					{
						readFile.close();
						break;
					}
				}
				break;
			}
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

void writeFileFromNetwork(char* fileName, TCPClient* client)
{
	ofstream file(fileName);
	char fileChunk[BUFFER_SIZE];

	while(true)
	{
		client->readFromSocket(fileChunk);
		file << fileChunk;
		if(file.eof())
		{
			file.close();
			break;
		}
	}
}
