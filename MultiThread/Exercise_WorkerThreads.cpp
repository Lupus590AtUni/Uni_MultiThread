#include "stdafx.h"
#include "windows.h"
#include <conio.h>
//#include <ctype.h>

#include <iostream>
using std::cout;

int terminate_app = 0;

int x, y;

bool newData = false;

int data[] = { 2,2,4,4,8,8,2,2,4,4,8,8 };
int dataSize = 6;

const int threadCount = 3;


LPCTSTR mutexname = (LPCTSTR)"FirstMutex";

CRITICAL_SECTION g_cs;
DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	HANDLE  mutexhdl;
	int threadnum = (int)lpParameter;

	mutexhdl = OpenMutex(MUTEX_ALL_ACCESS, FALSE, mutexname);
	int localX, localY;
	
	EnterCriticalSection(&g_cs);
	cout << "Thread [" << threadnum << "] awake\n";
	LeaveCriticalSection(&g_cs);

	while (1)
	{
		WaitForSingleObject(mutexhdl, INFINITE);
		if (newData)
		{
			localX = x;
			localY = y;
			newData = false;
			ReleaseMutex(mutexhdl);
			EnterCriticalSection(&g_cs);
			cout << "Thread [" << threadnum << "] value = " << localX + localY << "\n";
			LeaveCriticalSection(&g_cs);
			Sleep(200);// really intensive task simulation
			EnterCriticalSection(&g_cs);
			cout << "Thread [" << threadnum << "] done with nap\n";
			LeaveCriticalSection(&g_cs);
		}
		else
		{
			ReleaseMutex(mutexhdl);
			Sleep(10);
		}
		
		
		if (terminate_app)
			break;

		
	}

	CloseHandle(mutexhdl);
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE  mutexhdl;
	HANDLE  threadhdl[threadCount];

	mutexhdl = CreateMutex(NULL, FALSE, mutexname);
	InitializeCriticalSection(&g_cs);

	for (int i = 0; i < threadCount; i++)
	{
		threadhdl[i] = CreateThread(NULL, 0, ThreadProc, (LPVOID)i, 0, NULL);
	}

	
	
	for (int i = 0; i < dataSize*2; i += 2)
	{
		WaitForSingleObject(mutexhdl, INFINITE);
		while (newData)
		{
			ReleaseMutex(mutexhdl);
			cout << "Main waiting for data to be taken\n";
			Sleep(10);
			WaitForSingleObject(mutexhdl, INFINITE);
		}
		x = data[i];
		y = data[i + 1];
		EnterCriticalSection(&g_cs);
		cout << "Main pushed value: " << data[i] << "\n";
		LeaveCriticalSection(&g_cs);
		newData = true;
		ReleaseMutex(mutexhdl);
		Sleep(10);
	}


	EnterCriticalSection(&g_cs);
	cout << "Main terminating app\n";
	LeaveCriticalSection(&g_cs);



	terminate_app = 1;

	WaitForMultipleObjects(threadCount, threadhdl, TRUE, INFINITE);

	for (int i = 0; i < threadCount; i++)
	{
		CloseHandle(threadhdl[i]);
	}
	CloseHandle(mutexhdl);
	DeleteCriticalSection(&g_cs);

	cout << " Main done\n";

	getchar();
	return 0;
}