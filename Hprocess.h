#pragma once

#include <Windows.h>
#include <TlHelp32.h>

class CHackProcess
{
public:
	PROCESSENTRY32 _gameProcess;
	HANDLE HandleProcess;
	DWORD dwordClient;
	DWORD dwordEngine;
	DWORD FindProcessName(const char* ProcessName, PROCESSENTRY32* pEntry)
	{
		PROCESSENTRY32 ProcessEntry;
		ProcessEntry.dwSize = sizeof(PROCESSENTRY32);
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE)
        {
		    return 0;
        }
		if (!Process32First(hSnapshot, &ProcessEntry))
		{
			CloseHandle(hSnapshot);
			return 0;
		}
		do {
			if (!_strcmpi(ProcessEntry.szExeFile, ProcessName))
			{
				memcpy((void*)pEntry, (void*)&ProcessEntry, sizeof(PROCESSENTRY32));
				CloseHandle(hSnapshot);
				return ProcessEntry.th32ProcessID;
			}
		} while (Process32Next(hSnapshot, &ProcessEntry));
		CloseHandle(hSnapshot);
		return 0;
	}

	template <class c>
    c Read(DWORD addr)
	{
		c cRead;
		ReadProcessMemory(HandleProcess, (PBYTE*)addr, &cRead, sizeof(c), NULL);
		return cRead;
	}

    template<class c>
    BOOL Write(DWORD dwAddress, c ValueToWrite)
    {
        return WriteProcessMemory(HandleProcess, (PBYTE*)dwAddress, &ValueToWrite, sizeof(c), NULL);
    }

	DWORD getThreadByProcess(DWORD DwordProcess)
	{
		THREADENTRY32 ThreadEntry;
		ThreadEntry.dwSize = sizeof(THREADENTRY32);
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE) {
		    return 0;
		}

		if (!Thread32First(hSnapshot, &ThreadEntry)) {
		    CloseHandle(hSnapshot); return 0;
		}

		do {
			if (ThreadEntry.th32OwnerProcessID == DwordProcess)
			{
				CloseHandle(hSnapshot);
				return ThreadEntry.th32ThreadID;
			}
		} while (Thread32Next(hSnapshot, &ThreadEntry));
		CloseHandle(hSnapshot);
		return 0;
	}

	DWORD GetModuleNamePointer(LPSTR LPSTRModuleName, DWORD DwordProcessId)
	{
		MODULEENTRY32 lpModuleEntry = { 0 };
		HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, DwordProcessId);
		if (!hSnapShot) {
            return NULL;
        }
		lpModuleEntry.dwSize = sizeof(lpModuleEntry);
		BOOL RunModule = Module32First(hSnapShot, &lpModuleEntry);
		while (RunModule)
		{
			if (!strcmp(lpModuleEntry.szModule, LPSTRModuleName))
			{
				CloseHandle(hSnapShot);
				return (DWORD)lpModuleEntry.modBaseAddr;
			}
			RunModule = Module32Next(hSnapShot, &lpModuleEntry);
		}
		CloseHandle(hSnapShot);
		return NULL;
	}


//	void runSetDebugPrivs()
//	{
//		HANDLE HandleProcess = GetCurrentProcess(), HandleToken;
//		TOKEN_PRIVILEGES privileges;
//		LUID LUID;
//		OpenProcessToken(HandleProcess, TOKEN_ADJUST_PRIVILEGES, &HandleToken);
//		LookupPrivilegeValue(nullptr, "seDebugPrivilege", &LUID);
//        privileges.PrivilegeCount = 1;
//        privileges.Privileges[0].Luid = LUID;
//        privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
//		AdjustTokenPrivileges(HandleToken, false, &privileges, 0, 0, 0);
//		CloseHandle(HandleToken);
//		CloseHandle(HandleProcess);
//	}



	void RunProcess()
	{
		while (!FindProcessName("csgo.exe", &_gameProcess)) Sleep(12);
		while (!(getThreadByProcess(_gameProcess.th32ProcessID))) Sleep(12);
		HandleProcess = OpenProcess(PROCESS_ALL_ACCESS, false, _gameProcess.th32ProcessID);
        while (dwordClient == 0x0) dwordClient = GetModuleNamePointer("client.dll", _gameProcess.th32ProcessID);
        while (dwordEngine == 0x0) dwordEngine = GetModuleNamePointer("engine.dll", _gameProcess.th32ProcessID);
    }
};


