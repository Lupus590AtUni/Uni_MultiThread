#include "stdafx.h"
#include "windows.h"
#include <list>
#include <sstream>
#include <conio.h>

int terminate_app = 0;

#define MAXTHREADS  3

typedef struct
{
	std::string text;
} MESSAGE;

std::list<MESSAGE> messages;

CRITICAL_SECTION g_cs;

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	MESSAGE txMessage;
	char textNumber[16];
	int threadnum = (int)lpParameter;

	while (1)
	{
		// This is going to generate messages and add them to the list
		txMessage.text = "Hello_";
		sprintf(textNumber, "%d", threadnum);

		txMessage.text += textNumber;

		EnterCriticalSection(&g_cs);
			messages.push_back(txMessage);
		LeaveCriticalSection(&g_cs);


		if (terminate_app)
			break;

		Sleep(20);
	}

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE  threadhdl[MAXTHREADS - 1];
	DWORD   threadid;
	int     index = 0;

	InitializeCriticalSection(&g_cs);
	for (index = 0; index < MAXTHREADS; index++)
	{
		threadhdl[index] = CreateThread(NULL, 0, ThreadProc, (LPVOID)index, 0, &threadid);
	}

	MESSAGE m;

	// Process all recived messages from background threads
	while(!terminate_app)
	{
		if (_kbhit())
			terminate_app = 1;


		EnterCriticalSection(&g_cs);
			if (messages.size() == 0)
			{
				LeaveCriticalSection(&g_cs);
				Sleep(100);
				continue;
			}
			m = messages.front();
			messages.pop_front();
		LeaveCriticalSection(&g_cs);
		printf("[Main] msg=%s\r\n", m.text.c_str());
		Sleep(100);
	}

	WaitForMultipleObjects(MAXTHREADS, threadhdl, TRUE, INFINITE);

	for (index = 0; index < MAXTHREADS; index++)
	{
		CloseHandle(threadhdl[index]);
	}

	DeleteCriticalSection(&g_cs);



	getchar(); // pause

	return 0;
}