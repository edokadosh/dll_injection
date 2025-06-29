#include "pch.h"
#include <stdio.h>
#define LIBRARY "mydll.dll"


int main() {
	printf("Hello from loader!\n");
	HMODULE hModule = LoadLibraryA(LIBRARY);
	if (hModule == NULL) {
		printf("Failed to load dll!\n");
		DWORD lasterror = GetLastError();
		printf("error: %d", lasterror);
	}
	
	return 0;
}