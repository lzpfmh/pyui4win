#include "stdafx.h"
#include "WinSys.h"

#include <DbgHelp.h>
#include <Shlwapi.h>

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "shlwapi.lib")

void DumpMiniDump(HANDLE hFile, PEXCEPTION_POINTERS excpInfo)
{
	if (excpInfo == NULL) 
	{
		__try 
		{
			RaiseException(EXCEPTION_BREAKPOINT, 0, 0, NULL);
		} 
		__except(DumpMiniDump(hFile, GetExceptionInformation()),
			EXCEPTION_CONTINUE_EXECUTION) 
		{
		}
	} 
	else
	{
		MINIDUMP_EXCEPTION_INFORMATION eInfo;
		eInfo.ThreadId = GetCurrentThreadId();
		eInfo.ExceptionPointers = excpInfo;
		eInfo.ClientPointers = FALSE;
		MiniDumpWriteDump(
			GetCurrentProcess(),
			GetCurrentProcessId(),
			hFile,
			MiniDumpNormal,
			excpInfo ? &eInfo : NULL,
			NULL,
			NULL);
	}
}

#define CRASH_DUMP_FILE		TEXT("MultiWin")
LONG WINAPI Local_UnhandledExceptionFilter(PEXCEPTION_POINTERS pExcept)
{
	TCHAR szDir[MAX_PATH] = {0};
	GetModuleFileName(NULL, szDir, MAX_PATH);
	PathRemoveFileSpec(szDir);
	PathAddBackslash(szDir);
	lstrcat(szDir, _T("Dump\\"));
	CreateDirectory(szDir, NULL);

	SYSTEMTIME st = {0};
	GetLocalTime(&st);
	TCHAR szModuleName[MAX_PATH] = {0};
	wsprintf(szModuleName, _T("%s%s-%02d%02d%02d%02d%02d%02d.dmp"), szDir, CRASH_DUMP_FILE,
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	HANDLE hFile = CreateFile(szModuleName,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
		NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		DumpMiniDump(hFile, pExcept);
		CloseHandle(hFile);
	}

	return 0;
}
