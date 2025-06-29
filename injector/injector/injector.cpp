#include "pch.h"
#include <stdio.h>
#include <string.h>
#include <string>

const char* INJECTED_DLL_NAME = "Z:\\shared\\dll\\injector\\my_awesome_dll.dll";

std::string GetLastErrorAsString() {
    DWORD errorCode = GetLastError();
    if (errorCode == 0) {
        return std::string();
    }

    LPSTR messageBuffer = nullptr;

    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);

    LocalFree(messageBuffer);

    return message;
}

int PrintErrorAndExit(const char* func_name) {
    DWORD error_code = GetLastError();
    printf("error: %s failed with error %s\n", func_name, GetLastErrorAsString());
    getchar();
    return 1;
}

#pragma comment(lib, "advapi32.lib")

bool SetPrivilege(
    HANDLE hToken,          // access token handle
    LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
    bool bEnablePrivilege   // to enable or disable privilege
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
        return PrintErrorAndExit("OpenProcess");
	}

    int dll_name_len = strlen(INJECTED_DLL_NAME) + 1;

	LPVOID dll_name = VirtualAllocEx(proc, NULL, dll_name_len, MEM_COMMIT, PAGE_READWRITE);

	if (dll_name == NULL) {
        return PrintErrorAndExit("VirtualAllocEx");
	}

	if (WriteProcessMemory(proc, dll_name, INJECTED_DLL_NAME, strlen(INJECTED_DLL_NAME) + 1, NULL) == 0) {
        return PrintErrorAndExit("WriteProcessMemory");
	}

    HMODULE hModule = GetModuleHandleA("kernel32.dll");
    if (hModule == NULL) {
        return PrintErrorAndExit("module_handle");
    }

	LPVOID func_to_run = GetProcAddress(hModule, "LoadLibraryA");
	if (func_to_run == NULL) {
        return PrintErrorAndExit("WriteProcessMemory");
	}
	
	HANDLE thread = CreateRemoteThread(proc, NULL, 0, (LPTHREAD_START_ROUTINE)func_to_run, dll_name, NULL, NULL);

	if (thread == NULL) {
        return PrintErrorAndExit("CreateRemoteThread");
	}

    VirtualFreeEx(proc, dll_name, dll_name_len, MEM_RELEASE);
    CloseHandle(hToken);
    CloseHandle(proc);
    CloseHandle(hModule);
    CloseHandle(thread);


	return 0;
}