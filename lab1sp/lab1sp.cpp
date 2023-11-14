#include "framework.h"
#include "lab1sp.h"
#pragma comment(lib, "Msimg32.lib")

// Глобальные переменные
HBITMAP hBitmap = NULL;
HWND hwnd = NULL;
UINT_PTR timerId;
int imageWidth = 100;
int imageHeight = 100;
int imageLeft = 0;
int imageTop = 0;
const wchar_t* imagePath = L"ship.bmp";
const int windowWidth = 800;
const int windowHeight = 600;
const int imageSpeed = 5;

// Прототипы функций
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void UpdateImagePosition();

// Главная функция приложения
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Регистрация класса окна
	const wchar_t CLASS_NAME[] = L"MyWindowClass";

	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	// Создание окна
	hwnd = CreateWindowEx(
		0,                              // Дополнительные стили окна
		CLASS_NAME,                     // Имя класса окна
		L"lab1sp",                      // Заголовок окна
		WS_OVERLAPPEDWINDOW,            // Стили окна
		CW_USEDEFAULT, CW_USEDEFAULT,   // Положение окна
		windowWidth, windowHeight,      // Размер окна
		NULL,                           // Дескриптор родительского окна
		NULL,                           // Дескриптор меню окна
		hInstance,                      // Дескриптор текущего экземпляра приложения
		NULL                            // Дополнительные параметры создания окна
	);

	if (hwnd == NULL)
	{
		return 0;
	}

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

// Функция обновления позиции изображения
void UpdateImagePosition()
{
	// Получение размеров клиентской области окна
	RECT clientRect;
	GetClientRect(hwnd, &clientRect);
	int clientWidth = clientRect.right - clientRect.left;
	int clientHeight = clientRect.bottom - clientRect.top;

	// Проверка состояния клавиш W, A, S, D
	if (GetAsyncKeyState('W') & 0x8000)
	{
		imageTop -= imageSpeed;
	}
	if (GetAsyncKeyState('A') & 0x8000)
	{
		imageLeft -= imageSpeed;
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
		imageTop += imageSpeed;
	}
	if (GetAsyncKeyState('D') & 0x8000)
	{
		imageLeft += imageSpeed;
	}
}

// Функция обработки сообщений окна
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
	{
		// Создание таймера
		timerId = SetTimer(hwnd, 1, 10, NULL);

		// Загрузка изображения
		hBitmap = (HBITMAP)LoadImage(NULL, imagePath, IMAGE_BITMAP, imageWidth, imageHeight, LR_LOADFROMFILE);
	}
	return 0;

	case WM_DESTROY:
	{
		// Высвобождение памяти
		if (hBitmap != NULL)
		{
			DeleteObject(hBitmap);
		}

		// Удаление таймера
		if (timerId != 0)
		{
			KillTimer(hwnd, timerId);
			timerId = 0;
		}

		PostQuitMessage(0);
	}
	return 0;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		// Создание контекста устройства и битмапа для двойной буферизации
		HDC hdcBuffer = CreateCompatibleDC(hdc);
		RECT rect;
		GetClientRect(hwnd, &rect);
		HBITMAP hbmBuffer = CreateCompatibleBitmap(hdc, rect.left + rect.right, rect.top + rect.bottom);
		SelectObject(hdcBuffer, hbmBuffer);

		// Рисование на втором буфере
		FillRect(hdcBuffer, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
		HDC hdcMem = CreateCompatibleDC(hdcBuffer);
		SelectObject(hdcMem, hBitmap);
		COLORREF transparentColor = RGB(0, 0, 0);
		TransparentBlt(hdcBuffer, imageLeft, imageTop, imageWidth, imageHeight, hdcMem, 0, 0, imageWidth, imageHeight, transparentColor);

		// Копирование содержимого второго буфера на экран
		BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top,
			ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top,
			hdcBuffer, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);

		// Высвобождение памяти
		DeleteDC(hdcMem);
		DeleteDC(hdcBuffer);
		DeleteObject(hbmBuffer);

		EndPaint(hwnd, &ps);
	}
	return 0;

	case WM_MOUSEWHEEL:
	{
		short delta = GET_WHEEL_DELTA_WPARAM(wParam);

		// Получение состояния клавиши Shift
		bool isShiftKeyDown = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;

		// Прокрутка вертикальная или горизонтальная в зависимости от состояния клавиши Shift
		if (isShiftKeyDown)
		{
			// Прокрутка по горизонтали
			if (delta > 0)
			{
				imageLeft -= imageSpeed;
			}
			else if (delta < 0)
			{
				imageLeft += imageSpeed;
			}
		}
		else
		{
			// Прокрутка по вертикали
			if (delta > 0)
			{
				imageTop -= imageSpeed;
			}
			else if (delta < 0)
			{
				imageTop += imageSpeed;
			}
		}

		// Обновление окна
		InvalidateRect(hwnd, NULL, FALSE);
	}
	return 0;

	case WM_TIMER:
	{
		// Вызов функции обновления позиции изображения
		UpdateImagePosition();
		// Проверка колизии
		RECT rect;
		GetClientRect(hwnd, &rect);
		if (imageLeft < rect.left)
		{
			imageLeft = rect.left + 10;
		}
		else if ((imageLeft + imageWidth) > (rect.left + rect.right))
		{
			imageLeft = rect.left + rect.right - imageWidth - 10;
		}
		else if (imageTop < rect.top)
		{
			imageTop = rect.top + 10;
		}
		else if ((imageTop + imageHeight) > (rect.top + rect.bottom))
		{
			imageTop = rect.top + rect.bottom - imageHeight - 10;
		}
		// Обновление окна
		InvalidateRect(hwnd, NULL, FALSE);
	}
	return 0;

	case WM_CLOSE:
	{
		DestroyWindow(hwnd);
	}
	return 0;

	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}