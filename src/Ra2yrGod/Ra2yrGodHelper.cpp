#include "Ra2yrGodHelper.h"
#include <iostream>
#include "windows.h"
#include "psapi.h"
#include <vector>

namespace Ra2yrGodHelper
{
    HMODULE getBaseModuleInfo(const HANDLE hProcess, std::string& baseModuleName) {
        if (hProcess == NULL)
            return NULL; // ��� ������� � ��������

        // ������ ��� ���������� ������ �������
        HMODULE lphModule[1024];
        // ��������� ������ EnumProcessModules.
        // ��������� ���������� ����, ����������� ��� ���������� ���� ������������ ������� ������� lphModule
        DWORD lpcbNeeded(NULL);

        // ��������� ������ �������
        // https://learn.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-enumprocessmodules
        if (!EnumProcessModules(hProcess, lphModule, sizeof(lphModule), &lpcbNeeded))
            return NULL; // �� ������� ��������� ���������� � �������

        // ��������� ���� ����� ��� ������.
        // � �������� �������� ������ ������������ ������ � ���������� ������.
        TCHAR szModName[MAX_PATH];
        // https://learn.microsoft.com/ru-ru/windows/win32/api/psapi/nf-psapi-getmodulefilenameexa
        if (!GetModuleFileNameEx(hProcess, lphModule[0], szModName, sizeof(szModName) / sizeof(TCHAR)))
            return NULL; // �� ������� ��������� ���������� � �������
        std::wstring wModuleName(&szModName[0]); //convert to wstring
        std::string moduleName(wModuleName.begin(), wModuleName.end()); //and convert to string.

        // ������������ ��������
        baseModuleName = moduleName;

        // ������� ������ � �������� 0 ����������� ������ �������� ����� ����������� ������,
        // �� ���� ������� ������� ��������
        return (HMODULE)lphModule[0];
    }

    bool isProcessRunning(HANDLE process)
    {
        DWORD exitCodeOut;

        // GetExitCodeProcess returns zero on failure
        if (GetExitCodeProcess(process, &exitCodeOut) == 0)
        {
            // Optionally get the error
            // DWORD error = GetLastError();
            return false;
        }
        // Return if the process is still active
        return exitCodeOut == STILL_ACTIVE;
    }

    bool CloseHandleSafe(HANDLE& h) {
        if (h != nullptr && h && h != INVALID_HANDLE_VALUE) {
            bool result = CloseHandle(h);
            h = nullptr;
            return result;
        }
        return TRUE;
    }
}
