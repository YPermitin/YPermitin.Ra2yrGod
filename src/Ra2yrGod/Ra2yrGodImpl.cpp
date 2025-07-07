#include "Ra2yrGodImpl.h"
#include "Ra2yrGodHelper.h"

#include <ctime>
#include <thread>

namespace Ra2yrGodImpl
{
    Ra2yrGod::Ra2yrGod()
    {
        resetState();
    }
    Ra2yrGod::~Ra2yrGod()
    {
        resetState();
    }
    
    bool Ra2yrGod::initProcess() {
        // 0. Перед запуском сбрасываем состояние
        resetState();

        // 1. Находим процесс игры и получаем доступ к процессу
        // Находим окно игры
        HWND hwnd = Ra2yrGodHelper::FindWindowByProcessAndTitle(L"gamerb.exe", L"Ra2: Reborn");
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
    bool Ra2yrGod::processConnected(bool forceCheck)
    {
        // Информация о процессе не была инициализирована
        if (processId <= 0)
        {
            return false;
        }

        // Дескриптор процесса не определен
        if (processHandle == nullptr || processHandle == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        if (!forceCheck)
        {
            // Проверку выполняем не чаще, чем 1 раз в 1 секунд
            time_t currentDateTime;
            time(&currentDateTime);
            int lastCheckTimeLeftSec = std::difftime(currentDateTime, lastProcessHandleCheck);
            if (lastCheckTimeLeftSec < 1)
            {
                return processHandleValid;
            }
        }

        // Проверяем фактически запущенный процесс по дескриптору
        processHandleValid = Ra2yrGodHelper::isProcessRunning(processHandle);
        if (!processHandleValid)
        {
            resetState();
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
       
        if (processConnected())
        {
            // 1-2. Используем данные ранее найденного процесса
            ULONG_PTR baseModuleAddress = (ULONG_PTR)processBaseModule;

            // 3. Получаем значение баланса игрока.
            std::vector<int> offsets
            {
                0x0070F6CC,
                0,
                0x2C,
                0x258,
                0x4C,
                0x2C,
                0x30C
            };

            moneyValue = Ra2yrGodHelper::ReadRemotePointer<int, int>(processHandle, baseModuleAddress, offsets);
        }

        return moneyValue;
    }  
    void Ra2yrGod::setPlayerMoney(int money)
    {
        if (processConnected())
        {
            // 1-2. Используем данные ранее найденного процесса
            ULONG_PTR baseModuleAddress = (ULONG_PTR)processBaseModule;

            // 3. Получаем значение баланса игрока.
            std::vector<int> offsets
            {
                0x0070F6CC,
                0,
                0x2C,
                0x258,
                0x4C,
                0x2C,
                0x30C
            };

            Ra2yrGodHelper::SetRemotePointer<int, int>(processHandle, baseModuleAddress, offsets, money);
        }
    }
    void Ra2yrGod::freezePlayerMoney(bool enable, int targetMoney)
    {
        if (processConnected())
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
    }
    bool Ra2yrGod::playerMoneyFrozenState()
    {
        return playerMoneyFrozen;
    }

    void Ra2yrGod::resetState()
    {
        Ra2yrGodHelper::CloseHandleSafe(processHandle);

        processId = -1;
        processName = "-";
        processHandle = nullptr;
        processHandleValid = false;
        processBaseModule = nullptr;
        lastProcessHandleCheck = -1;

        playerMoneyFrozen = false;
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
