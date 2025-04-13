// dllmain.cpp : 定义 DLL 应用程序的入口点。

#define _CRT_SECURE_NO_WARNINGS

#include "pch.h"
#include <detours.h>
#include <stdio.h>
#include <stdlib.h>

typedef BOOL(WINAPI* pfnReadConsole)(
	__in          HANDLE hConsoleInput,
	__out         LPVOID lpBuffer,
	__in          DWORD nNumberOfCharsToRead,
	__out         LPDWORD lpNumberOfCharsRead,
	__in_opt      LPVOID pInputControl
	);

typedef BOOL(WINAPI* pfnCreateProcessWithLogonW)(
	__in          LPCWSTR lpUsername,
	__in          LPCWSTR lpDomain,
	__in          LPCWSTR lpPassword,
	__in          DWORD dwLogonFlags,
	__in          LPCWSTR lpApplicationName,
	__in          LPWSTR lpCommandLine,
	__in          DWORD dwCreationFlags,
	__in          LPVOID lpEnvironment,
	__in          LPCWSTR lpCurrentDirectory,
	__in          LPSTARTUPINFOW lpStartupInfo,
	__out         LPPROCESS_INFORMATION lpProcessInfo
	);

BOOL WINAPI MyReadConsole(
	__in          HANDLE hConsoleInput,
	__out         LPVOID lpBuffer,
	__in          DWORD nNumberOfCharsToRead,
	__out         LPDWORD lpNumberOfCharsRead,
	__in_opt      LPVOID pInputControl
);

BOOL WINAPI MyCreateProcessWithLogonW(
	__in          LPCWSTR lpUsername,
	__in          LPCWSTR lpDomain,
	__in          LPCWSTR lpPassword,
	__in          DWORD dwLogonFlags,
	__in          LPCWSTR lpApplicationName,
	__in          LPWSTR lpCommandLine,
	__in          DWORD dwCreationFlags,
	__in          LPVOID lpEnvironment,
	__in          LPCWSTR lpCurrentDirectory,
	__in          LPSTARTUPINFOW lpStartupInfo,
	__out         LPPROCESS_INFORMATION lpProcessInfo
);

BOOL MByteToWChar(LPCSTR lpcszStr, LPWSTR lpwszStr, DWORD dwSize);

pfnReadConsole realReadConsole = (pfnReadConsole)GetProcAddress(LoadLibrary("Kernel32.dll"), "ReadConsoleW");
pfnCreateProcessWithLogonW realCreateProcessWithLogonW = (pfnCreateProcessWithLogonW)GetProcAddress(LoadLibrary("Advapi32.dll"), "CreateProcessWithLogonW");

HANDLE g_hDLL = NULL;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	char szModuleName[MAX_PATH] = { 0 };
	char szExe[MAX_PATH] = { 0 };
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		
		g_hDLL = hModule;

		GetModuleFileName(NULL, szModuleName, MAX_PATH - 1);
		strcpy(szExe, strrchr(szModuleName, '\\') + 1);

		if (_strcmpi(szExe, "RUNAS.EXE"))
		{
			return TRUE;
		}
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach((PVOID*)&realReadConsole, MyReadConsole);
		DetourAttach((PVOID*)&realCreateProcessWithLogonW, MyCreateProcessWithLogonW);
		DetourTransactionCommit();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


BOOL WINAPI MyReadConsole(
	__in          HANDLE hConsoleInput,
	__out         LPVOID lpBuffer,
	__in          DWORD nNumberOfCharsToRead,
	__out         LPDWORD lpNumberOfCharsRead,
	__in_opt      LPVOID pInputControl
)
{
	//MessageBox(0, "", "", 0);
	*((char*)lpBuffer) = 0x0d;
	*lpNumberOfCharsRead = 1;
	return FALSE;

	//继续则
	//*((char*)lpBuffer) != 0x0d;
	//return TRUE;
}

BOOL WINAPI MyCreateProcessWithLogonW(
	__in          LPCWSTR lpUsername,
	__in          LPCWSTR lpDomain,
	__in          LPCWSTR lpPassword,
	__in          DWORD dwLogonFlags,
	__in          LPCWSTR lpApplicationName,
	__in          LPWSTR lpCommandLine,
	__in          DWORD dwCreationFlags,
	__in          LPVOID lpEnvironment,
	__in          LPCWSTR lpCurrentDirectory,
	__in          LPSTARTUPINFOW lpStartupInfo,
	__out         LPPROCESS_INFORMATION lpProcessInfo
)
{
	//MessageBox(0, "", "", 0);

	WCHAR wcsPassword[128] = { 0 };
	const char* password = ::getenv("password");//从环境变量中获取密码
	if (password)
	{
		MByteToWChar(password, wcsPassword, sizeof(wcsPassword) / sizeof(wcsPassword[0]));
	}

	return realCreateProcessWithLogonW(lpUsername,
		lpDomain,
		wcsPassword,
		dwLogonFlags,
		lpApplicationName,
		lpCommandLine,
		dwCreationFlags,
		lpEnvironment,
		lpCurrentDirectory,
		lpStartupInfo,
		lpProcessInfo);
}

BOOL MByteToWChar(LPCSTR lpcszStr, LPWSTR lpwszStr, DWORD dwSize)
{
	// Get the required size of the buffer that receives the Unicode 
	// string. 
	DWORD dwMinSize;
	dwMinSize = MultiByteToWideChar(CP_ACP, 0, lpcszStr, -1, NULL, 0);

	if (dwSize < dwMinSize)
	{
		return FALSE;
	}

	// Convert headers from ASCII to Unicode.
	MultiByteToWideChar(CP_ACP, 0, lpcszStr, -1, lpwszStr, dwMinSize);
	return TRUE;
}

//必须要导出任意一个函数，否则会报错
extern "C" __declspec(dllexport) void test()
{
}
