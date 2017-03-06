#include "stdafx.h"
#include "windows.h"
#include <conio.h>
//#include <ctype.h>

int terminate_app = 0;

char    key = 'a';


LPCTSTR semaphorename = (LPCTSTR)"FirstSemaphore";


DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	HANDLE  semaphorehdl;
	//LONG    semaphorecount;

	semaphorehdl = OpenSemaphore(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, FALSE, semaphorename);

	while (1)
	{
		WaitForSingleObject(semaphorehdl, INFINITE);
		printf("Thread %c\r\n", key);
		ReleaseSemaphore(semaphorehdl, 1, NULL);

		if (terminate_app)
			break;
	}





	CloseHandle(semaphorehdl);

	return 0;  
}

int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE  threadhdl;
	//DWORD   threadid;
	LONG    semaphorecount;
	HANDLE  semaphorehdl;

	semaphorehdl = CreateSemaphore(NULL, 1, 1, semaphorename);

	threadhdl = CreateThread(NULL, 0, ThreadProc, (LPVOID)0, 0, NULL);

	while (key != 'q')
	{
		WaitForSingleObject(semaphorehdl, INFINITE);
		key = _getch();
		ReleaseSemaphore(semaphorehdl, 1, &semaphorecount);
	}

	terminate_app = 1;
	WaitForSingleObject(threadhdl, INFINITE);
	CloseHandle(threadhdl);
	CloseHandle(semaphorehdl);

	//getchar();        

	return 0;
}