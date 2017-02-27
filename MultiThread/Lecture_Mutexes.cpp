#include "stdafx.h"
#include "windows.h"
#include <conio.h>
//#include <ctype.h>

int terminate_app = 0;

char    key = 'a'; // Global data


LPCTSTR mutexname = (LPCTSTR)"FirstMutex";


DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	HANDLE  mutexhdl;

	mutexhdl = OpenMutex(MUTEX_ALL_ACCESS, FALSE, mutexname);

	while (1)
	{
		WaitForSingleObject(mutexhdl, INFINITE);
		printf("Thread %c\r\n", key);
		ReleaseMutex(mutexhdl);
		if (terminate_app)
			break;

		Sleep(0);
	}

	CloseHandle(mutexhdl);
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE  mutexhdl;
	HANDLE  threadhdl;

	mutexhdl = CreateMutex(NULL, FALSE, mutexname); 

	threadhdl = CreateThread(NULL, 0, ThreadProc, (LPVOID)0, 0, NULL);

	while (key != 'q')
	{
		WaitForSingleObject(mutexhdl, INFINITE);
		key = _getch();
		ReleaseMutex(mutexhdl);
	}

	terminate_app = 1;

	WaitForSingleObject(threadhdl, INFINITE);

	CloseHandle(threadhdl);
	CloseHandle(mutexhdl);

	//getchar();
	return 0;
}