// myrunas.cpp : 定义控制台应用程序的入口点。
//

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <detours.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		MessageBox(GetForegroundWindow(), "MyRunas只是一个外壳，在启动runas时修改了它的某些代码，使其支持自动输入密码，其中密码是在MyRunas.ini文件中设置的，除此之外MyRunas用法和runas命令完全相同。"
			"在XP、Win7、Win2008下测试通过。\n\nMyRunas v1.0 2014/3\nby YangFan\nEmail:522419441@qq.com", "MyRunas", MB_ICONINFORMATION);
		return -1;
	}

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	si.cb = sizeof(si);
	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));

	char szMyRunasCMDLine[1024] = { 0 };
	char szRunasCMDLine[1024] = { 0 };
	strcpy(szMyRunasCMDLine, GetCommandLine());
	strcpy(szRunasCMDLine, "runas ");
	strcat(szRunasCMDLine, strstr(szMyRunasCMDLine + strlen(__argv[0]), __argv[1]));

	char szDLL[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szDLL, 1024);
	strcpy(strrchr(szDLL, '\\') + 1, "InputPasswordHelper.dll");///runasdll.dll必须要导出函数，否则会报错
	// Start the child process. 
	if (!DetourCreateProcessWithDll(
		NULL,			// No module name (use command line)
		szRunasCMDLine, // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi,
		szDLL,
		NULL)
		)
	{
		printf("CreateProcess failed (%d)\n", GetLastError());
		return -1;
	}

	// Wait until child process exits.
	WaitForSingleObject(pi.hProcess, INFINITE);

	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);



	return 0;
}

