#pragma once

#include "windows.h"
#include <vector>
#include <iostream>
#include <thread> // Для использования задержки
#include <chrono> // Для работы с временем
#include <future> // Для std::async
using namespace std;

// Вектор для хранения найденных окон
std::vector<HWND> foundWindows;
HWND desktopIconsGlobal;
float timee = 0;
RECT rect;

int timeToHide;
int hideSpeedMs;


bool HasLayeredStyle(HWND hwnd) {
    // Получаем расширенные стили окна
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

    // Проверяем наличие стиля WS_EX_LAYERED
    if (exStyle & WS_EX_LAYERED) {
        return true;  // Окно имеет стиль WS_EX_LAYERED
    }
    else {
        return false; // Окно не имеет стиль WS_EX_LAYERED
    }
}

BYTE GetTransp() {
    COLORREF keyColor;
    BYTE alpha = 210;
    BOOL result = false;
    if (HasLayeredStyle(desktopIconsGlobal)) {
        result = GetLayeredWindowAttributes(desktopIconsGlobal, &keyColor, &alpha, nullptr);
    }
    else {
        alpha = 255;
    }
    return alpha;
}

void SetWindowTransparency(HWND hwnd, BYTE alpha) {
    // Установить стиль WS_EX_LAYERED, если он еще не установлен
    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, style | WS_EX_LAYERED);

    // Установить прозрачность окна
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), alpha, LWA_ALPHA | LWA_COLORKEY);
}

void SetRussianLang() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

}
void InitRect() {
    if (!GetWindowRect(desktopIconsGlobal, &rect)) cout << "Не получен IS Rect иконок" << endl;
}
bool IconsIsOpen() {
    //InitRect();

    BYTE transp = GetTransp();
    if (transp == 255 || transp == 204) return true;
    //int height = rect.bottom - rect.top;
    //cout << height << endl;
    //if (height == 1080) return true;
    return false;
}

bool CheckSizeIsCorrect() {
    //InitRect();
    BYTE transp = GetTransp();
    if (transp != 0 && (transp != 255 || transp == 204)) return false;
    //int height = rect.bottom - rect.top;
    //if (height != 0 && height != 1080) return false;
    return true;
}

void smoothTransitionHeight(HWND hWnd, int width, int targetHeight, int durationMs) {
    // Получаем текущее положение и размер окна
    RECT rect;
    GetWindowRect(hWnd, &rect);
    int currentHeight = rect.bottom - rect.top;

    // Разница в высоте
    int heightDifference = targetHeight - currentHeight;

    // Высокоточный таймер
    const int refreshRate = 75; // частота обновления в Гц
    const float frameDuration = 1000.0f / refreshRate; // длительность одного кадра в мс

    // Количество шагов для анимации
    int steps = durationMs / frameDuration;
    if (steps <= 0) steps = 1;

    // Инкремент высоты на каждом шаге
    float increment = static_cast<float>(heightDifference) / steps;

    auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < steps; ++i) {
        // Точное текущее время
        auto currentTime = std::chrono::high_resolution_clock::now();
        float elapsedMs = std::chrono::duration<float, std::milli>(currentTime - startTime).count();

        // Обновляем текущую высоту
        int newHeight = currentHeight + static_cast<int>(increment * i);

        // Устанавливаем новое положение и размер окна
        SetWindowPos(hWnd, nullptr, 0, 0, width, newHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_NOSENDCHANGING | SWP_NOACTIVATE);

        // Рассчитываем точное время для следующего кадра
        float sleepTime = (frameDuration * (i + 1)) - elapsedMs;
        if (sleepTime > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sleepTime)));
        }
    }

    // Убедимся, что конечная высота точно установлена
    SetWindowPos(hWnd, nullptr, 0, 0, width, targetHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_NOSENDCHANGING | SWP_NOACTIVATE);
}
bool hasLayeredStyle(HWND hWnd) {
    LONG_PTR exStyle = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
    if (exStyle == 0) {
        DWORD error = GetLastError();
        std::cerr << "Error getting window styles. Error code: " << error << std::endl;
        return false; // Не удалось получить стиль
    }

    // Проверяем наличие стиля WS_EX_LAYERED
    return (exStyle & WS_EX_LAYERED) != 0;
}
void smoothTransitionTransparency(HWND hWnd, BYTE targetAlpha, int durationMs) {
    // Получаем текущую прозрачность окна
    //LONG_PTR style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    LONG_PTR style = 0;

    if (targetAlpha == 0) {
        SetWindowLongPtr(hWnd, GWL_EXSTYLE, style | WS_EX_LAYERED | WS_EX_TRANSPARENT);
    }
    else {
        SetWindowLongPtr(hWnd, GWL_EXSTYLE, style);
        SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 255, LWA_ALPHA | LWA_COLORKEY);
        return;
    }
    // Высокоточный таймер
    LARGE_INTEGER frequency, startTime;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&startTime);

    // Определяем количество шагов для анимации
    const int refreshRate = 60; // частота обновления в FPS
    const float frameDuration = 1000.0f / refreshRate; // длительность одного кадра в мс

    // Определяем количество шагов для анимации
    int steps = durationMs / static_cast<int>(frameDuration);
    if (steps <= 0) steps = 1;

    // Получаем текущую прозрачность окна
    BYTE currentAlpha = 255 - targetAlpha;

    // Разница в прозрачности
    int alphaDifference = targetAlpha - currentAlpha;

    // Инкремент альфа-канала на каждом шаге
    float increment = static_cast<float>(alphaDifference) / steps;

    // Начинаем анимацию
    for (int i = 0; i < steps; ++i) {
        // Текущее время
        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);
//
        // Расчёт прошедшего времени
        float elapsedMs = (currentTime.QuadPart - startTime.QuadPart) * 1000.0 / frequency.QuadPart;

        // Обновляем текущую прозрачность
        BYTE newAlpha = static_cast<BYTE>(currentAlpha + increment * i);
        SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), newAlpha, LWA_ALPHA | LWA_COLORKEY);

        // Подождите, пока не пройдет необходимое время
        while (elapsedMs < frameDuration * (i + 1)) {
            // Обновляем текущее время
            QueryPerformanceCounter(&currentTime);
            elapsedMs = (currentTime.QuadPart - startTime.QuadPart) * 1000.0 / frequency.QuadPart;
        }
    }

    // Убедимся, что конечная прозрачность точно установлена
    SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), targetAlpha, LWA_ALPHA | LWA_COLORKEY);
}
bool IsFullScreenWindow() {
    // Получаем дескриптор активного окна
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return false;

    // Получаем размеры активного окна
    RECT windowRect;
    GetWindowRect(hwnd, &windowRect);

    // Получаем размеры экрана
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Проверяем, совпадают ли размеры окна с размерами экрана
    if (windowRect.left == 0 && windowRect.top == 0 &&
        windowRect.right == screenWidth && windowRect.bottom == screenHeight) {
        return true; // Окно в полноэкранном режиме
    }
    return false; // Окно не полноэкранное
}
void Hide() {
    //auto future1 = std::async(std::launch::async, smoothTransitionHeight, desktopIconsGlobal, 1920, 0, 300);
    auto future2 = std::async(std::launch::async, smoothTransitionTransparency, desktopIconsGlobal, 0, hideSpeedMs);
}

void Show() {
    //auto future1 = std::async(std::launch::async, smoothTransitionHeight, desktopIconsGlobal, 1920, 1080, 300);
    auto future2 = std::async(std::launch::async, smoothTransitionTransparency, desktopIconsGlobal, 255, hideSpeedMs);
}

std::wstring GetWindowTitle(HWND hwnd) {
    const int titleLength = 256; // Длина буфера для заголовка окна
    wchar_t title[titleLength]; // Буфер для заголовка

    // Получаем заголовок окна
    //if (GetWindowTextW(hwnd, title, titleLength) > 0) {
    //    return std::wstring(title); // Возвращаем заголовок как std::string
    //}
    if (GetClassNameW(hwnd, title, titleLength) > 0) {
        return std::wstring(title); // Возвращаем заголовок как std::string
    }
    else {
        return L"Не удалось получить заголовок."; // Если не удалось, возвращаем сообщение
    }
}
void checkInput() {
    while (1)
    {
        if (GetAsyncKeyState(VK_UP) != 0)
        {
            LONG_PTR style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

            SetWindowLongPtr(desktopIconsGlobal, GWL_EXSTYLE, style | WS_EX_LAYERED);
            // Обновляем параметры окна, не инициируя полную перерисовку
            SetWindowPos(desktopIconsGlobal, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        }
        if (GetAsyncKeyState(VK_DOWN) != 0)
        {
            LONG_PTR style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

            SetWindowLongPtr(desktopIconsGlobal, GWL_EXSTYLE, style);
            // Обновляем параметры окна, не инициируя полную перерисовку
            SetWindowPos(desktopIconsGlobal, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

            //SmoothResizeWindowAsync(desktopIconsGlobal, 0, 2000); // Изменение высоты до 500
            //smoothTransitionHeight(desktopIconsGlobal, 1920, 0, 150);
            //Hide();
        }
        if (GetAsyncKeyState(VK_LBUTTON) != 0)
        {
            POINT pt;
            GetCursorPos(&pt); // Получаем позицию курсора
            // Проверяем, находится ли курсор на рабочем столе
            HWND hDesktopWnd = GetDesktopWindow(); // Получаем дескриптор рабочего стола
            HWND hWnd = WindowFromPoint(pt); // Получаем дескриптор окна под курсором
            wstring title = GetWindowTitle(hWnd);
            // std::wcout << "AHAHA: " << GetWindowTitle(hWnd) << std::endl;
             // Сравниваем, является ли окно рабочим столом
            if (title == L"SHELLDLL_DefView" || title == L"SysListView32") {
                timee = 0;

                //std::wcout << "Левая кнопка мыши нажата на рабочем столе." << std::endl;
                if (!IconsIsOpen()) {
                    //std::cout << "Открыли" << std::endl;
                    Show();
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
void update() {
    while (true) {
        timee += 1;
        if (timee > timeToHide) {
            timee = 0;
            if (IconsIsOpen()) {
                Hide();
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

// Функция для перечисления дочерних окон
void FindChildWindows(HWND parent) {
    EnumChildWindows(parent, [](HWND hwnd, LPARAM lParam) -> BOOL {
        wchar_t className[256];
        GetClassNameW(hwnd, className, sizeof(className) / sizeof(className[0]));

        if (wcscmp(className, L"SysListView32") == 0) {
            foundWindows.push_back(hwnd);
        }

        return TRUE;  // Продолжаем перечисление
        }, NULL);
}
// Функция обратного вызова для перечисления окон
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    char className[256];
    char windowTitle[256];

    // Получаем название класса окна
    GetClassName(hwnd, className, sizeof(className) / sizeof(className[0]));

    // Получаем заголовок окна
    GetWindowText(hwnd, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));

    // Выводим класс и заголовок окна
    //std::wcout << L"HWND: " << hwnd << L", Class: " << className << L", Title: " << windowTitle << std::endl;

    return TRUE;  // Продолжаем перечисление
}