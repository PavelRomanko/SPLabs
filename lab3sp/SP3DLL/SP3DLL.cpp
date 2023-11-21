#include "SP3DLL.h"
#include "pch.h"

// Функция для поиска и замены строки в виртуальной памяти
extern "C" __declspec(dllexport) void FindAndReplaceString(const char* targetString, const char* replacementString)
{
    // Проходим по всей виртуальной памяти процесса
    MEMORY_BASIC_INFORMATION memInfo;
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    char* currAddr = (char*)sysInfo.lpMinimumApplicationAddress;
    char* endAddr = (char*)sysInfo.lpMaximumApplicationAddress;
    SIZE_T bytesRead;

    while (currAddr < endAddr)
    {
        if (VirtualQuery(currAddr, &memInfo, sizeof(memInfo)) == sizeof(memInfo))
        {
            if (memInfo.State == MEM_COMMIT && (memInfo.Type == MEM_MAPPED || memInfo.Type == MEM_PRIVATE))
            {
                char* buffer = new char[memInfo.RegionSize];
                if (ReadProcessMemory(GetCurrentProcess(), currAddr, buffer, memInfo.RegionSize, &bytesRead))
                {
                    std::string str(buffer, bytesRead);
                    size_t foundPos = str.find(targetString);
                    while (foundPos != std::string::npos)
                    {
                        str.replace(foundPos, strlen(targetString), replacementString);
                        foundPos = str.find(targetString, foundPos + strlen(replacementString));
                    }
                    if (WriteProcessMemory(GetCurrentProcess(), currAddr, str.c_str(), str.size(), nullptr))
                    {
                        // Успешно заменили строку в памяти
                    }
                }
                delete[] buffer;
            }
        }

        currAddr += memInfo.RegionSize;
    }
}