#include <windows.h>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

// Глобальные переменные
std::vector<std::wstring> tableData; // Данные таблицы
int tableRowCount = 3; // Количество строк таблицы
int tableColumnCount = 3; // Количество столбцов таблицы

// Прототипы функций
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void LoadTableData();
void DrawTable(HWND hwnd, HDC hdc, RECT clientRect);

// Функция загрузки данных таблицы из текстового файла
void LoadTableData()
{
    std::wifstream file("text.txt");
    if (!file)
    {
        MessageBox(NULL, L"Ошибка при открытии файла!", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    tableData.clear();
    std::wstring line;
    while (std::getline(file, line))
    {
        // Удаление пробелов из строки
        //line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
        tableData.push_back(line);
    }

    file.close();
}

void DrawTable(HWND hwnd, HDC hdc, RECT clientRect)
{
    int cellWidth = clientRect.right / tableColumnCount;
    int cellHeight = clientRect.bottom / tableRowCount;

    // Очистка области клиента
    FillRect(hdc, &clientRect, (HBRUSH)(COLOR_WINDOW + 1));

    // Отрисовка таблицы
    for (int row = 0; row < tableRowCount; ++row)
    {
        for (int col = 0; col < tableColumnCount; ++col)
        {
            RECT cellRect;
            cellRect.left = col * cellWidth;
            cellRect.top = row * cellHeight;
            cellRect.right = (col + 1) * cellWidth;
            cellRect.bottom = (row + 1) * cellHeight;

            int fontSize = 50; // Начальный размер шрифта
            HFONT hFont = CreateFont(fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial");

            SelectObject(hdc, hFont);

            RECT textRect = cellRect;
            DrawText(hdc, tableData[0].c_str(), -1, &textRect, DT_CENTER | DT_CALCRECT | DT_WORDBREAK);

            while (textRect.right - textRect.left > cellRect.right - cellRect.left || textRect.bottom - textRect.top > cellRect.bottom - cellRect.top)
            {
                fontSize--; // Уменьшение размера шрифта
                DeleteObject(hFont);
                hFont = CreateFont(fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                    CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial");

                SelectObject(hdc, hFont);

                textRect = cellRect;
                DrawText(hdc, tableData[0].c_str(), -1, &textRect, DT_CENTER | DT_CALCRECT | DT_WORDBREAK);
            }

            SelectObject(hdc, hFont);

            DrawText(hdc, tableData[0].c_str(), -1, &cellRect, DT_CENTER | DT_WORDBREAK);

            // Отрисовка границ ячеек
            DrawEdge(hdc, &cellRect, EDGE_RAISED, BF_LEFT | BF_TOP);
        }
    }
}

// Основная функция программы
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Регистрация класса окна
    const wchar_t CLASS_NAME[] = L"TableWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Создание окна
    HWND hwnd = CreateWindowEx(
        0,                          // дополнительные стили окна
        CLASS_NAME,                 // имя класса окна
        L"Отрисовка таблицы",       // заголовок окна
        WS_OVERLAPPEDWINDOW,        // стиль окна
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, // позиция и размеры окна
        NULL,                       // родительское окно
        NULL,                       // меню окна
        hInstance,                  // идентификатор приложения
        NULL                        // дополнительная информация
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    // Загрузка данных таблицы
    LoadTableData();

    // Отображение окна
    ShowWindow(hwnd, nCmdShow);

    // Цикл обработки сообщений
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// Обработчик сообщений окна
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SIZE:
    {
        // Перерасчет размеров таблицы при изменении размеров окна
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        InvalidateRect(hwnd, &clientRect, TRUE);
        break;
    }
    case WM_PAINT:
    {
        // Отрисовка таблицы при необходимости
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT clientRect;
        GetClientRect(hwnd, &clientRect);

        DrawTable(hwnd, hdc, clientRect);

        EndPaint(hwnd, &ps);
        break;
    }
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        break;
    }
    default:
    {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    }

    return 0;
}