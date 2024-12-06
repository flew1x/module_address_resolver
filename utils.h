#ifndef UTILS_H
#define UTILS_H

#include <Windows.h>
#include <string>
#include <vector>
#include <tchar.h>

namespace utils
{
    void Log(const char* message);

    DWORD GetModuleBaseAddress(const TCHAR* lpszModuleName, DWORD pID);
    DWORD GetPointerAddress(DWORD gameBaseAddr, DWORD address, const std::vector<DWORD>& offsets);
}

#endif