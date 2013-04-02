#ifndef CLIENT_H
#define CLIENT_H

#include "../utils/utils.h"

void WINAPI ClientMulticastThread();
DWORD WINAPI MicSessionThread();
void StartMicSession(); //start mic session client thread
int __stdcall SendRoutine(void* instance, void *user_data, libZPlay::TCallbackMessage message, unsigned int param1, unsigned int param2);
#endif