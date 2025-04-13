// myrunas.cpp : 定义控制台应用程序的入口点。
//

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <detours.h>
#include <stdio.h>
#include <string>

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		printf("RunasPW的命令参数和系统的runas是一致的，只是可以使用/password:xxx来指定密码\n");
		return -1;
	}

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	si.cb = sizeof(si);
	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));

	std::string cmdLine = GetCommandLine();
	std::string cmdLineNew = cmdLine;
	std::string password;
	size_t idx = cmdLine.find("/password:");
	if (idx != std::string::npos)
	{
		//"/password:xxx"这种情况
		bool bDouhao = false;
		if (cmdLine[idx - 1] == '\"')
		{
			idx -= 1;
			bDouhao = true;
		}
		
		size_t idxEnd = cmdLine.find(' ', idx + 1);
		if (idxEnd != std::string::npos)
		{
			//删除命令行中的密码段
			cmdLineNew = cmdLine.substr(0, idx) + cmdLine.substr(idxEnd + 1);

			std::string passwordCmdLine = cmdLine.substr(idx, idxEnd - idx);
			size_t idxPass = passwordCmdLine.find(':');
			password = passwordCmdLine.substr(idxPass + 1);
			if (password.size() > 0)
			{
				if (bDouhao)// "/password:xxx"
				{
					password.pop_back();
				}
				else if (password[0] == '"' && password.size() > 1)// /password:"xxx"
				{
					password = password.substr(1, password.size() - 2);
				}
			}
		}

	}

	char szMyRunasCMDLine[1024] = { 0 };
	char szRunasCMDLine[1024] = { 0 };
	strcpy(szMyRunasCMDLine, cmdLineNew.c_str());
	strcpy(szRunasCMDLine, "runas ");
	strcat(szRunasCMDLine, strstr(szMyRunasCMDLine + strlen(__argv[0]), __argv[1]));

	std::string env = "password=" + password;//把密码放到环境变量中

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
		(LPVOID)env.c_str(),// Use parent's environment block
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

