// Semaphores.cpp : Defines the entry point for the console application.
//
// 
//

#include "stdafx.h"
#include "windows.h"
#include <conio.h>
//#include <ctype.h>

int terminate_app = 0;

char    key = 'a';

HANDLE  semaphorehdl;
LPCTSTR semaphorename = (LPCTSTR)"FirstSemaphore";


DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    HANDLE  semaphorehdl;
    LONG    semaphorecount;

    // The OpenSemaphore function opens an existing named semaphore object.
    //
    semaphorehdl    = OpenSemaphore(SYNCHRONIZE|SEMAPHORE_MODIFY_STATE, // Access to the semaphore object
                                FALSE,                                  // Inherit the handle ? 
                                semaphorename);                         // Names the semaphore to be opened
    
    while(1)
    {
        WaitForSingleObject(semaphorehdl, INFINITE);

        printf("Thread %c\r\n", key);

        ReleaseSemaphore(semaphorehdl, 1, &semaphorecount);

        if(terminate_app)
            break;

        Sleep(0); // Yield, wait for next key press
    }

    CloseHandle(semaphorehdl);

    return 0;   // Return a value, use GetExitCodeThread function to read
}


int _tmain(int argc, _TCHAR* argv[])
{
    HANDLE  threadhdl;
    //DWORD   threadid;
    LONG    semaphorecount;

    // The CreateSemaphore function creates or opens a named or 
    // unnamed semaphore object.
    //
    semaphorehdl = CreateSemaphore( NULL,               // Security attributes -- NULL for none.
                                    1,                  // Initial count for the semaphore object. 
                                    1,                  // Maximum count for the semaphore object 
                                    semaphorename);     // name of the semaphore object. 
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
        // in the signaled state i.e. semaphore count is greater than zero. 
        // The sum of the semaphore count is decremented by one and thread
        // is allowed to run.
        // Object is in nonsignaled state when count is zero and this thread is
        // blocked until a call to ReleaseSemaphore is made to increase count.
        // Wait forever until it is released.
        //
        WaitForSingleObject(semaphorehdl, INFINITE);

        key = _getch();

        //The ReleaseSemaphore function increases the count of the specified 
        // semaphore object by a specified amount.
        //
        ReleaseSemaphore(semaphorehdl,          // Handle to the semaphore object 
                        1,                      // Amount by which the semaphore object's current count is to be increased  
                        &semaphorecount);       // Pointer to a variable to receive the previous count for the semaphore
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

