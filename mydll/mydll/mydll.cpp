// mydll.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "mydll.h"
#include <stdio.h>

// This is an example of an exported variable
MYDLL_API int nmydll=0;

// This is an example of an exported function.
MYDLL_API int fnmydll(void)
{
    return 0;
}

MYDLL_API int share(void) {
    printf("Hi from share!\n");
    return 0;
}


// This is the constructor of a class that has been exported.
Cmydll::Cmydll()
{
    return;
}
