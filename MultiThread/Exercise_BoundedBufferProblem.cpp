#include "stdafx.h"
#include "windows.h"
#include <conio.h>
//#include <ctype.h>

bool terminate_app = false;

#define BUFFER_COUNT 5

char buffer[BUFFER_COUNT];

const LPCTSTR EMPTY_BUFFER_SEMAPHORE = (LPCTSTR)"emptyBufferSemaphore";
const LPCTSTR FULL_BUFFER_SEMAPHORE = (LPCTSTR)"fullBufferSemaphore";

DWORD WINAPI consumerThread(LPVOID lpParameter)
{
	HANDLE  emptyBufferSemaphore =  OpenSemaphore(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, FALSE, EMPTY_BUFFER_SEMAPHORE);
	HANDLE  fullBufferSemaphore = OpenSemaphore(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, FALSE, FULL_BUFFER_SEMAPHORE);

	int readPos = 0;
	while (true)
	{
		if (readPos >= BUFFER_COUNT)
			readPos = 0;

		
		char internalBuffer;
		WaitForSingleObject(fullBufferSemaphore, INFINITE);
			internalBuffer = buffer[readPos];
		ReleaseSemaphore(emptyBufferSemaphore, 1, NULL);

		printf("Thread %c\r\n", internalBuffer);
		readPos++;
		Sleep(1000); // 'Slow process'

		if (terminate_app)
			break;
	}


	CloseHandle(emptyBufferSemaphore);
	CloseHandle(fullBufferSemaphore);

	return 0;
}



int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE  comsumerThreadHandle = CreateThread(NULL, 0, consumerThread, (LPVOID)0, 0, NULL);

	HANDLE  emptyBufferSemaphore = CreateSemaphore(NULL, BUFFER_COUNT, BUFFER_COUNT, EMPTY_BUFFER_SEMAPHORE);
	HANDLE  fullBufferSemaphore = CreateSemaphore(NULL, 0, BUFFER_COUNT, FULL_BUFFER_SEMAPHORE);

	int writePos = 0;

	char key = 'k';
	while (key != 'q')
	{
		if (writePos >= BUFFER_COUNT)
			writePos = 0;

		key = _getch();
		printf("Main recived char, adding to buffer\r\n");

		WaitForSingleObject(emptyBufferSemaphore, INFINITE);
			buffer[writePos] = key;
		ReleaseSemaphore(fullBufferSemaphore, 1, NULL);

		writePos++;
		printf("Main char added to buffer\r\n");
	}

	WaitForSingleObject(comsumerThreadHandle, INFINITE);
	CloseHandle(comsumerThreadHandle);
	CloseHandle(emptyBufferSemaphore);
	CloseHandle(fullBufferSemaphore);

	return 0;
}