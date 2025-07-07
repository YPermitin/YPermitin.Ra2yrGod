#include "Ra2yrGodHelper.h"
#include <iostream>
#include "windows.h"
#include "psapi.h"
#include <vector>
#include <tlhelp32.h>
#include <tchar.h>
#include <string>

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

    DWORD FindProcessId(const std::wstring& processName) 
    {
        PROCESSENTRY32 processInfo;
        processInfo.dwSize = sizeof(processInfo);

        // Создаем снимок информации обо всех процессах в системе
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
        if (snapshot == INVALID_HANDLE_VALUE) return 0;

        // Извлекаем информацию о процессе из снимка
        Process32First(snapshot, &processInfo);
        do {
            // Сравниваем имена исполняемых файлов без учета регистра, чтобы найти первый процесс по имени
            if (!_wcsicmp(processInfo.szExeFile, processName.c_str())) 
            {   
                // Если строки равны и возвращено значение 0, то закрываем дескриптор снимка процессов,
                // и возвращаем PID найденного процесса
                CloseHandle(snapshot);
                return processInfo.th32ProcessID;
            }
        } while (Process32Next(snapshot, &processInfo)); // Извлекаем информацию о процессе из снимка, если есть

        CloseHandle(snapshot);
        return 0;
    }

    struct WindowSearchData {
        DWORD targetPid;
        std::wstring targetTitle;
        HWND foundHwnd = nullptr;
    };

    BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) 
    {
        // Указатель параметра преобразовываем к указатею на структуру
        WindowSearchData* data = reinterpret_cast<WindowSearchData*>(lParam);

        // Определяем PID для переданного окна
        DWORD pid = 0;
        GetWindowThreadProcessId(hwnd, &pid);

        // Если PID окна и переданного параметра равны, то мы нашли нужное окно для процесса
        if (pid == data->targetPid) 
        {
            // Получаем заголовок окна и сравниваем с переданным значением поиска
            wchar_t title[256];
            GetWindowText(hwnd, title, sizeof(title) / sizeof(wchar_t));
            if (data->targetTitle == title) 
            {
                // Если заголовок окна совпал с искомым, то поиск выполнен успешно.
                // Заполняем стуркутру результатов поиска и возвращаем флаг для остановки обхода окон.
                data->foundHwnd = hwnd;
                return FALSE; // Остановить перечисление
            }
        }

        // Иначе продолжаем поиск
        return TRUE;
    }

    HWND FindWindowByProcessAndTitle(const std::wstring& processName, const std::wstring& windowTitle) 
    {
        // Находим PID по имени процесса
        DWORD pid = FindProcessId(processName);
        if (pid == 0) return nullptr;

        // Инициализация структуры данных для поиска окна процесса,
        // при этом по умоланию заполняем PID и заголовок процесса
        WindowSearchData data{ pid, windowTitle };

        // Поиск окна процесса через обход всех окон текущего пользователя
        EnumWindows( // Перечисляет все окна верхнего уровня на экране
            // Указатель на функцию обратного вызова, определяемую приложением.
            EnumWindowsProc, 
            // Определяемое приложением значение, передаваемое функции обратного вызова.
            reinterpret_cast<LPARAM>(&data)
        );
        return data.foundHwnd;
    }
}
