// MultiThread.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    DWORD param = (DWORD)lpParameter;
    int index =0;

    printf("Second thread starting, param = %d\r\n", param);

    for(index = 0; index < 60; index++)
    {
        printf("[%x] cnt=%i\r\n", GetCurrentThreadId(), index);

        // Set a value to yield for given number of mS e.g. Sleep(100)
        // Sleep(0) means yield immediately to thread of equal priority.
        // Sleep(INFINITE) means yield (suspend) forever
        //
        Sleep(0);
    }

    return 0;   // Return a value, use GetExitCodeThread function to read
}


int _tmain(int argc, _TCHAR* argv[])
{
    HANDLE  threadhdl;
    DWORD   threadid;
    int     index = 0;
	DWORD	param = 2;  // Some value to pass to thread

    printf("Process ID = %x\r\n", GetCurrentProcessId());
    printf("Thread ID = %x\r\n", GetCurrentThreadId());
    printf("Starting a second thread\r\n");

    threadhdl = CreateThread(
                            NULL,               // No security attributes reqd
                            0,                  // Use default thread stack size
                            ThreadProc,         // Thread function
                            (LPVOID)param,     // Pass parameter to function
                            0,                  // Creation flags (e.g. CREATE_SUSPENDED)
                            &threadid);         // Return value of thread ID

    //SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

    for(index = 0; index < 30; index++)
    {
        printf("[%x] cnt=%i\r\n", GetCurrentThreadId(), index); 
        //Sleep(100);     // Sleep for a 100ms
    }

    TerminateThread(threadhdl, 0);  // Not a good idea to terminate a thread,
                                    // send a message to cause thread to
                                    // terminate itself.

    CloseHandle(threadhdl);

    printf("\r\nTerminating threads\r\n");

    getchar();          // Pause

	return 0;
}

