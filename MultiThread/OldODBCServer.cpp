// ODBC_Console.cpp : Defines the entry point for the console application.
//
/*
The first task for any ODBC application is to load the Driver Manager; 
how this is done is operating-system dependent. 
For example, on a computer running Microsoft® Windows NT® Server/Windows 2000 Server, 
Windows NT Workstation/Windows 2000 Professional, or Microsoft 
Windows® 95/98, the application either links to the Driver Manager library or calls 
LoadLibrary to load the Driver Manager DLL.


*/
#include "stdafx.h"
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sql.h>
#include <sqlext.h>
#include <stdio.h>
//#include <mbstring.h>
#include <malloc.h>
#include <conio.h>

#define MYSQLSUCCESS(rc) ((rc==SQL_SUCCESS)||(rc==SQL_SUCCESS_WITH_INFO))

#define NUL '\0'
#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

#define MAX_ERR_MSGSZ   50
#define CHARNAME_TEXT_LEN  50
#define CHARTYPE_TEXT_LEN  50

#define MAX_SQL_LEN     200

SQLWCHAR    errsqlmsg[SQL_MAX_MESSAGE_LENGTH+1];
char        errmsg[SQL_MAX_MESSAGE_LENGTH+1000];
SQLSMALLINT errmsglen;

#pragma pack(1)
typedef struct {
    unsigned short  msgtype;
    unsigned char   name[CHARNAME_TEXT_LEN];
    unsigned char   type[CHARTYPE_TEXT_LEN];
} ApplicationMsg;

void PackApplicationMsg(char *pbuf, ApplicationMsg *msg)
{
    pbuf[0] = (msg->msgtype >> 8)&0xFF;
    pbuf[1] = msg->msgtype&0xFF;
    memcpy(pbuf+2, msg->name, CHARNAME_TEXT_LEN); 
    memcpy(pbuf+CHARNAME_TEXT_LEN+2, msg->type, CHARTYPE_TEXT_LEN); 
}

void UnPackApplicationMsg(ApplicationMsg *msg, char *pbuf)
{
    msg->msgtype = ((pbuf[0]&0xFF) << 8)|(pbuf[1]&0xFF);
    memcpy(msg->name, pbuf+2, CHARNAME_TEXT_LEN); 
    memcpy(msg->type, pbuf+CHARNAME_TEXT_LEN+2, CHARTYPE_TEXT_LEN); 
}

char *InsertRecord(SQLCHAR *charname, SQLCHAR *chartype, int *sendbuflen)
{
    SQLHENV henv1;
    SQLRETURN ret;
    SQLHDBC     hdbc1;
    SQLWCHAR    datasrcname[SQL_MAX_DSN_LENGTH +1];
   SQLHSTMT     hstmt1;

   SQLINTEGER   charnamelen = SQL_NTS, chartypelen = SQL_NTS;
   SQLWCHAR     sqlstatement[MAX_SQL_LEN];
   SQLINTEGER   modifiedrowcnt;

   char         *sendbuf = NULL;

   *sendbuflen = 0;

    // Before an application can call any other ODBC function, is to initialize the 
    // ODBC environment and allocate an environment handle.
    //

    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1);

    // Before an application allocates a connection, it must set the 
    // SQL_ATTR_ODBC_VERSION environment attribute. This attribute states 
    // that the application follows the ODBC 2.x or ODBC 3.x specification.
    // We'll use ODBC 3.x.
    //

    ret = SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0); 
    if(!MYSQLSUCCESS(ret))
    {
        SQLFreeHandle(SQL_HANDLE_ENV, henv1); 
        printf("Environment Attribute Error :%i\n", ret);
        return NULL;
    }

    // Before the application can connect to a data source or driver, 
    // it must allocate a connection handle
    //
    ret = SQLAllocHandle(SQL_HANDLE_DBC, henv1, &hdbc1);
   if (!MYSQLSUCCESS(ret))
   {
        SQLFreeHandle(SQL_HANDLE_ENV, henv1); 
        printf("Failed to allocate connection handle :%i\n", ret);
        return NULL;
   }

   // SQLConnect is the simplest connection function. It requires a data source name
   // and accepts an optional user ID and password. It works well for applications 
   // that hard-code a data source name and do not require a user ID or password.
   //
   //SQLCHAR datasrcname[SQL_MAX_DSN_LENGTH];   // Data source name
   //_mbscpy(datasrcname, (const unsigned char *)"Northwind");
    wcscpy(datasrcname, L"CharacterDNS");

   ret = SQLConnect(hdbc1, 
                    datasrcname, SQL_NTS,   // Data source name and length (SQL_NTS = Nul Terminated String)
                    NULL, 0,                // Username and length
                    NULL, 0);               // Password and length

   if (!MYSQLSUCCESS(ret))
   { 
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc1); 
        printf("Failed to connect to database :%i\n", ret);
        return NULL;
   }

   // SQLSetConnectAttr sets attributes that govern aspects of connections
   //
   ret = SQLSetConnectAttr(hdbc1,
                           SQL_ATTR_ACCESS_MODE,
                           (SQLPOINTER)SQL_MODE_READ_WRITE, // SQL_MODE_READ_ONLY
                           0);
   if (!MYSQLSUCCESS(ret))
   {
        SQLFreeHandle(SQL_HANDLE_ENV, henv1);
        SQLDisconnect(hdbc1);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc1); 
        printf("Failed to set connection attribute :%i\n", ret);
        return NULL;
   }

   // Before the application can execute a statement, it must allocate a statement handle
   //
   SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1);

   wcscpy(sqlstatement, L"INSERT INTO character(name, type) VALUES (?, ?)");
   
   // Prepare the SQL statement with parameter markers.
   //
   ret = SQLPrepare(hstmt1, sqlstatement, SQL_NTS);

   if (MYSQLSUCCESS(ret))
   {
       SQLBindParameter(hstmt1,             // Statement handle
                        1,                  // Parameter number (starts at 1 for first database field)
                        SQL_PARAM_INPUT,    // Input (output only if stored procedure)
                        SQL_C_CHAR,         // C data type
                        SQL_CHAR,           // SQL data type
                        CHARNAME_TEXT_LEN,  // Column size is maximum number of digits or length in chars
                        0,                  // Decimal digits of the column (maximum number of digits to the right of the decimal point)
                        charname,           // Parameter value ptr
                        0,                  // Length of the ParameterValuePtr buffer in bytes
                        &charnamelen);       // A pointer to a buffer for the parameter's length

       SQLBindParameter(hstmt1,
                        2,
                        SQL_PARAM_INPUT,
                        SQL_C_CHAR,
                        SQL_CHAR,
                        CHARTYPE_TEXT_LEN,
                        0,
                        chartype,
                        0,
                        &chartypelen);

       // Now we can set the values
       //
       //strcpy((char *)description, "Hello world");
       //strcpy((char *)description, "Hello world");
       
       // Execute statement with the new parameter values.
       //
       ret = SQLExecute(hstmt1); 
       if (!MYSQLSUCCESS(ret))
       {
           printf("INSERT failed\r\n");
       }
       else
       {
           SQLRowCount(hstmt1, &modifiedrowcnt);
           sendbuf = (char *)malloc(50);
           sprintf(sendbuf, "Num modified rows = %i\r\n", modifiedrowcnt);
           *sendbuflen = strlen(sendbuf)+1;
       }
   }
  
   // Close the cursor.
   SQLFreeStmt(hstmt1, SQL_UNBIND); // Release all column buffers bound by SQLBindCol for the given StatementHandle
   SQLCloseCursor(hstmt1);
   
   SQLFreeHandle(SQL_HANDLE_STMT, hstmt1);
   SQLFreeHandle(SQL_HANDLE_ENV, henv1);
   SQLDisconnect(hdbc1);
   SQLFreeHandle(SQL_HANDLE_DBC, hdbc1);

   return sendbuf;
}

char *ReadRecords(int *sendbuflen)
{
    SQLHENV henv1;
    SQLRETURN ret;
    SQLHDBC     hdbc1;
    SQLWCHAR    datasrcname[SQL_MAX_DSN_LENGTH +1];
   SQLWCHAR     dbname[100];
   SQLSMALLINT  dbnamesz;
   SQLHSTMT     hstmt1;
   ApplicationMsg txmsg;
   SQLINTEGER   charnamelen = SQL_NTS, chartypelen = SQL_NTS;

   SQLWCHAR     sqlstatement[MAX_SQL_LEN];
   char         *sendbuf = NULL;

   *sendbuflen = 0;

    // Before an application can call any other ODBC function, is to initialize the 
    // ODBC environment and allocate an environment handle.
    //

    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1);

    // Before an application allocates a connection, it must set the 
    // SQL_ATTR_ODBC_VERSION environment attribute. This attribute states 
    // that the application follows the ODBC 2.x or ODBC 3.x specification.
    // We'll use ODBC 3.x.
    //

    ret = SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0); 
    if(!MYSQLSUCCESS(ret))
    {
        SQLFreeHandle(SQL_HANDLE_ENV, henv1); 
        printf("Environment Attribute Error :%i", ret);
        return NULL;
    }

    // Before the application can connect to a data source or driver, 
    // it must allocate a connection handle
    //
    ret = SQLAllocHandle(SQL_HANDLE_DBC, henv1, &hdbc1);
   if (!MYSQLSUCCESS(ret))
   {
        SQLFreeHandle(SQL_HANDLE_ENV, henv1); 
        printf("Failed to allocate connection handle :%i", ret);
        return NULL;
   }

   // SQLConnect is the simplest connection function. It requires a data source name
   // and accepts an optional user ID and password. It works well for applications 
   // that hard-code a data source name and do not require a user ID or password.
   //
   //SQLCHAR datasrcname[SQL_MAX_DSN_LENGTH];   // Data source name
   //_mbscpy(datasrcname, (const unsigned char *)"Northwind");
    wcscpy(datasrcname, L"CharacterDNS");

   ret = SQLConnect(hdbc1, 
                    datasrcname, SQL_NTS,    // Data source name and length (SQL_NTS = Nul Terminated String)
                    NULL, 0,                            // Username and length
                    NULL, 0);                           // Password and length

   if (!MYSQLSUCCESS(ret))
   {
        SQLFreeHandle(SQL_HANDLE_ENV, henv1); 
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc1); 
        printf("Failed to connect to database :%i", ret);
        return NULL;
   }

   // SQLSetConnectAttr sets attributes that govern aspects of connections
   //
   ret = SQLSetConnectAttr(hdbc1,
                           SQL_ATTR_ACCESS_MODE,
                           (SQLPOINTER)SQL_MODE_READ_ONLY,
                           0);
   if (!MYSQLSUCCESS(ret))
   {
        SQLFreeHandle(SQL_HANDLE_ENV, henv1);
        SQLDisconnect(hdbc1);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc1); 
        printf("Failed to set connection attribute :%i", ret);
        return NULL;
   }

   // Check that the database driver and database source support the
   // required functionality.
   //
   
   ret = SQLGetInfo(hdbc1,
                SQL_DATABASE_NAME,
                (SQLPOINTER)dbname,
                (SQLSMALLINT)sizeof(dbname),
                &dbnamesz);

   if (!MYSQLSUCCESS(ret))
   {
        SQLFreeHandle(SQL_HANDLE_ENV, henv1);
        SQLDisconnect(hdbc1);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc1); 
        printf("Failed to read driver info :%i", ret);
        return NULL;
   }

   // Before the application can execute a statement, it must allocate a statement handle
   //
   SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1);

   wcscpy(sqlstatement, L"SELECT name,type FROM character");
   //ret = SQLPrepare(hstmt1, sqlstatement, SQL_NTS);
   //ret = SQLExecute(hstmt1);
   ret = SQLExecDirect(hstmt1, sqlstatement, SQL_NTS);

   if (MYSQLSUCCESS(ret))
   {
       // SQLBindCol binds application data buffers to columns (fields) in the result set
       // Fields are numbered from 1=name, 2=type
       //
       SQLBindCol(hstmt1, 1, SQL_C_CHAR, txmsg.name, CHARNAME_TEXT_LEN, &charnamelen);
       SQLBindCol(hstmt1, 2, SQL_C_CHAR, txmsg.type, CHARTYPE_TEXT_LEN, &chartypelen);

       while((ret = SQLFetch(hstmt1)) != SQL_NO_DATA)
       {
           // Instead of SQLBindCol above you can use SQLGetData
           // SQLGetData(hstmt1, 1, SQL_C_SSHORT, &orderid, 0, &orderidlen);

           if(charnamelen != SQL_NULL_DATA
           && chartypelen != SQL_NULL_DATA)
           {
               txmsg.msgtype = 3;
               sendbuf = (char *)realloc(sendbuf, *sendbuflen+sizeof(txmsg));
               PackApplicationMsg(sendbuf+*sendbuflen, &txmsg);
               *sendbuflen += sizeof(txmsg);
           }
       }

   }

   // Close the cursor.
    SQLCloseCursor(hstmt1);
   
   SQLFreeHandle(SQL_HANDLE_STMT, hstmt1);
   SQLFreeHandle(SQL_HANDLE_ENV, henv1);
   SQLDisconnect(hdbc1);
   SQLFreeHandle(SQL_HANDLE_DBC, hdbc1);

   return sendbuf;
}

DWORD WINAPI DatabaseThread(LPVOID lpParameter)
{
	SOCKET  ClientSocket = (SOCKET)lpParameter;
	char	recvbuf[DEFAULT_BUFLEN];
	int		recvbuflen = DEFAULT_BUFLEN;
	int     iRcvdBytes = 0;
    char    *sendbuf;
    int     sendbuflen = 0;
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
			printf("[%x] Bytes received: %d\n", iThreadId, iResult);
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

    ApplicationMsg  character;
    UnPackApplicationMsg(&character, recvbuf);
    switch(character.msgtype)
    {
    case 1:
        sendbuf = InsertRecord(character.name, character.type, &sendbuflen);
        break;
    case 2:
        sendbuf = ReadRecords(&sendbuflen);
        break;
    default:
        sendbuf = NULL;
        break;
    }

    if(sendbuf != NULL)
    {
	    // Echo the buffer back to the sender
	    iResult = send( ClientSocket, sendbuf, sendbuflen, 0 );
	    if (iResult == SOCKET_ERROR) 
	    {
	        printf("[%x] send failed: %d\n", iThreadId, WSAGetLastError());
		    closesocket(ClientSocket);
		    WSACleanup();
            free(sendbuf);
		    return 1;
	     }
        free(sendbuf);
     }

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

    //while(recv(ClientSocket, recvbuf, recvbuflen, 0) > 0);

	// cleanup
	//closesocket(ClientSocket);

    printf("[%x] Connection Closed\n", iThreadId);

    return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA	wsaData;
	int		iResult;
	int		recvbuflen = DEFAULT_BUFLEN;
    char    key = '\0';

	struct addrinfo *addressList = NULL;
	struct addrinfo *ptr = NULL;
	struct addrinfo hints;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;	
    
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
                               DatabaseThread,          // Thread function
                               (LPVOID)ClientSocket,    // Pass parameter to function
                               0,                       // Creation flags (e.g. CREATE_SUSPENDED)
                               &threadid);              // Return value of thread ID

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

    printf("Exiting...\n");
	while(!_kbhit());

	WSACleanup();

	return 0;
}