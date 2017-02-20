#include "stdafx.h"
#include "windows.h"

int terminate_app = 0;

#define MAXTHREADS  3

char msg[40]; // Global data

CRITICAL_SECTION g_cs;

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	int threadnum = (int)lpParameter;

	while (1)
	{
		sprintf(msg, "Thread %i", threadnum);
		printf("[%i] %s\r\n", threadnum, msg);

		if (terminate_app)
			break;
		Sleep(100);
	}

	return 0; 
}


int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE  threadhdl[MAXTHREADS - 1];
	DWORD   threadid;
	int     index;

	for (index = 0; index < MAXTHREADS; index++)
	{
		threadhdl[index] = CreateThread(NULL, 0, ThreadProc, (LPVOID)index, 0, &threadid);
	}

	for (index = 0; index < 30; index++)
	{
		printf("[Main] cnt=%i\r\n", index);
		Sleep(100);
	}

	terminate_app = 1; // Terminate all threads

	WaitForMultipleObjects(MAXTHREADS, threadhdl, TRUE, INFINITE);

	for (index = 0; index < MAXTHREADS; index++)
	{
		CloseHandle(threadhdl[index]);
	}

	getchar(); // pause

	return 0;
}