#include "stdafx.h"
#include "windows.h"

int terminate_app = 0; // Flag

#define MAXTHREADS  1000

// Thread code
DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	DWORD param = (DWORD)lpParameter;	

	int index = 0;
	while(1)
	{
		printf("Back [%x] cnt=%i\r\n", GetCurrentThreadId(), index);

		if (terminate_app)
		{
			printf("[%x] Thread Terminaating\r\n",GetCurrentThreadId());
			break;
		}
		Sleep(0);
		index++;
	}

	return 0; //exit code
}


int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE  threadhdl[MAXTHREADS];
	DWORD   threadid;
	int     index = 0;
	DWORD	param = 2;  // Some value to pass to thread

	printf("Process ID = %x\r\n", GetCurrentProcessId());
	printf("Thread ID = %x\r\n", GetCurrentThreadId());
	printf("Starting a second thread\r\n");

	for (index = 0; index < MAXTHREADS; index++)
	{
		threadhdl[index] = CreateThread(NULL, 0, ThreadProc, (LPVOID)param, 0, NULL);
	}


	for (index = 0; index < 10; index++)
	{
		printf("Main [%x] cnt=%i\r\n", GetCurrentThreadId(), index);
		Sleep(1);
	}
	Sleep(10);



	printf("Main [%x] Start thread termination\r\n", GetCurrentThreadId());
	terminate_app = 1;

//#if MAXTHREADS > 1
	WaitForMultipleObjects(MAXTHREADS, threadhdl, TRUE, INFINITE);
//#else
//	WaitForSingleObject(threadhdl[0], INFINITE);
//#endif

	for (index = 0; index < MAXTHREADS; index++)
	{
		CloseHandle(threadhdl[index]);
	}


	printf("\r\nTerminated threads\r\n");

	getchar();          // Pause

	return 0;
}