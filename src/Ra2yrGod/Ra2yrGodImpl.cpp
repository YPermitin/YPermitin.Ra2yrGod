#include "Ra2yrGodImpl.h"
#include "Ra2yrGodHelper.h"

#include <ctime>
#include <thread>

namespace Ra2yrGodImpl
{
    Ra2yrGod::Ra2yrGod()
    {
        processId = -1;
        processName = "-";
        processHandle = nullptr;
        processHandleValid = false;
        processBaseModule = nullptr;
        lastProcessHandleCheck = -1;

        playerMoneyFrozen = false;
    }
    bool Ra2yrGod::initProcess() {
        // 1. Находим процесс игры и получаем доступ к процессу
        // Находим окно игры
        HWND hwnd = FindWindowA(NULL, "Ra2: Reborn");
        DWORD procID;
        // Получаем идентификатор процесса
        GetWindowThreadProcessId(hwnd, &procID);
        // Получаем доступ к процессу. В результате получим дескриптор процесса.
        processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);
        if (hwnd == 0x0 || processHandle == 0x0 || procID == 0)
        {
            processName = "-";
            processId = -1;
            return false;
        }
        processId = procID;

        // 2. Находим базовый модуль и его адрес
        std::string baseModuleName;
        processBaseModule = Ra2yrGodHelper::getBaseModuleInfo(processHandle, baseModuleName);
        ULONG_PTR baseModuleAddress = (ULONG_PTR)processBaseModule;

        // 3. Извлекаем имя процесса из полного имени модуля
        int startIndex = baseModuleName.rfind('\\');
        if (startIndex != std::string::npos)
        {
            int baseModuleNameLength = baseModuleName.length() - startIndex - 1;
            baseModuleName = baseModuleName.substr(startIndex + 1, baseModuleNameLength);
            processName = baseModuleName;

            time(&lastProcessHandleCheck);

            return true;
        }

        processName = "-";
        processId = -1;
        return false;
    }
    bool Ra2yrGod::processConnected()
    {
        // Информация о процессе не была инициализирована
        if (processId <= 0)
        {
            return false;
        }

        // Дескриптор процесса не определен
        if (processHandle == nullptr)
        {
            return false;
        }

        // Проверку выполняем не чаще, чем 1 раз в 1 секунд
        time_t currentDateTime;
        time(&currentDateTime);
        int lastCheckTimeLeftSec = std::difftime(currentDateTime, lastProcessHandleCheck);
        if (lastCheckTimeLeftSec < 1)
        {
            return processHandleValid;
        }

        // Проверяем фактически запущенный процесс по дескриптору
        processHandleValid = Ra2yrGodHelper::isProcessRunning(processHandle);
        if (!processHandleValid)
        {
            processId = -1;
            processName = "-";
            processHandle = nullptr;
            processBaseModule = nullptr;
        }
        else
        {
            time(&lastProcessHandleCheck);
        }

        return processHandleValid;
    }
    int Ra2yrGod::getProcessId()
    {
        return processId;
    }
    std::string Ra2yrGod::getProcessName()
    {
        return processName;
    }

    int Ra2yrGod::getPlayerMoney()
    {
        int moneyValue = -1;
       
        // 1-2. Используем данные ранее найденного процесса
        ULONG_PTR baseModuleAddress = (ULONG_PTR)processBaseModule;

        // 3. Получаем значение баланса игрока. 
        // 3.1. Базовый адрес с учетом первого смещения
        ULONG_PTR basePointerAddressWithOffset = baseModuleAddress + 0x0070F6CC;
        int basePointer; // HEX: 209EA5E0, INT: 547268064
        if (ReadProcessMemory(processHandle, (LPVOID)basePointerAddressWithOffset, &basePointer, sizeof(basePointer), 0))
        {
            // 3.2. Адрес №1
            ULONG_PTR moneyPointerAddress_1 = (ULONG_PTR)basePointer + 0;
            int moneyPointer_1; // HEX: 20849450, INT: 545559632
            if (ReadProcessMemory(processHandle, (LPVOID)moneyPointerAddress_1, &moneyPointer_1, sizeof(moneyPointer_1), 0))
            {
                // 3.3. Адрес №2
                ULONG_PTR moneyPointerAddress_2 = (ULONG_PTR)moneyPointer_1 + 0x2C;
                int moneyPointer_2; // HEX: 202CC4E0, INT: 539804896
                if (ReadProcessMemory(processHandle, (LPVOID)moneyPointerAddress_2, &moneyPointer_2, sizeof(moneyPointer_2), 0))
                {
                    // 3.3. Адрес №3
                    ULONG_PTR moneyPointerAddress_3 = (ULONG_PTR)moneyPointer_2 + 0x258;
                    int moneyPointer_3; // HEX: 2084AD40, INT: 545566016
                    if (ReadProcessMemory(processHandle, (LPVOID)moneyPointerAddress_3, &moneyPointer_3, sizeof(moneyPointer_3), 0))
                    {
                        // 3.3. Адрес №4
                        ULONG_PTR moneyPointerAddress_4 = (ULONG_PTR)moneyPointer_3 + 0x4C;
                        int moneyPointer_4; // HEX: 2084A7F0, INT: 545564656
                        if (ReadProcessMemory(processHandle, (LPVOID)moneyPointerAddress_4, &moneyPointer_4, sizeof(moneyPointer_4), 0))
                        {
                            // 3.3. Адрес №5
                            ULONG_PTR moneyPointerAddress_5 = (ULONG_PTR)moneyPointer_4 + 0x2C;
                            int moneyPointer_5; // HEX: 202CC4E0, INT: 539804896
                            if (ReadProcessMemory(processHandle, (LPVOID)moneyPointerAddress_5, &moneyPointer_5, sizeof(moneyPointer_5), 0))
                            {
                                // 3.3. Адрес №6
                                ULONG_PTR moneyPointerAddress_6 = (ULONG_PTR)moneyPointer_5 + 0x30C;
                                if (!ReadProcessMemory(processHandle, (LPVOID)moneyPointerAddress_6, &moneyValue, sizeof(moneyValue), 0))
                                {
                                    moneyValue = -1;
                                }
                            }
                        }
                    }
                }
            }
        }

        return moneyValue;
    }  
    void Ra2yrGod::setPlayerMoney(int money)
    {
        // 1-2. Используем данные ранее найденного процесса
        ULONG_PTR baseModuleAddress = (ULONG_PTR)processBaseModule;

        // 3. Получаем значение баланса игрока.
        // 3.1. Базовый адрес с учетом первого смещения
        ULONG_PTR basePointerAddressWithOffset = baseModuleAddress + 0x0070F6CC;
        int basePointer; // HEX: 209EA5E0, INT: 547268064
        if (ReadProcessMemory(processHandle, (LPVOID)basePointerAddressWithOffset, &basePointer, sizeof(basePointer), 0))
        {
            // 3.2. Адрес №1
            ULONG_PTR moneyPointerAddress_1 = (ULONG_PTR)basePointer + 0;
            int moneyPointer_1; // HEX: 20849450, INT: 545559632
            if (ReadProcessMemory(processHandle, (LPVOID)moneyPointerAddress_1, &moneyPointer_1, sizeof(moneyPointer_1), 0))
            {
                // 3.3. Адрес №2
                ULONG_PTR moneyPointerAddress_2 = (ULONG_PTR)moneyPointer_1 + 0x2C;
                int moneyPointer_2; // HEX: 202CC4E0, INT: 539804896
                if (ReadProcessMemory(processHandle, (LPVOID)moneyPointerAddress_2, &moneyPointer_2, sizeof(moneyPointer_2), 0))
                {
                    // 3.3. Адрес №3
                    ULONG_PTR moneyPointerAddress_3 = (ULONG_PTR)moneyPointer_2 + 0x258;
                    int moneyPointer_3; // HEX: 2084AD40, INT: 545566016
                    if (ReadProcessMemory(processHandle, (LPVOID)moneyPointerAddress_3, &moneyPointer_3, sizeof(moneyPointer_3), 0))
                    {
                        // 3.3. Адрес №4
                        ULONG_PTR moneyPointerAddress_4 = (ULONG_PTR)moneyPointer_3 + 0x4C;
                        int moneyPointer_4; // HEX: 2084A7F0, INT: 545564656
                        if (ReadProcessMemory(processHandle, (LPVOID)moneyPointerAddress_4, &moneyPointer_4, sizeof(moneyPointer_4), 0))
                        {
                            // 3.3. Адрес №5
                            ULONG_PTR moneyPointerAddress_5 = (ULONG_PTR)moneyPointer_4 + 0x2C;
                            int moneyPointer_5; // HEX: 202CC4E0, INT: 539804896
                            if (ReadProcessMemory(processHandle, (LPVOID)moneyPointerAddress_5, &moneyPointer_5, sizeof(moneyPointer_5), 0))
                            {
                                // 3.3. Адрес №6
                                ULONG_PTR moneyPointerAddress_6 = (ULONG_PTR)moneyPointer_5 + 0x30C;

                                WriteProcessMemory(processHandle, (LPVOID)moneyPointerAddress_6, &money, sizeof(money), 0);
                            }
                        }
                    }
                }
            }
        }
    }
    void Ra2yrGod::freezePlayerMoney(bool enable, int targetMoney)
    {
        if (enable)
        {
            playerMoneyFrozen = true;
            std::thread frozenMoneyTask(
                [this, targetMoney]
                {
                    this->freezePlayerMoneyTask(targetMoney);
                }
            );
            frozenMoneyTask.detach();
        }
        else
        {
            playerMoneyFrozen = false;
        }
    }
    bool Ra2yrGod::playerMoneyFrozenState()
    {
        return playerMoneyFrozen;
    }

    void Ra2yrGod::freezePlayerMoneyTask(int targetMoney)
    {
        while (playerMoneyFrozen)
        {
            int playerMoneyProcess = getPlayerMoney();
            int playerMoneyTarget = targetMoney;
            if (playerMoneyProcess != playerMoneyTarget)
            {
                setPlayerMoney(playerMoneyTarget);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
}
