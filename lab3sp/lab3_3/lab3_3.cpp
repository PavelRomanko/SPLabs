#include <Windows.h>
#include <iostream>

// Прототип функции из DLL
typedef void (*FindAndReplaceStringFunc)(const char*, const char*);

// Функция, которая будет выполнена в удаленном потоке
DWORD WINAPI RemoteThreadProc(LPVOID lpParameter)
{
    // Получение адреса функции из загруженной DLL
    HMODULE dllHandle = static_cast<HMODULE>(lpParameter);
    FARPROC functionAddress = GetProcAddress(dllHandle, "FindAndReplaceString");
    if (functionAddress == NULL)
    {
        std::cerr << "Не удалось найти функцию в DLL." << std::endl;
        return 1;
    }

    // Вызов функции из DLL
    FindAndReplaceStringFunc findAndReplaceFunc = reinterpret_cast<FindAndReplaceStringFunc>(functionAddress);
    findAndReplaceFunc("Hello", "World");

    return 0;
}

int main()
{
    // Загрузка DLL
    HMODULE dllHandle = LoadLibrary(L"SP3DLL.dll");
    if (dllHandle == NULL)
    {
        std::cerr << "Не удалось загрузить DLL." << std::endl;
        return 1;
    }

    // Получение идентификатора процесса, в который будет внедрена DLL
    DWORD processId = 0; // Идентификатор процесса (заменить)
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (hProcess == NULL)
    {
        std::cerr << "Не удалось открыть процесс." << std::endl;
        FreeLibrary(dllHandle);
        return 1;
    }

    // Выделение памяти в удаленном процессе для имени DLL
    const char* dllPath = "SP3DLL.dll";
    LPVOID remoteDllPath = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (remoteDllPath == NULL)
    {
        std::cerr << "Не удалось выделить память в удаленном процессе." << std::endl;
        CloseHandle(hProcess);
        FreeLibrary(dllHandle);
        return 1;
    }

    // Запись имени DLL в выделенную память в удаленном процессе
    if (!WriteProcessMemory(hProcess, remoteDllPath, dllPath, strlen(dllPath) + 1, NULL))
    {
        std::cerr << "Не удалось записать имя DLL в удаленный процесс." << std::endl;
        VirtualFreeEx(hProcess, remoteDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        FreeLibrary(dllHandle);
        return 1;
    }

    // Создание удаленного потока для загрузки и выполнения DLL
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, RemoteThreadProc, dllHandle, 0, NULL);
    if (hThread == NULL)
    {
        std::cerr << "Не удалось создать удаленный поток." << std::endl;
        VirtualFreeEx(hProcess, remoteDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        FreeLibrary(dllHandle);
        return 1;
    }

    // Ожидание завершения удаленного потока
    WaitForSingleObject(hThread, INFINITE);

    // Освобождение ресурсов
    VirtualFreeEx(hProcess, remoteDllPath, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);
    FreeLibrary(dllHandle);

    return 0;
}