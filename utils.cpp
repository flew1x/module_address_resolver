#include "Windows.h"
#include <string>
#include <vector>
#include <TlHelp32.h>
#include <tchar.h>
#include "utils.h"
#include <sstream>

namespace utils
{
    static std::string toHexString(int value) {
        std::stringstream ss;
        ss << "0x" << std::hex << value;

        return ss.str();
    }

    void Log(const char* message) 
    {
        OutputDebugStringA(message);
    }

    DWORD GetModuleBaseAddress(const TCHAR* lpszModuleName, DWORD pID) 
    {
        DWORD dwModuleBaseAddress = 0;

        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pID);
        if (hSnapshot == INVALID_HANDLE_VALUE) {
            Log("Failed to create toolhelp snapshot.\n");

            return 0;
        }

        MODULEENTRY32 ModuleEntry32{};
        ModuleEntry32.dwSize = sizeof(MODULEENTRY32);

        if (Module32First(hSnapshot, &ModuleEntry32)) {
            do {
                if (_tcscmp(ModuleEntry32.szModule, lpszModuleName) == 0) {
                    dwModuleBaseAddress = (DWORD)ModuleEntry32.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnapshot, &ModuleEntry32));
        }
        else {
            Log("Failed to enumerate modules.\n");
        }

        CloseHandle(hSnapshot);

        return dwModuleBaseAddress;
    }

    DWORD GetPointerAddress(DWORD gameBaseAddr, DWORD address, const std::vector<DWORD>& offsets) 
    {
        DWORD pID = GetCurrentProcessId();
        HANDLE phandle = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pID);

        if (phandle == NULL) {
            std::string errorMessage = "Failed to open process for reading. Error Code: ";
            errorMessage += std::to_string(GetLastError());
            Log(errorMessage.c_str());

            return NULL;
        }

        DWORD pointerAddress = 0;

        // Read initial pointer address
        if (!ReadProcessMemory(phandle, (LPCVOID)(gameBaseAddr + address), &pointerAddress, sizeof(pointerAddress), NULL)) 
        {
            Log(("Failed to read initial pointer address. Error Code: " + std::to_string(GetLastError())).c_str());
            CloseHandle(phandle);

            return NULL;
        }

        // Begin reading
        DWORD currentAddress = pointerAddress;

        // Process each offset
        for (size_t i = 0; i < offsets.size(); ++i) {
            if (i > 0) {
                currentAddress += offsets[i];  // Add offset to current address
                Log(("Address after offset " + toHexString(offsets[i]) + ": " + toHexString(currentAddress)).c_str());
            }

            // If we're at the last offset, break
            if (i == offsets.size() - 1) {
                break;
            }

            // Read next address
            DWORD nextAddress = 0;
            if (!ReadProcessMemory(phandle, (LPCVOID)(currentAddress), &nextAddress, sizeof(nextAddress), NULL)) {
                Log(("Failed to read level " + std::to_string(i + 1) + " address. Error Code: " + std::to_string(GetLastError())).c_str());
                CloseHandle(phandle);

                return NULL;
            }

            currentAddress = nextAddress;  // Set current address to next address
            Log(("Level " + std::to_string(i + 1) + ": " + toHexString(currentAddress)).c_str());
        }

        CloseHandle(phandle);

        return currentAddress;
    }
}