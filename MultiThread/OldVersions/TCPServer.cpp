// UDPexample.cpp : Defines the entry point for the console application.
//
// Add WS2_32.lib to the Additional Dependencies for the Linker
//

#include "stdafx.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#define NUL '\0'

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

DWORD WINAPI EchoThread(LPVOID lpParameter)
{
	SOCKET  ClientSocket = (SOCKET)lpParameter;
	char	recvbuf[DEFAULT_BUFLEN];
	int		recvbuflen = DEFAULT_BUFLEN;
	int     iRcvdBytes = 0;
	int		iResult;
    DWORD   iThreadId = GetCurrentThreadId();

    printf("[%x] Connection Open...\n", iThreadId);

    // The send and recv functions both return an integer value of the 
	// number of bytes sent or received, respectively, or an error. 
	// Each function also takes the same parameters: the active socket, 
	// a char buffer, the number of bytes to send or receive, and any 
	// flags to use.

	// Receive until the peer shuts down the connection
	do 
	{
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
		{
			printf("[%x] Bytes received: %d Msg=%s\n", iThreadId, iResult, recvbuf);
			iRcvdBytes = iResult;
		}
		else if (iResult == 0)
			printf("[%x] Connection closing...\n", iThreadId);
		else  
		{
			printf("[%x] recv failed: %d\n", iThreadId, WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

	} while (iResult > 0);

	// Echo the buffer back to the sender
	iResult = send( ClientSocket, recvbuf, iRcvdBytes, 0 );
	if (iResult == SOCKET_ERROR) 
	{
	    printf("[%x] send failed: %d\n", iThreadId, WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	 }

	 printf("[%x] Bytes Sent: %ld\n", iThreadId, iResult);

	// shutdown the connection since we're done
	//
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) 
	{
		printf("[%x] shutdown failed: %d\n", iThreadId, WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(ClientSocket);

    printf("[%x] Connection Closed\n", iThreadId);

    return 0;
}

int wstrcmp(_TCHAR *str1, const char *str2)
{
    int len = strlen(str2);

    while(len--)
    {
        if(*str2++ != (const char)*str1++)
            break;
    }

    if(len >= 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

char *strcvt(_TCHAR *wchar)
{
    static char    cvtstr[1000];
    char    *chr = cvtstr;

    while((unsigned char)*wchar != NUL)
    {
        *chr++ = (unsigned char)*wchar;
        wchar++;
    }
    *chr = NUL;

    return cvtstr;
}

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA	wsaData;
	char	recvbuf[DEFAULT_BUFLEN];
	int		iResult;
	int		recvbuflen = DEFAULT_BUFLEN;
    char    key = '\0';

	struct addrinfo *addressList = NULL;
	struct addrinfo *ptr = NULL;
	struct addrinfo hints;

	if(argc<2)
	{
		printf("usage: %s -client server-name\r\n", argv[0]);
		printf("usage: %s -server\r\n", argv[0]);
		return 1;
	}

	// Initialize Winsock
	//
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) 
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	if(wstrcmp(argv[1], "-client")==0)
	{
		// For a client to communicate on a network, it must connect to a server.
		// Start by connecting to a socket.
		//
		SOCKET  ConnectSocket = INVALID_SOCKET;

		char    sendbuf[400] = "";
        char    *bptr = sendbuf;
		    
		// Validate the parameters
		if (argc != 3) 
		{
			printf("usage: %s -client server-name\n", argv[0]);
			return 1;
		}

		// ****************************************
		// After initialization, a SOCKET object must be instantiated
		// ****************************************

		//
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		// Using the hostname(or IP address) typed in by the user, use DNS to
		// Resolve the server address. We pass a portnumber so that it can
		// be included in the returned addresses.
		//
		iResult = getaddrinfo(strcvt(argv[2]), DEFAULT_PORT, &hints, &addressList);
		if ( iResult != 0 ) 
		{
			printf("getaddrinfo failed: %d\n", iResult);
			WSACleanup();
			return 1;
		}

		// Attempt to connect to an address until one succeeds
		for(ptr=addressList; ptr != NULL ;ptr=ptr->ai_next) 
		{
			// Create a SOCKET for connecting to server
			ConnectSocket = socket(ptr->ai_family, 
									ptr->ai_socktype, 
									ptr->ai_protocol);

			// Check for errors to ensure that the socket is a valid socket.
			//
			if (ConnectSocket == INVALID_SOCKET) 
			{
				printf("Error at socket(): %ld\n", WSAGetLastError());
				freeaddrinfo(addressList);
				WSACleanup();
				return 1;
			}

			// Call the connect function, passing the created socket and the 
			// sockaddr structure as parameters. Check for general errors.

			// Connect to server.
			iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult == SOCKET_ERROR) 
			{
				closesocket(ConnectSocket);
				ConnectSocket = INVALID_SOCKET;
				continue;
			}
			break;
		}

		freeaddrinfo(addressList);

		if (ConnectSocket == INVALID_SOCKET) 
		{
			printf("Unable to connect to server!\n");
			WSACleanup();
			return 1;
		}

		// The send and recv functions both return an integer value of the 
		// number of bytes sent or received, respectively, or an error. 
		// Each function also takes the same parameters: the active socket, 
		// a char buffer, the number of bytes to send or receive, and any 
		// flags to use.


        // THIS IS SILLY.
        // CLEARLY WE DON'T WANT TO KEEP THE CONNECTION OPEN ALL THE TIME
        // THE USER IS TYPING IN THE TEXT. THIS CODE SHOULD BE PLACED
        // BEFORE WE EVEN CREATED THE SOCKET BUT I JUST WANTED TO
        // SHOW THE THREADS RUNNING WHEN WE RUN MULTIPLE CLIENTS SIMULTANOUSLY.
        //
        printf(">");
        while(key != '\r'&&key != '\n')
        {
           *bptr++ = key = _getche();
        }
        printf("\r\n");

        *bptr = '\0';

		// Send a buffer of data
		//
	    iResult = send( ConnectSocket, sendbuf, (int)strlen(sendbuf)+1, 0 );
   	    if (iResult == SOCKET_ERROR) 
        {
	        printf("send failed: %d\n", WSAGetLastError());
    	    closesocket(ConnectSocket);
		    WSACleanup();
	   	    return 1;
        }

		// shutdown the connection since no more data will be sent
		// shutdown() acts like a close() but waits for the receiver
		// to ACK the last data sent by the client.
		// SD_SEND = no more sends allowed
		//
		iResult = shutdown(ConnectSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) 
		{
			printf("shutdown failed: %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return 1;
		}

		// Receive until the peer closes the connection
		do 
		{
		    iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		    if ( iResult > 0 )
			    printf("Bytes received: %d Msg=%s\n", iResult, recvbuf);
		    else if ( iResult == 0 )
			    printf("Connection closed\n");
		    else
			    printf("recv failed: %d\n", WSAGetLastError());

	    } while( iResult > 0 );

		// cleanup
		closesocket(ConnectSocket);
	}
	else
	{
		// For a server to accept client connections, it must be bound to a 
		// network address within the system.
		//
		SOCKET ListenSocket = INVALID_SOCKET;
		SOCKET ClientSocket = INVALID_SOCKET;

		// ****************************************
		// After initialization, a SOCKET object must be instantiated
		// ****************************************
		//
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		// Resolve the server address and port
		iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &addressList);
		if ( iResult != 0 ) 
		{
			printf("getaddrinfo failed: %d\n", iResult);
			WSACleanup();
			return 1;
		}

		// Create a SOCKET for connecting to server
		ListenSocket = socket(addressList->ai_family, addressList->ai_socktype, addressList->ai_protocol);
		if (ListenSocket == INVALID_SOCKET) 
		{
			printf("socket failed: %ld\n", WSAGetLastError());
			freeaddrinfo(addressList);
			WSACleanup();
			return 1;
		}

		// Call the bind function (associate socket with local address), 
		// passing the created socket and sockaddr structure 
		// returned from the getaddrinfo function as parameters. 
		// Check for general errors.
		//
		// Setup the TCP listening socket
		//
		iResult = bind( ListenSocket, addressList->ai_addr, (int)addressList->ai_addrlen);
		if (iResult == SOCKET_ERROR) 
		{
			printf("bind failed: %d\n", WSAGetLastError());
			freeaddrinfo(addressList);
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		freeaddrinfo(addressList);

		// After the socket is bound to an IP address and port on the system, 
		// the server must then listen on that IP address and port for 
		// incoming connection requests.
		// To listen on a socket call the listen function, passing the created 
		// socket and the maximum number of allowed connections to accept as 
		// parameters. Check for general errors.
		//
		iResult = listen(ListenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR) 
		{
			printf("listen failed: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		// Create a continuous loop that checks for connections requests. 
		// If a connection request occurs, call the accept function to 
		// handle the request.
		//
        while(1)
        {
            HANDLE  threadhdl;
            DWORD   threadid;

    		// Accept a client socket
	    	ClientSocket = accept(ListenSocket, NULL, NULL);
		    if (ClientSocket == INVALID_SOCKET) 
		    {
    			printf("accept failed: %d\n", WSAGetLastError());
	    		closesocket(ListenSocket);
		    	WSACleanup();
			    return 1;
    		}

            threadhdl = CreateThread(
                                    NULL,                   // No security attributes reqd
                                    0,                      // Use default thread stack size
                                    EchoThread,             // Thread function
                                    (LPVOID)ClientSocket,   // Pass parameter to function
                                    0,                      // Creation flags (e.g. CREATE_SUSPENDED)
                                    &threadid);             // Return value of thread ID

            // Note: Closing a handle doesn't terminate a thread
            // Since I don't want to use the handle I'll close
            // it immediately.
            //
            CloseHandle(threadhdl);

        }


		// When the client connection has been accepted, close the 
		// original socket, transfer control from the temporary socket 
		// to the original socket, and stop checking for new connections.
		// No longer need server socket.
		//
		closesocket(ListenSocket);

	}

    printf("Exiting...\n");
	while(!_kbhit());

	WSACleanup();

	return 0;
}

