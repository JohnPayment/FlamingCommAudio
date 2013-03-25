#include "client.h"
/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: ReadFromMulticastGroup
--
-- DATE: 2013/03/24
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Wright
--
-- PROGRAMMER: Jesse Wright
--
-- INTERFACE: int ReadFromMulticastGroup(SOCKET s, char* buf, struct sockaddr *from)
--
-- RETURNS: int - number of bytes read
--
-- NOTES: Wrapper for recieving data from the multicast group
----------------------------------------------------------------------------------------------------------------------*/
int ReadFromMulticastGroup(SOCKET s, char* buf, struct sockaddr *from)
{
	int result;
	int size = sizeof(struct sockaddr);
	if((result = recvfrom(s, buf, BUFLEN, 0, from, &size) == SOCKET_ERROR))
	{
		printf("recvfrom() error. Err: %d\n", WSAGetLastError());
	}
	return result;
}