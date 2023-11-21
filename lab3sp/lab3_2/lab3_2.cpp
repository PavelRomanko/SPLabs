#include <Windows.h>
#include <iostream>

// Прототип функции из DLL
typedef void (*FindAndReplaceStringFunc)(const char*, const char*);

int main()
{
    // Загрузка DLL
    HMODULE dllHandle = LoadLibrary(L"SP3DLL.dll");
    if (dllHandle == NULL)
    {
        std::cerr << "Не удалось загрузить DLL." << std::endl;
        return 1;
    }

    // Получение адреса функции из DLL
    FindAndReplaceStringFunc findAndReplaceFunc = reinterpret_cast<FindAndReplaceStringFunc>(
        GetProcAddress(dllHandle, "FindAndReplaceString"));
    if (findAndReplaceFunc == NULL)
    {
        std::cerr << "Не удалось найти функцию в DLL." << std::endl;
        FreeLibrary(dllHandle);
        return 1;
    }

    // Вызов функции из DLL
    const char* targetString = "Hello";
    const char* replacementString = "World";
    findAndReplaceFunc(targetString, replacementString);

    // Выгрузка DLL
    FreeLibrary(dllHandle);

    return 0;
}