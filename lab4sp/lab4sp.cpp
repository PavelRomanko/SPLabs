#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <string>
#include <algorithm>
#include <Windows.h>

// Контейнер для отсортированных строк
std::vector<std::string> sortedLines;

// Очередь заданий для сортировки
std::queue<std::vector<std::string>> taskQueue;

// Количество сортирующих потоков
const int numThreads = 4;

// Критическая секция для синхронизации доступа к очереди
CRITICAL_SECTION criticalSection;

// Условная переменная для сигнализации о наличии заданий в очереди
CONDITION_VARIABLE conditionVariable;

// Флаг для указания, что все задания обработаны
bool allTasksProcessed = false;

// Функция потока, сортирующая задания
DWORD WINAPI SortThread(LPVOID lpParam) {
    while (true) {
        // Проверяем, есть ли задания в очереди
        EnterCriticalSection(&criticalSection);
        while (taskQueue.empty() && !allTasksProcessed) {
            // Если заданий нет и обработка не завершена, ждем сигнала
            SleepConditionVariableCS(&conditionVariable, &criticalSection, INFINITE);
        }

        // Если все задания обработаны, выходим из цикла
        if (taskQueue.empty() && allTasksProcessed) {
            LeaveCriticalSection(&criticalSection);
            break;
        }

        // Извлекаем задание из очереди
        std::vector<std::string> task = taskQueue.front();
        taskQueue.pop();
        LeaveCriticalSection(&criticalSection);

        // Сортируем задание
        std::sort(task.begin(), task.end());

        // Записываем данные в контейнер для отсортированных строк
        for (const auto& line : task) {
            // Используем std::lower_bound для нахождения позиции, куда вставить строку
            auto insertionPos = std::lower_bound(sortedLines.begin(), sortedLines.end(), line);
            sortedLines.insert(insertionPos, line);
        }
    }

    return 0;
}

int main() {
    // Инициализация критической секции
    InitializeCriticalSection(&criticalSection);

    // Инициализация условной переменной
    InitializeConditionVariable(&conditionVariable);

    // Открываем входной файл
    std::ifstream inputFile("input.txt");
    if (!inputFile.is_open()) {
        std::cout << "Failed to open input file." << std::endl;
        return 1;
    }

    // Читаем строки из файла и разделяем их на части
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(inputFile, line)) {
        lines.push_back(line);
    }
    inputFile.close();

    // Разбиваем строки на части для каждого задания
    const int chunkSize = lines.size() / numThreads;
    for (int i = 0; i < numThreads; ++i) {
        int start = i * chunkSize;
        int end = (i == numThreads - 1) ? lines.size() : (i + 1) * chunkSize;
        std::vector<std::string> task(lines.begin() + start, lines.begin() + end);

        // Добавляем задание в очередь
        EnterCriticalSection(&criticalSection);
        taskQueue.push(task);
        LeaveCriticalSection(&criticalSection);

        // Сигнализируем потокам о наличии заданий
        WakeConditionVariable(&conditionVariable);
    }

    // Устанавливаем флаг, что все задания добавлены
    EnterCriticalSection(&criticalSection);
    allTasksProcessed = true;
    LeaveCriticalSection(&criticalSection);

    // Сигнализируем потокам о завершении обработки заданий
    WakeAllConditionVariable(&conditionVariable);

    // Ожидание завершения потоков-сортировщиков
    std::vector<HANDLE> sortThreads(numThreads);
    for (int i = 0; i < numThreads; ++i) {
        sortThreads[i] = CreateThread(NULL, 0, SortThread, NULL, 0, NULL);
    }
    WaitForMultipleObjects(numThreads, sortThreads.data(), TRUE, INFINITE);

    // Запись отсортированных строк в файл
    std::ofstream outputFile("output.txt");
    if (outputFile.is_open()) {
        for (const auto& line : sortedLines) {
            outputFile << line << std::endl;
        }
        outputFile.close();
    }
    else {
        std::cout << "Failed to open output file." << std::endl;
        return 1;
    }

    // Освобождение ресурсов
    for (int i = 0; i < numThreads; ++i) {
        CloseHandle(sortThreads[i]);
    }

    // Уничтожение критической секции
    DeleteCriticalSection(&criticalSection);

    return 0;
}