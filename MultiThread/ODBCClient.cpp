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

#define CHARNAME_TEXT_LEN  50
#define CHARTYPE_TEXT_LEN  50

#pragma pack(1)
typedef struct {
    unsigned short  msgtype;
    unsigned short  length;
} ApplicationMsg;

typedef struct {
    unsigned char   name[CHARNAME_TEXT_LEN];
    unsigned char   type[CHARTYPE_TEXT_LEN];
} ApplicationRecordData;

#define DEFAULT_BUFLEN (sizeof(ApplicationMsg)+(sizeof(ApplicationRecordData)*10))

void getstring(char *buffer)
{
    char    *bptr = buffer;
    char    key = '\0';

    while(key != '\r'&&key != '\n')
    {
        *bptr++ = key = _getche();
    }
    bptr--;
    *bptr = '\0';
    printf("\r\n");
}

void PackApplicationMsg(char *pbuf, ApplicationMsg *msg)
{
    pbuf[0] = (msg->msgtype >> 8)&0xFF;
    pbuf[1] = msg->msgtype&0xFF;
    pbuf[2] = (msg->length >> 8)&0xFF;
    pbuf[3] = msg->length&0xFF;

}

void UnPackApplicationMsg(ApplicationMsg *msg, char *pbuf)
{
    msg->msgtype = ((pbuf[0]&0xFF) << 8)|(pbuf[1]&0xFF);
    msg->length = ((pbuf[2]&0xFF) << 8)|(pbuf[3]&0xFF); 
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

	SOCKET  ConnectSocket = INVALID_SOCKET;

	char    sendbuf[400] = "";
    ApplicationMsg  msg;

    // Initialize Winsock
	//
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) 
	{
		printf("WSAStartup failed: %d\n", iResult);
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

    printf("Enter Server>");
    getstring(recvbuf);
    if(recvbuf[0] == '\r'||recvbuf[1] == '\n')
    {
        strcpy(recvbuf, "127.0.0.1");
    }

	// Using the hostname(or IP address) typed in by the user, use DNS to
	// Resolve the server address. We pass a portnumber so that it can
	// be included in the returned addresses.
	//
	iResult = getaddrinfo(recvbuf, DEFAULT_PORT, &hints, &addressList);
	if ( iResult != 0 ) 
	{
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

    while(1)
    {
        key = '\0';
        while(key == '\0')
        {
            printf("Menu:\n");
            printf("1. Insert\n");
            printf("2. Select\n");
            printf("3. Exit\n");
            printf(">");           
        
            key = _getche();

            switch(key)
            {
            case '1':
                int bpos;
                msg.msgtype = 1;
                msg.length = sizeof(ApplicationMsg)+CHARNAME_TEXT_LEN+CHARTYPE_TEXT_LEN;
                PackApplicationMsg(sendbuf, &msg);
                printf("\nName>");
                bpos = sizeof(ApplicationMsg);
                getstring(sendbuf+bpos);
                printf("Type>");
                bpos += CHARNAME_TEXT_LEN;
                getstring(sendbuf+bpos);
                break;
            case '2':
                msg.msgtype = 2;
                msg.length = sizeof(ApplicationMsg);
                PackApplicationMsg(sendbuf, &msg);
                break;
            case '3':
	            freeaddrinfo(addressList);
			    WSACleanup();
                return 1;
            default:
                key = '\0';
                continue;
            }        
            printf("\r\n");
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

	    if (ConnectSocket == INVALID_SOCKET) 
	    {
	        printf("Unable to connect to server!\n");
		    WSACleanup();
		    return 1;
	    }

		// Send a buffer of data
		//
	    iResult = send( ConnectSocket, sendbuf, (int)msg.length, 0 );
   	    if (iResult == SOCKET_ERROR) 
        {
	        printf("send failed: %d\n", WSAGetLastError());
    	    closesocket(ConnectSocket);
		    WSACleanup();
	   	    return 1;
        }

#ifdef  DO_SHUTDOWN_SIGNAL

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

        if(key == '2')
        {
            int     iByteCnt = 0;

		    do 
		    {
		        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		        if ( iResult > 0 )
                {
                    int bpos = 0;
                    ApplicationRecordData rdata;

                    printf("Bytes received: %u Total:%u\n", iResult, iByteCnt);
              
                    // The first thing we receive is the message header.
                    // Subsequent receive calls will return the database records
                    //
                    if(iByteCnt == 0)
                    {
                        UnPackApplicationMsg(&msg, recvbuf+bpos);
			            printf("Msg=%i %i\n", msg.msgtype, msg.length);
                        bpos = sizeof(ApplicationMsg);
                        iByteCnt = iResult;
                        iResult -= sizeof(ApplicationMsg);
                    }
                    else
                    {
                        iByteCnt += iResult;
                    }

                    while(iResult > 0)
                    {
                        strncpy((char *)rdata.name, recvbuf+bpos, CHARNAME_TEXT_LEN); 
                        bpos += CHARNAME_TEXT_LEN;
                        strncpy((char *)rdata.type, recvbuf+bpos, CHARTYPE_TEXT_LEN); 
                        bpos += CHARTYPE_TEXT_LEN;
			            printf("Msg = %s %s\n", rdata.name, rdata.type);
                        iResult -= sizeof(ApplicationRecordData);
                    }
                }
		        else if ( iResult == 0 )
                {
			        printf("Connection closed\n");
                    break;
                }
		        else
                {
			        printf("recv failed: %d\n", WSAGetLastError());
                    break;
                }

	        } while(iByteCnt < msg.length);
        }
#else
       // There appears to be a problem with calling shutdown() before receiving
        // data from the server (at least on my machine). I think this might be
        // due to running the client and server being on the same machine, since in
        // principle this shouldn't be a problem (in fact it's the way
        // Microsofts examples work).
        //


        // If this was a SELECT then read all the records returned.
        //
        if(key == '2')
        {
            int     iByteCnt = 0;

		    do 
		    {
		        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		        if ( iResult > 0 )
                {
                    int bpos = 0;
                    ApplicationRecordData rdata;

                    printf("Bytes received: %u Total:%u\n", iResult, iByteCnt);
              
                    // The first thing we receive is the message header.
                    // Subsequent receive calls will return the database records
                    //
                    if(iByteCnt == 0)
                    {
                        UnPackApplicationMsg(&msg, recvbuf+bpos);
			            printf("Msg=%i %i\n", msg.msgtype, msg.length);
                        bpos = sizeof(ApplicationMsg);
                        iByteCnt = iResult;
                        iResult -= sizeof(ApplicationMsg);
                    }
                    else
                    {
                        iByteCnt += iResult;
                    }

                    while(iResult > 0)
                    {
                        strncpy((char *)rdata.name, recvbuf+bpos, CHARNAME_TEXT_LEN); 
                        bpos += CHARNAME_TEXT_LEN;
                        strncpy((char *)rdata.type, recvbuf+bpos, CHARTYPE_TEXT_LEN); 
                        bpos += CHARTYPE_TEXT_LEN;
			            printf("Msg = %s %s\n", rdata.name, rdata.type);
                        iResult -= sizeof(ApplicationRecordData);
                    }
                }
		        else if ( iResult == 0 )
                {
			        printf("Connection closed\n");
                    break;
                }
		        else
                {
			        printf("recv failed: %d\n", WSAGetLastError());
                    break;
                }

	        } while(iByteCnt < msg.length);
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
#endif

	    // cleanup
	    closesocket(ConnectSocket);
    }

	freeaddrinfo(addressList);

    printf("Exiting...\n");
	while(!_kbhit());

	WSACleanup();

	return 0;
}



