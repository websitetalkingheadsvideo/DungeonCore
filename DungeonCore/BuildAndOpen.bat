@echo off
set UE="C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat"
set PROJECT="E:\Dungeon Core\DungeonCore\DungeonCore.uproject"

echo Building DungeonCoreEditor...
call %UE% DungeonCoreEditor Win64 Development -Project=%PROJECT% -WaitMutex
if errorlevel 1 (
    echo.
    echo BUILD FAILED. Fix errors above, then run this script again.
    pause
    exit /b 1
)

echo.
echo Build OK. Opening Unreal Editor...
start "" %PROJECT%
