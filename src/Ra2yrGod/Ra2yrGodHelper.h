#pragma once

#include <iostream>
#include "windows.h"
#include <vector>

namespace Ra2yrGodHelper
{
    HMODULE getBaseModuleInfo(const HANDLE hProcess, std::string& baseModuleName);

    bool isProcessRunning(HANDLE process);

    template<typename T, typename R>
    R ReadRemotePointer(HANDLE hProcess,
        T baseAddress,
        std::vector<int> offsets)
    {
        T addr = baseAddress;
        T nextPtr = 0;

        for (auto off : offsets) {
            // читаем из адреса (addr + off)
            if (!ReadProcessMemory(hProcess,
                reinterpret_cast<LPCVOID>(addr + off),
                &nextPtr,
                sizeof(nextPtr),
                nullptr))
            {
                throw std::runtime_error("ReadProcessMemory failed");
            }
            addr = nextPtr;
        }

        return (R)addr; 
    }

    template<typename T, typename R>
    void SetRemotePointer(HANDLE hProcess,
        T baseAddress,
        std::vector<int> offsets,
        R newValue)
    {
        T addr = baseAddress;
        T nextPtr = 0;

        int totalOffsets = offsets.size();
        int currentOffsetNumber = 0;

        for (auto off : offsets) 
        {
            currentOffsetNumber++;

            if (totalOffsets == currentOffsetNumber)
            {
                // Последнее смещение адреса в памяти используется
                // для установки конкретного значения.
                auto newValueAddress = reinterpret_cast<LPVOID>(addr + off);

                if (!WriteProcessMemory(hProcess,
                    newValueAddress,
                    &newValue,
                    sizeof(newValue),
                    nullptr))
                {
                    throw std::runtime_error("WriteProcessMemory failed");
                }
            } else
            {
                // читаем из адреса (addr + off).
                // По этому указателю находим следующий адрес в цепочке.
                if (!ReadProcessMemory(hProcess,
                    reinterpret_cast<LPCVOID>(addr + off),
                    &nextPtr,
                    sizeof(nextPtr),
                    nullptr))
                {
                    throw std::runtime_error("ReadProcessMemory failed");
                }
                addr = nextPtr;
            }            
        }
    }
}
