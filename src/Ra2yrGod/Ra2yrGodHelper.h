#pragma once

#include <iostream>
#include "windows.h"
#include <vector>

namespace Ra2yrGodHelper
{
    HMODULE getBaseModuleInfo(const HANDLE hProcess, std::string& baseModuleName);

    bool isProcessRunning(HANDLE process);
}
