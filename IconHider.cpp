#include "IconHider.h"
#include "IniParser.h"

std::string getDirectory(const std::string& fullPath) {
    // Находим последний символ '/', чтобы выделить директорию
    size_t pos = fullPath.find_last_of("/\\"); // Ищем как '/' так и '\'
    if (pos == std::string::npos) {
        return ""; // Если символ не найден, возвращаем пустую строку
    }
    return fullPath.substr(0, pos); // Возвращаем подстроку до найденного символа
}

void INIData() {
    // Instantiate an IniParser object
    IniParser parser;

    try {
        char path[MAX_PATH];
        GetModuleFileName(nullptr, path, MAX_PATH);
        std::cout << "Текущий путь к исполняемому файлу: " << getDirectory(path) << std::endl;

        parser.createIniFile(getDirectory(path));

        // Parse an existing INI file
        parser.parseFromFile("settings.ini");

        // Access and modify data
        timeToHide = std::stoi(parser.getValue("Section1", "timeToHide"));
        hideSpeedMs = std::stoi(parser.getValue("Section1", "hideSpeedMs"));

        cout << "timeToHide: " << timeToHide << endl;
        cout << "hideSpeedMs: " << hideSpeedMs << endl;

        // Save modified data back to the INI file
        parser.saveToFile("settings.ini");
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;
    }
}

int main() {
    // Получаем дескриптор окна консоли
    HWND hWnd = GetConsoleWindow();

#if defined _DEBUG
    ShowWindow(hWnd, SW_SHOW);
#else
    ShowWindow(hWnd, SW_HIDE);
#endif

    SetRussianLang();

    INIData();

if (timeToHide == 1488) cout << "Вау" << endl;

    // Запускаем перечисление окон
    EnumWindows(EnumWindowsProc, 0);
    HWND progman = FindWindow("Progman", NULL);
    if (progman) {
        // Ищем дочерние окна у Progman
        FindChildWindows(progman);

        // Если не нашли, продолжаем искать среди других окон
        HWND workerw = FindWindowEx(nullptr, nullptr, "WorkerW", nullptr);
        while (workerw) {
            FindChildWindows(workerw);
            workerw = FindWindowEx(nullptr, workerw, "WorkerW", nullptr);
        }
    }
    else {
        std::cout << "Progman is not found" << std::endl;
    }

    if (foundWindows.empty()) cout << "SysListView32 is not found!" << endl;

    HWND desktopIcons = foundWindows[0];
    desktopIconsGlobal = foundWindows[0];

    LONG_PTR currentStyle = GetWindowLongPtr(desktopIconsGlobal, GWL_EXSTYLE);
    InitRect();
    if (!CheckSizeIsCorrect()) Show();

    auto future1 = std::async(std::launch::async, checkInput);
    auto future2 = std::async(std::launch::async, update);
}