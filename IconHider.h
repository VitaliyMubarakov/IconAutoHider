#pragma once

#include "windows.h"
#include <vector>
#include <iostream>
#include <thread>
#include <chrono>
#include <future>
using namespace std;

std::vector<HWND> foundWindows;
HWND desktopIconsGlobal;
float timee = 0;
RECT rect;

int timeToHide;
int hideSpeedMs;


bool HasLayeredStyle(HWND hwnd) {
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

    if (exStyle & WS_EX_LAYERED) {
        return true; 
    }
    else {
        return false;
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
    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, style | WS_EX_LAYERED);

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
    RECT rect;
    GetWindowRect(hWnd, &rect);
    int currentHeight = rect.bottom - rect.top;

    int heightDifference = targetHeight - currentHeight;

    const int refreshRate = 75; // частота обновления в гц
    const float frameDuration = 1000.0f / refreshRate; // длительность одного кадра в мс

    int steps = durationMs / frameDuration;
    if (steps <= 0) steps = 1;

    float increment = static_cast<float>(heightDifference) / steps;

    auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < steps; ++i) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float elapsedMs = std::chrono::duration<float, std::milli>(currentTime - startTime).count();

        int newHeight = currentHeight + static_cast<int>(increment * i);

        SetWindowPos(hWnd, nullptr, 0, 0, width, newHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_NOSENDCHANGING | SWP_NOACTIVATE);

        float sleepTime = (frameDuration * (i + 1)) - elapsedMs;
        if (sleepTime > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sleepTime)));
        }
    }

    SetWindowPos(hWnd, nullptr, 0, 0, width, targetHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_NOSENDCHANGING | SWP_NOACTIVATE);
}
bool hasLayeredStyle(HWND hWnd) {
    LONG_PTR exStyle = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
    if (exStyle == 0) {
        DWORD error = GetLastError();
        std::cerr << "Error getting window styles. Error code: " << error << std::endl;
        return false;
    }

    return (exStyle & WS_EX_LAYERED) != 0;
}
void smoothTransitionTransparency(HWND hWnd, BYTE targetAlpha, int durationMs) {
    LONG_PTR style = 0;

    if (targetAlpha == 0) {
        SetWindowLongPtr(hWnd, GWL_EXSTYLE, style | WS_EX_LAYERED | WS_EX_TRANSPARENT);
    }
    else {
        SetWindowLongPtr(hWnd, GWL_EXSTYLE, style);
        SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 255, LWA_ALPHA | LWA_COLORKEY);
        return;
    }
    LARGE_INTEGER frequency, startTime;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&startTime);

    const int refreshRate = 60;
    const float frameDuration = 1000.0f / refreshRate;

    int steps = durationMs / static_cast<int>(frameDuration);
    if (steps <= 0) steps = 1;

    BYTE currentAlpha = 255 - targetAlpha;

    int alphaDifference = targetAlpha - currentAlpha;

    float increment = static_cast<float>(alphaDifference) / steps;

    for (int i = 0; i < steps; ++i) {
        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);
        float elapsedMs = (currentTime.QuadPart - startTime.QuadPart) * 1000.0 / frequency.QuadPart;

        BYTE newAlpha = static_cast<BYTE>(currentAlpha + increment * i);
        SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), newAlpha, LWA_ALPHA | LWA_COLORKEY);

        while (elapsedMs < frameDuration * (i + 1)) {
            QueryPerformanceCounter(&currentTime);
            elapsedMs = (currentTime.QuadPart - startTime.QuadPart) * 1000.0 / frequency.QuadPart;
        }
    }

    SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), targetAlpha, LWA_ALPHA | LWA_COLORKEY);
}
bool IsFullScreenWindow() {
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return false;

    RECT windowRect;
    GetWindowRect(hwnd, &windowRect);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    if (windowRect.left == 0 && windowRect.top == 0 &&
        windowRect.right == screenWidth && windowRect.bottom == screenHeight) {
        return true;
    }
    return false;
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
    const int titleLength = 256;
    wchar_t title[titleLength];

    if (GetClassNameW(hwnd, title, titleLength) > 0) {
        return std::wstring(title);
    }
    else {
        return L"Не удалось получить заголовок.";
    }
}
void checkInput() {
    while (1)
    {
        if (GetAsyncKeyState(VK_LBUTTON) != 0)
        {
            POINT pt;
            GetCursorPos(&pt);

            HWND hWnd = WindowFromPoint(pt);
            wstring title = GetWindowTitle(hWnd);

            if (title == L"SHELLDLL_DefView" || title == L"SysListView32") {
                timee = 0;
                if (!IconsIsOpen()) {
                    Show();
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(80));
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

void FindChildWindows(HWND parent) {
    EnumChildWindows(parent, [](HWND hwnd, LPARAM lParam) -> BOOL {
        wchar_t className[256];
        GetClassNameW(hwnd, className, sizeof(className) / sizeof(className[0]));

        if (wcscmp(className, L"SysListView32") == 0) {
            foundWindows.push_back(hwnd);
        }

        return TRUE;  // Продолжаем
        }, NULL);
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    char className[256];
    char windowTitle[256];

    GetClassName(hwnd, className, sizeof(className) / sizeof(className[0]));

    GetWindowText(hwnd, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));

    return TRUE;  // Продолжаем
}