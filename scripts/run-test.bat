@echo off
echo === Open-Sankore Test Runner ===
echo.
echo [1/6] Killing any running instance...
taskkill /F /IM Open-Sankore.exe >nul 2>&1
timeout /t 2 /nobreak >nul
echo [2/6] Removing old C:\Sankore and copying fresh...
rmdir /S /Q C:\Sankore >nul 2>&1
if exist C:\Sankore (
    echo WARNING: Could not fully remove C:\Sankore, retrying...
    timeout /t 2 /nobreak >nul
    rmdir /S /Q C:\Sankore >nul 2>&1
)
mkdir C:\Sankore
robocopy "%~dp0" C:\Sankore /E /NFL /NDL /NJH /NJS >nul 2>&1
echo [3/6] Cleaning old log...
del C:\Sankore\startup.log >nul 2>&1
echo [4/6] Launching Open-Sankore.exe...
cd /d C:\Sankore
Open-Sankore.exe
echo.
echo [5/6] === VERSION INFO ===
echo.
if exist C:\Sankore\startup.log (
    findstr /C:"Build:" C:\Sankore\startup.log
) else (
    echo NO LOG - crash before static initialization
)
echo.
echo [6/6] === FULL STARTUP LOG ===
echo.
if exist C:\Sankore\startup.log (
    type C:\Sankore\startup.log
) else (
    echo NO LOG FILE CREATED
)
echo.
echo === END ===
pause
