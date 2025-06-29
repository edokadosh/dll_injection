#include "pch.h"
#include <stdio.h>
#include <string.h>

#define INJECTING_DLL "Z:\\shared\\dll\\injector\\my_awesome_dll.dll"

#define ERROR_MESSAGE(name) \
	printf("error: " #name " failed with error code: %d\n", GetLastError()); \
	getchar(); \
	return 1;

#pragma comment(lib, "advapi32.lib")

BOOL SetPrivilege(
    HANDLE hToken,          // access token handle
    LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
    BOOL bEnablePrivilege   // to enable or disable privilege
) {
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!LookupPrivilegeValue(
        NULL,            // lookup privilege on local system
        lpszPrivilege,   // privilege to lookup 
        &luid))        // receives LUID of privilege
    {
        printf("LookupPrivilegeValue error: %u\n", GetLastError());
        return FALSE;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if (bEnablePrivilege)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

    // Enable the privilege or disable all privileges.

    if (!AdjustTokenPrivileges(
        hToken,
        FALSE,
        &tp,
        sizeof(TOKEN_PRIVILEGES),
        (PTOKEN_PRIVILEGES)NULL,
        (PDWORD)NULL)) {
        printf("AdjustTokenPrivileges error: %u\n", GetLastError());
        return FALSE;
    }

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)

    {
        printf("The token does not have the specified privilege. \n");
        return FALSE;
    }

    return TRUE;
}


int main() {
    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) {
        printf("OpenProcessToken failed: %d", GetLastError());
        return 1;
    }

    if (!SetPrivilege(hToken, SE_DEBUG_NAME, true)) {
        printf("Failed to enable SeDebugPrivilege\n");
        CloseHandle(hToken);
        return 1;
    }

    printf("SeDebugPrivilege successfully enabled!\n");

    printf("Enter PID to inject: ");
    int pid = 0;
    scanf_s("%d", &pid);

    printf("injecting PID %d\n", pid);

	HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
	if (proc == INVALID_HANDLE_VALUE) {
		ERROR_MESSAGE(OpenProcess)
	}

	LPVOID dll_name = VirtualAllocEx(proc, NULL, strlen(INJECTING_DLL)+1, MEM_COMMIT, PAGE_READWRITE);

	if (dll_name == NULL) {
		ERROR_MESSAGE(VirtualAllocEx)
	}

	if (WriteProcessMemory(proc, dll_name, INJECTING_DLL, strlen(INJECTING_DLL) + 1, NULL) == 0) {
		ERROR_MESSAGE(WriteProcessMemory)
	}

	LPVOID func_to_run = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
	if (func_to_run == NULL) {
		ERROR_MESSAGE(GetProcAddress)
	}
	
	HANDLE thread = CreateRemoteThread(proc, NULL, 0, (LPTHREAD_START_ROUTINE)func_to_run, dll_name, NULL, NULL);

	if (thread == NULL) {
		ERROR_MESSAGE(CreateRemoteThread)
	}


	return 0;
}