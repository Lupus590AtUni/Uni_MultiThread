// BackgroundProcess.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "string.h"

//FILE	*g_rdfp;
//FILE	*g_wrfp;

int _tmain(int argc, _TCHAR* argv[])
{
    HANDLE	hStdin = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	char	rdbuff[2048];
	char	wrbuff[2048];
	DWORD	rd_bytes;
	DWORD	wr_bytes;

	// If you want to output text to this console window or read characters from it
    // then you will need to open file handles to the console because stdin and stdout
    // are being redirected (piped) to the main process.
	//
	//g_wrfp = fopen("CON", "w");
	//g_rdfp = fopen("CON", "r");

	while(1)
	{
		memset(rdbuff, '/0', 2048);

	    if(!ReadFile(hStdin, rdbuff, 2048, &rd_bytes, NULL) || rd_bytes == 0)
	    {
		    break;
	    }

        sprintf(wrbuff, "BKG Received: size:%u Msg:%s", rd_bytes, rdbuff);

        if(!WriteFile(hStdout, wrbuff, (DWORD)strlen(wrbuff)+1, &wr_bytes, NULL) || wr_bytes == 0)	// Could use printf(rdbuff);
		{
			break;
		}

		// Must flush output buffers or else redirection 
		// will be problematic.
		//
		FlushFileBuffers(hStdout);
        //CloseHandle(g_hPipeBkgWrite); Could close pipe to signal process is done.
	}

	return 0;
}

