/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: main.cpp - The entry point for the server side of the comm audio application.
--
-- PROGRAM: CommAudio
--
-- FUNCTIONS:
-- int main(int argc, char *argv[])
--
-- DATE: 2013/03/23
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Wright
--
-- PROGRAMMER: Jesse Wright
--
-- NOTES:
-- The main function simply creates threads that will be delegated jobs to handle the clients requests as they come in.
----------------------------------------------------------------------------------------------------------------------*/
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include "server.h"

/*-------------------------------------------------------------------------------------------------------------------- 
-- FUNCTION: main
--
-- DATE: 2013/03/23
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jesse Wright
--
-- PROGRAMMER: Jesse Wright
--
-- INTERFACE: int main (int argc, char *argv[])
--
-- RETURNS: int - 0 upon success
--
-- NOTES: Main entry point to the server.
----------------------------------------------------------------------------------------------------------------------*/
int main(int argc, char *argv[]) 
{
	RunMulticast();
	WSACleanup();

	return (0);
}

