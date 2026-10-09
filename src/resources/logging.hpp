#pragma once
#include "spdlog/spdlog.h"

class Logging final
{
private:
    static std::string GetLinuxOSVersion();
public:
    static void ShowConsole();
    static void Initialize();
    static void LogSysInfo();

    bool bConsoleShown = false;
    std::chrono::time_point<std::chrono::high_resolution_clock> initStartTime;
    bool bVerboseLogging = true;
};

/// Global logging instance
inline Logging g_Logging;
