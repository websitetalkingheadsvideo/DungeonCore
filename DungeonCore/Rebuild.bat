@echo off
set UE="C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat"
set PROJECT="E:\Dungeon Core\DungeonCore\DungeonCore.uproject"

echo Close Unreal Editor and stop PIE before building.
echo.
call %UE% DungeonCoreEditor Win64 Development -Project=%PROJECT% -WaitMutex
if errorlevel 1 (
    echo BUILD FAILED — if Live Coding is active, close the editor and run this again.
    pause
    exit /b 1
)

if not exist "E:\Dungeon Core\DungeonCore\Binaries\Win64\UnrealEditor-DungeonCore.dll" (
    echo ERROR: UnrealEditor-DungeonCore.dll was not produced.
    pause
    exit /b 1
)

echo.
echo OK: UnrealEditor-DungeonCore.dll built.
pause
