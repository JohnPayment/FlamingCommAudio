#ifndef SERVER_H
#define SERVER_H
#include "../utils/utils.h"

#define BUFSIZE     1024
#define MAXADDRSTR  16
#define TIMECAST_ADDR   "234.5.6.7"
#define TIMECAST_PORT   8910
#define TIMECAST_TTL    2

void DisableLoopback(SOCKET *socketfd);
void RunMulticast();
int SendToMulticastGroup(SOCKET s, char* buf, const struct sockaddr *dest);
void SetTimeToLive(SOCKET s, u_long TTL);
#endif