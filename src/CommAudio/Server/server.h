#ifndef SERVER_H
#define SERVER_H
#include "../utils/utils.h"

#define BUFSIZE     1023
#define MAXADDRSTR  16
#define TIMECAST_ADDR   "234.5.6.8"
#define TIMECAST_PORT   8910
#define TIMECAST_TTL    2

void DisableLoopback(SOCKET *socketfd);
void RunMulticast();
void SetTimeToLive(SOCKET s, u_long TTL);
DWORD WINAPI MicServerSessionThread();
void StartServerMicSession(); //start mic session client thread
int __stdcall SendRoutine(void* instance, void *user_data, libZPlay::TCallbackMessage message, unsigned int param1, unsigned int param2);

#endif