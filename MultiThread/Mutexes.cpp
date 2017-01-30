// Mutexes.cpp : Defines the entry point for the console application.
//
// 
//

#include "stdafx.h"
#include "windows.h"
#include <conio.h>
//#include <ctype.h>

int terminate_app = 0;

char    key = 'a';

HANDLE  mutexhdl;
LPCTSTR mutexname = (LPCTSTR)"FirstMutex";


DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    HANDLE  mutexhdl;

    // The OpenMutex function opens an existing named mutex object.
    //
    mutexhdl = OpenMutex(MUTEX_ALL_ACCESS,  // Access to the mutex object
                        FALSE,              // Inherit the handle?
                        mutexname);         // Name the mutex to be opened
    
    while(1)
    {
        WaitForSingleObject(mutexhdl, INFINITE);

        printf("Thread %c\r\n", key);

        if(terminate_app)
            break;

        ReleaseMutex(mutexhdl);
        Sleep(0); // Yield, wait for next key press
    }

    CloseHandle(mutexhdl);

    return 0;   // Return a value, use GetExitCodeThread function to read
}


int _tmain(int argc, _TCHAR* argv[])
{
    HANDLE  threadhdl;
    //DWORD   threadid;

    // The CreateMutex function creates or opens a named or 
    // unnamed mutex object.
    //
    mutexhdl = CreateMutex( NULL,           // Security attributes -- NULL for none.
                            FALSE,          // The calling thread obtains initial ownership of the mutex object? 
                            mutexname);     // Name of the semaphore object. 
                                            // The name is limited to MAX_PATH characters.               
    

    threadhdl = CreateThread(
                            NULL,
                            0,
                            ThreadProc,
                            (LPVOID)0,
                            0,
                            NULL);

    while(key != 'q')
    {
        // The WaitForSingleObject function returns when the specified object is
        // in the signaled state i.e. thread gains ownership of the mutex. 
        // Object is in nonsignaled state when another thread owns the mutex and is
        // blocked until a call to ReleaseMutex is made to release ownership.
        // Wait forever until it is released.
        //
        WaitForSingleObject(mutexhdl, INFINITE);

        key = _getch();

        //The ReleaseMutex function releases ownership of the mutex
        //
        ReleaseMutex(mutexhdl); // Handle to the mutex object
    }

    terminate_app = 1;

    // Wait for the thread to finish if its still doing something.
    //
    WaitForSingleObject(threadhdl, INFINITE);

    CloseHandle(threadhdl);

    printf("Exit..");
    getchar();          // Pause

	return 0;
}

