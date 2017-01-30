// Termination.cpp : Defines the entry point for the console application.
//
// The UNIX operating system supports a wide range of signals. 
// UNIX signals are software interrupts that catch or indicate different types of events. 
// Windows on the other hand supports only a small set of signals that is restricted to exception events only.
// 
// SIGABRT          Abnormal termination. The default action terminates the calling 
//                  program with exit code 3.
// SIGABRT_COMPAT   Same as SIGABRT. For compatibility with other platforms. 
// SIGFPE           Floating-point error, such as overflow, division by zero, or invalid operation. 
//                  The default action terminates the calling program. 
// SIGILL           Illegal instruction. The default action terminates the calling program.
// SIGINT           CTRL+C interrupt. The default action terminates the calling the program with exit code 3.
// SIGSEGV          Illegal storage access. The default action terminates the calling program.
// SIGTERM          Termination request sent to the program. The default action terminates the 
//                  calling program with exit code 3.
// SIG_ERR          A return type from a signal indicating an error has occurred.
//
// A state is signaled or unsignaled.
//
// WaitForSingleObject and WaitForMultipleObjects functions return
// WAIT_ABANDONED   The specified object is a mutex object that was not released by the thread 
//                  that owned the mutex object before the owning thread terminated. 
//                  Ownership of the mutex object is granted to the calling thread, and the mutex 
//                  is set to nonsignaled. 
// WAIT_OBJECT_0    The state of the specified object is signaled (i.e. an interrupt occurred). 
// WAIT_TIMEOUT     The time-out interval elapsed, and the object's state is nonsignaled. 
//

#include "stdafx.h"
#include "windows.h"

int terminate_app = 0;

#define MAXTHREADS  1

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    int param = (int)lpParameter;
    int index =0;

    while(1)
    {
        printf("[%i] cnt=%i\r\n", threadnum, index);

        if(terminate_app)
            break;

        // Set a value to yield for given number of mS e.g. Sleep(100)
        // Sleep(0) means yield immediately to thread of equal priority.
        // Sleep(INFINITE) means yield (suspend) forever
        //
        Sleep(100);
        index++;
    }

    return 0;   // Return a value, use GetExitCodeThread function to read
}


int _tmain(int argc, _TCHAR* argv[])
{
    HANDLE  threadhdl[MAXTHREADS];
    DWORD   threadid;
    int     index = 0;

    for(index = 0; index < MAXTHREADS; index++)
    {

        threadhdl[index] = CreateThread(
                                NULL,       // No security attributes reqd
                                0,          // Use default thread stack size
                                ThreadProc, // Thread function
                                (LPVOID)index,  // Pass parameter to function
                                0,          // Creation flags (e.g. CREATE_SUSPENDED)
                                &threadid); // Return value of thread ID

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

    getchar();          // Pause

	return 0;
}

