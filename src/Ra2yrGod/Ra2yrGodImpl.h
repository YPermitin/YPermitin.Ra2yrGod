#pragma once
#include <string>
#include "windows.h"

namespace Ra2yrGodImpl
{
    class Ra2yrGod
    {
        std::string processName;
        int processId;
        HANDLE processHandle;
        time_t lastProcessHandleCheck;
        bool processHandleValid;
        HMODULE processBaseModule;

        bool playerMoneyFrozen;
    public:
        Ra2yrGod();

        bool initProcess();
        bool processConnected();

        int getProcessId();
        std::string getProcessName();

        int getPlayerMoney();
        void setPlayerMoney(int money);
        bool playerMoneyFrozenState();
        void freezePlayerMoney(bool enable, int targetMoney);

    private:
        void freezePlayerMoneyTask(int targetMoney);
    };
}
