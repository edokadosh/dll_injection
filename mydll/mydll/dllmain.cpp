// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <stdio.h>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    MessageBox(NULL, TEXT("Hello Injected"), TEXT("Hello world!"), MB_OK);
    printf("Hello from dll!\n");
    return true;
}


