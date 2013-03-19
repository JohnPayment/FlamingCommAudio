#ifndef SERVER_H
#define SERVER_H

#define BUFSIZE     1024
#define MAXADDRSTR  16
#define TIMECAST_ADDR   "234.5.6.7"
#define TIMECAST_PORT   8910
#define TIMECAST_TTL    2
#define TIMECAST_INTRVL 30

int bind_socket(SOCKET *socketfd);
int join_multicast(SOCKET *socketfd, char *achMcAddr);
int disable_loopback(SOCKET *socketfd);
SOCKADDR_IN set_destination_addr();
void show_usage(void);
void run_multicast();

#endif