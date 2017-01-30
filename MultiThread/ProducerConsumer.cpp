// Semaphores.cpp : Defines the entry point for the console application.
//
// 
//

#include "stdafx.h"
#include "windows.h"
#include <conio.h>
//#include <ctype.h>

int terminate_app = 0;

char    key[2] = { 'a', 'b' };
int     wr_index = 0;
int     rd_index = 0;
int     dbg_cnt = 0;

HANDLE  emptyhdl;
LPCTSTR semaphorename = (LPCTSTR)"ProducerSemaphore";
HANDLE  fullhdl;
LPCTSTR semaphorename2 = (LPCTSTR)"ConsumerSemaphore";

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    HANDLE  emptyhdl;
    HANDLE fullhdl;

    // The OpenSemaphore function opens an existing named semaphore object.
    //
    emptyhdl    = OpenSemaphore(SYNCHRONIZE|SEMAPHORE_MODIFY_STATE, // Access to the semaphore object
                                FALSE,                                  // Inherit the handle ? 
                                semaphorename);                         // Names the semaphore to be opened

    fullhdl    = OpenSemaphore(SYNCHRONIZE|SEMAPHORE_MODIFY_STATE, // Access to the semaphore object
                                FALSE,                                  // Inherit the handle ? 
                                semaphorename2);                         // Names the semaphore to be opened
    
    while(1)
    {
        WaitForSingleObject(fullhdl, INFINITE);

        printf("Consumer Thread key[%i]=%c buffer_cnt=%i\r\n", rd_index, key[rd_index], dbg_cnt--);

        if(rd_index + 1 > 1)
        {
            rd_index = 0;
        }
        else
        {
            rd_index++;
        }

        ReleaseSemaphore(emptyhdl, 1, NULL);

        if(terminate_app)
            break;

        printf("Sleep\r\n");
        Sleep(1000); // Yield
    }

    CloseHandle(emptyhdl);
    CloseHandle(fullhdl);

    return 0;   // Return a value, use GetExitCodeThread function to read
}


int _tmain(int argc, _TCHAR* argv[])
{
    HANDLE  threadhdl;
    //DWORD   threadid;
    char    k = 'a';

    // The CreateSemaphore function creates or opens a named or 
    // unnamed semaphore object.
    //
    emptyhdl = CreateSemaphore( NULL,                   // Security attributes -- NULL for none.
                                    0,                  // Initial count for the semaphore object. 
                                    2,                  // Maximum count for the semaphore object 
                                    semaphorename);     // name of the semaphore object. 
                                                        // The name is limited to MAX_PATH characters.

    fullhdl = CreateSemaphore( NULL,                    // Security attributes -- NULL for none.
                                    0,                  // Initial count for the semaphore object. 
                                    2,                  // Maximum count for the semaphore object 
                                    semaphorename2);    // name of the semaphore object. 
                                                        // The name is limited to MAX_PATH characters.
    ReleaseSemaphore(emptyhdl, 2, NULL);

    threadhdl = CreateThread(
                            NULL,
                            0,
                            ThreadProc,
                            (LPVOID)0,
                            0,
                            NULL);

    while(k != 'q')
    {
        // The WaitForSingleObject function returns when the specified object is
        // in the signaled state i.e. semaphore count is greater than zero. 
        // The sum of the semaphore count is decremented by one and thread
        // is allowed to run.
        // Object is in nonsignaled state when count is zero and this thread is
        // blocked until a call to ReleaseSemaphore is made to increase count.
        // Wait forever until it is released.
        //
        WaitForSingleObject(emptyhdl, INFINITE);

        key[wr_index] = k = _getch();

        if(wr_index + 1 > 1)
        {
            wr_index = 0;
        }
        else
        {
            wr_index++;
        }

        printf("Producer buffer_cnt=%i\r\n", ++dbg_cnt);

        // The ReleaseSemaphore function increases the count of the specified 
        // semaphore object by a specified amount.
        //
        ReleaseSemaphore(fullhdl,   // Handle to the semaphore object 
                         1,         // Amount by which the semaphore object's current count is to be increased  
                         NULL);     // Pointer to a variable to receive the previous count for the semaphore
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

