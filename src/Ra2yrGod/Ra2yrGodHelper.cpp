#include "Ra2yrGodHelper.h"
#include <iostream>
#include "windows.h"
#include "psapi.h"
#include <vector>

namespace Ra2yrGodHelper
{
    HMODULE getBaseModuleInfo(const HANDLE hProcess, std::string& baseModuleName) {
        if (hProcess == NULL)
            return NULL; // Нет доступа к процессу

        // Массив для сохранения списка модулей
        HMODULE lphModule[1024];
        // Результат вызова EnumProcessModules.
        // Указывает количество байт, необходимых для сохранения всех дескрипторов модулей массива lphModule
        DWORD lpcbNeeded(NULL);

        // Получение списка модулей
        // https://learn.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-enumprocessmodules
        if (!EnumProcessModules(hProcess, lphModule, sizeof(lphModule), &lpcbNeeded))
            return NULL; // Не удалось прочитать информацию о модулях

        // Получение пути файла для модуля.
        // В качестве базового модуля используется первый в полученном списке.
        TCHAR szModName[MAX_PATH];
        // https://learn.microsoft.com/ru-ru/windows/win32/api/psapi/nf-psapi-getmodulefilenameexa
        if (!GetModuleFileNameEx(hProcess, lphModule[0], szModName, sizeof(szModName) / sizeof(TCHAR)))
            return NULL; // Не удалось прочитать информацию о модулях
        std::wstring wModuleName(&szModName[0]); //convert to wstring
        std::string moduleName(wModuleName.begin(), wModuleName.end()); //and convert to string.

        // Возвращаемые значения
        baseModuleName = moduleName;

        // Элемент модуля с индексом 0 практически всегда является самим исполняемым файлом,
        // то есть базовым модулем процесса
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
