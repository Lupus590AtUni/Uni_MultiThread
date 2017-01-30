// CriticalCode.cpp : Defines the entry point for the console application.
//
// 
//

#include "stdafx.h"
#include "windows.h"

int terminate_app = 0;

#define MAXTHREADS  3

char msg[40];

CRITICAL_SECTION g_cs;

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    int threadnum = (int)lpParameter;

    while(1)
    {
        // Demo: If the Enter and Leave Critical Section
        // functions aren't called below, the sprintf 
        // yields leaving another thread to execute after
        // the msg buffer has been filled and before the
        // printf displays it and the threadnum variable.
        // This allows another thread to overwrite the
        // string in the msg buffer before printf is able to
        // display it, so printf may display different numbers
        // for the threadnum i.e. the value in the msg buffer and
        // the value in the variable itself.
        //
        EnterCriticalSection(&g_cs);
        sprintf(msg, "Thread %i", threadnum);
        printf("[%i] %s\r\n", threadnum, msg);
        LeaveCriticalSection(&g_cs);

        if(terminate_app)
            break;

        // Set a value to yield for given number of mS e.g. Sleep(100)
        // Sleep(0) means yield immediately to thread of equal priority.
        // Sleep(INFINITE) means yield (suspend) forever
        //
        Sleep(100);
    }

    return 0;   // Return a value, use GetExitCodeThread function to read
}


int _tmain(int argc, _TCHAR* argv[])
{
    HANDLE  threadhdl[MAXTHREADS-1];
    DWORD   threadid;
    int     index = 0;
    int     num[MAXTHREADS-1];

    InitializeCriticalSection(&g_cs);

    for(index = 0; index < MAXTHREADS; index++)
    {
        num[index] = index;
        threadhdl[index] = CreateThread(
                                NULL,               // No security attributes reqd
                                0,                  // Use default thread stack size
                                ThreadProc,         // Thread function
                                (LPVOID)num[index],// Pass parameter to function
                                0,                  // Creation flags (e.g. CREATE_SUSPENDED)
                                &threadid);         // Return value of thread ID

    }

    for(index = 0; index < 30; index++)
    {
        printf("[Main] cnt=%i\r\n", index); 
        Sleep(100);     // Sleep for a 100ms
    }

    terminate_app = 1;

#if MAXTHREADS > 1
    // The WaitForMultipleObjects function returns when any one 
    // or all of the specified objects (threads) are in the signaled state 
    // or the time-out interval elapses.
    // In this case, we are waiting for thread to terminate (
    //
    WaitForMultipleObjects(
                        MAXTHREADS,     // Number of thread handles in array
                        threadhdl,      // Array of handles
                        TRUE,           // Wait for all threads
                        INFINITE);      // Wait forever, 0=test and return immediately, 
                                        // or specify time in mS
#else
    // The WaitForSingleObject function returns when the specified object is
    // in the signaled state or the time-out interval elapses.
    //
    WaitForSingleObject(threadhdl[0], INFINITE);
#endif

    for(index = 0; index < MAXTHREADS; index++)
    {
        CloseHandle(threadhdl[index]);
    }

    DeleteCriticalSection(&g_cs);

    getchar();          // Pause

	return 0;
}

