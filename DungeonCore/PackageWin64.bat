@echo off
setlocal
set UE="C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat"
set UAT="C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat"
set PROJECT="E:\Dungeon Core\DungeonCore\DungeonCore.uproject"
set ARCHIVE="E:\Dungeon Core\DungeonCore\Packaged\Win64"

echo Close Unreal Editor before packaging.
echo Run ONLY: Packaged\Win64\DungeonCore\Binaries\Win64\DungeonCore.exe
echo (Do not launch a second copy - two windows means two processes.)
echo.
taskkill /IM DungeonCore.exe /F >nul 2>&1
if not errorlevel 1 (
    echo Stopped running DungeonCore.exe so staged DLLs can be replaced.
    timeout /t 2 /nobreak >nul
)
echo.
call %UE% DungeonCore Win64 Development -Project=%PROJECT% -WaitMutex
if errorlevel 1 (
    echo BUILD FAILED
    pause
    exit /b 1
)

if not exist "%ARCHIVE%" mkdir "%ARCHIVE%"

call %UAT% BuildCookRun ^
  -project=%PROJECT% ^
  -noP4 ^
  -platform=Win64 ^
  -clientconfig=Development ^
  -serverconfig=Development ^
  -cook ^
  -build ^
  -stage ^
  -pak ^
  -archive ^
  -archivedirectory=%ARCHIVE%

if errorlevel 1 (
    echo PACKAGING FAILED
    pause
    exit /b 1
)

echo.
echo OK: Win64 package archived to %ARCHIVE%
pause
