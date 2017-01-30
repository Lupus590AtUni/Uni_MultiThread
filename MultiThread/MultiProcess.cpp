// MultiThread.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"

#define NUL '\0'

HANDLE	g_hPipeRead;
HANDLE	g_hPipeWrite;
HANDLE  g_hChildProcess = NULL;
HANDLE	g_hPipeBkgWrite;
HANDLE	g_hPipeBkgRead;


static int WriteToBkgProcess(char *buff, DWORD nBytesToWrite);
static int ReadFromBkgProcess(char *rdbuff, DWORD nBytesToRead);

int _tmain(int argc, _TCHAR* argv[])
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	SECURITY_ATTRIBUTES sa;
    char Args[4096];
    char *hello = "Hello";
	char read_buff[(512+1)*sizeof(TCHAR)];
    DWORD nRdBuffLen = 512;                 // Allow for NULL
    DWORD nBytesRead;
    _TCHAR currdir[512+1];
    DWORD currdirlen = 512;

    // Set up the security attributes struct.
	//
	sa.nLength= sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

    // NOTE: Creating a process in Windows is very expensive in time and resources.
    // Use threads instead.
    // If you want to implement seperate client and servers then it makes sense to use
    // seperate processes.
    //


    // To keep things simple the background process will be a Console Application.
    // To pass data to this background process the easiest way is by using pipes.
    // You can create a window as a background process instead. In which case,
    // you should send windows messages to the process.
    // COPYDATASTRUCT cds;
	// Fill the COPYDATA structure
	// 
	// cds.dwData = SETPIPEHANDLES;      // function identifier
	// cds.cbData = sizeof( MyStruct );  // size of data
	// cds.lpData = &mystructdata;       // data structure
	//
	// Call function, passing data
	//
	// g_hMainWindow = FindWindow(NULL,"Main Window Title")) != NULL)
	// g_hCmdWnd = FindWindow(NULL,"Background Process Title");
	// if( g_hCmdWnd != NULL )
	// {
	//	SendMessage( g_hCmdWnd,
    //               WM_COPYDATA,
    //               (WPARAM)(HWND) g_hMainWindow,   // who's sending data
    //               (LPARAM) (LPVOID) &cds );       // data to send
	// }
    //

	// CreatePipe
    // hReadPipe [out] Pointer to a variable that receives the read handle for the pipe. 
    // hWritePipe[out] Pointer to a variable that receives the write handle for the pipe. 
    // lpPipeAttributes [in] Pointer to a SECURITY_ATTRIBUTES structure that determines whether
    //      the returned handle can be inherited by child processes. 
    //      If lpPipeAttributes is NULL, the handle cannot be inherited.
    //      The lpSecurityDescriptor member of the structure specifies a security descriptor 
    //      for the new pipe. If lpPipeAttributes is NULL, the pipe gets a default security descriptor. 
    //      The ACLs in the default security descriptor for a pipe come from the primary or 
    //      impersonation token of the creator.
    // nSize [in] Size of the buffer for the pipe, in bytes. The size is only a suggestion; 
    //      the system uses the value to calculate an appropriate buffering mechanism. 
    //      If this parameter is zero, the system uses the default buffer size.
    //
    // Create two pipes, one for each direction going to and coming from the background process.
	//
	if (!CreatePipe(&g_hPipeBkgRead,&g_hPipeWrite,&sa,0))
	{
         printf("Failed to create pipe to background process\r\n");
		 return FALSE;
	}

	if (!CreatePipe(&g_hPipeRead,&g_hPipeBkgWrite,&sa,0))
	{
         printf("Failed to create pipe from background process\r\n");
		 return FALSE;
	}

	// Ensure the write handle to the pipe for STDIN is not inherited
	// and the read handle fo STDOUT is not inherited. 
	//
	SetHandleInformation(g_hPipeWrite, HANDLE_FLAG_INHERIT, 0);
	SetHandleInformation(g_hPipeRead, HANDLE_FLAG_INHERIT, 0);

    // Set up the start up info struct.
    // Setup the STDIN and STDOUT for background process.
	ZeroMemory(&si,sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES; //STARTF_USESHOWWINDOW;
	si.hStdOutput = g_hPipeBkgWrite;
	si.hStdInput  = g_hPipeBkgRead;
	si.hStdError  = g_hPipeBkgWrite;
    //si.wShowWindow = SW_HIDE;

	ZeroMemory( &pi, sizeof(PROCESS_INFORMATION) );

	// Launch the application you want to use as a second process.
	// Make sure the .exe file is in the same directory as the game.
	//
    GetCurrentDirectory(currdirlen, currdir);
    wcscat(currdir, L"\\Debug\\BackgroundProcess");
	if (!CreateProcess(NULL,currdir,NULL,NULL,TRUE,
		0,NULL,NULL,&si,&pi))
	{
		printf("Failed to create background process %i\r\n", GetLastError());
		if(GetLastError() == 14001)
		{
			printf("Try rebuilding BackgroundProcess and copy it to Debug folder\r\n");
		}
		return FALSE;
	}

	g_hChildProcess = pi.hProcess;

    printf("Write data to background process %s\n", hello);

    WriteToBkgProcess(hello, (DWORD)strlen(hello)+1);

    nBytesRead = ReadFromBkgProcess(read_buff, nRdBuffLen);

    printf("Read data from background process %u %s\n", nBytesRead, read_buff);

    getchar();          // Pause

    TerminateProcess(g_hChildProcess, 0);

	return 0;
}

inline int WriteToBkgProcess(char *wrbuff, DWORD nBytesToWrite)
{
    DWORD nBytesWritten;

	if (!WriteFile(g_hPipeWrite,wrbuff,nBytesToWrite,&nBytesWritten,NULL) || nBytesWritten == 0)
	{
		if (GetLastError() == ERROR_NO_DATA)
		{
            printf("Pipe to command window closed?? %i\r\n", GetLastError());
			return 0; // Pipe was closed (normal exit path).
		}
		else
		{
            printf("Error writing to command window %i\r\n", GetLastError());
			return 0;
		}
	}
    //CloseHandle(g_hPipeWrite);   Could close the pipe to tell bkg process we've finished 
	FlushFileBuffers(g_hPipeWrite);
	return 1;
}

inline int ReadFromBkgProcess(char *rdbuff, DWORD nBytesToRead)
{
    DWORD nBytesRead = 0;

	if(!ReadFile(g_hPipeRead,rdbuff,nBytesToRead,&nBytesRead,NULL))
	{
        if (GetLastError() != ERROR_BROKEN_PIPE)
		{
			printf("Error reading from command window %i\r\n", GetLastError());
		}
	}
	rdbuff[nBytesRead] = '\0'; // Follow input with a NULL.

	return nBytesRead;
}

