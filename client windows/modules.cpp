#include <iostream>
#include <windows.h>
#include <Secext.h>
#include <string>
#include <stdio.h>
#include <tchar.h>
#include "modules.h"

char* getComputerName()
{
    // Get computer name
    TCHAR computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(computerName) / sizeof(computerName[0]);
    GetComputerName(computerName, &size);

    char* cname = (char*)malloc(100);
    sprintf(cname,"%s",computerName);

    return cname;
}
