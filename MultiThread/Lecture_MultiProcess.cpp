#include "stdafx.h"
#include "windows.h"

#define NUL '\0'

HANDLE	g_hPipeRead;
HANDLE	g_hPipeWrite;
HANDLE  g_hChildProcess = NULL;
HANDLE	g_hPipeBkgWrite;
HANDLE	g_hPipeBkgRead;


static int WriteToBkgProcess(char *buff, DWORD nBytesToWrite);
static int ReadFromBkgProcess(char *rdbuff, DWORD nBytesToRead);

int _tmain(int argc, _TCHAR* argv[])
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	SECURITY_ATTRIBUTES sa;
	char Args[4096];
	char *hello = "Hello";
	char read_buff[(512 + 1) * sizeof(TCHAR)];
	DWORD nRdBuffLen = 512;                 // Allow for NULL
	DWORD nBytesRead;
	_TCHAR currdir[512 + 1];
	DWORD currdirlen = 512;

	// Set up the security attributes struct.
	//
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	// Create pipes
	CreatePipe(&g_hPipeBkgRead, &g_hPipeWrite, &sa, 0);
	CreatePipe(&g_hPipeRead, &g_hPipeBkgWrite, &sa, 0);

	SetHandleInformation(g_hPipeWrite, HANDLE_FLAG_INHERIT, 0);
	SetHandleInformation(g_hPipeRead, HANDLE_FLAG_INHERIT, 0);

	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdOutput = g_hPipeBkgWrite;
	si.hStdInput = g_hPipeBkgRead;
	si.hStdError = g_hPipeBkgWrite;

	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	
	// Get path for background .exe file
	GetCurrentDirectory(currdirlen, currdir);
	wcscat(currdir, L"\\Debug\\BackgroundProcess");

	// Create the process in which .exe will execute
	CreateProcess(NULL, currdir, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);

	g_hChildProcess = pi.hProcess; // Handle for bkg process

	WriteFile(g_hPipeWrite, &hello, sizeof(hello)+1, NULL, NULL);
	ReadFile(g_hPipeRead, &read_buff, sizeof(hello), NULL, NULL);

	printf("Recived: %s", read_buff);

	getchar(); // Pause

	TerminateProcess(g_hChildProcess, 0);
	return 0;
}