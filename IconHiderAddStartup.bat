@echo off
@chcp 65001 >nul
set "shortcutName=IconHider.lnk"
set "targetPath=%~dp0IconHider.exe"
set "shortcutPath=%USERPROFILE%\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup\%shortcutName%"

:: Проверка на существование ярлыка
if exist "%shortcutPath%" (
    echo Ярлык уже существует: %shortcutPath%
) else (
    :: Создание ярлыка через PowerShell
    powershell -Command "$WScriptShell = New-Object -ComObject WScript.Shell; $Shortcut = $WScriptShell.CreateShortcut('%shortcutPath%'); $Shortcut.TargetPath = '%targetPath%'; $Shortcut.WorkingDirectory = '%~dp0'; $Shortcut.Save()"

    echo Ярлык создан: %shortcutPath%
)

pause