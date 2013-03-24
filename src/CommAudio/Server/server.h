#ifndef SERVER_H
#define SERVER_H
#include "../utils/utils.h"

#define BUFSIZE     1024
#define MAXADDRSTR  16
#define TIMECAST_ADDR   "234.5.6.7"
#define TIMECAST_PORT   8910
#define TIMECAST_TTL    2
#define TIMECAST_INTRVL 30

int disable_loopback(SOCKET *socketfd);
void show_usage(void);
void run_multicast();

#endif