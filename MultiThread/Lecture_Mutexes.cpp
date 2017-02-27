#include "stdafx.h"
#include "windows.h"
#include <conio.h>
//#include <ctype.h>

int terminate_app = 0;

char    key = 'a'; // Global data

HANDLE  mutexhdl;
LPCTSTR mutexname = (LPCTSTR)"FirstMutex";


DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	HANDLE  mutexhdl;

	while (1)
	{
		printf("Thread %c\r\n", key);
		if (terminate_app)
			break;
	}

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE  threadhdl;

	threadhdl = CreateThread(NULL, 0, ThreadProc, (LPVOID)0, 0, NULL);

	while (key != 'q')
	{
		key = _getch();
	}

	terminate_app = 1;

	WaitForSingleObject(threadhdl, INFINITE);

	CloseHandle(threadhdl);

	//getchar();
	return 0;
}