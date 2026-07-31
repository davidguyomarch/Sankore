@echo off
echo === Open-Sankore Test Runner ===
echo.
echo [1/5] Killing any running instance...
taskkill /F /IM Open-Sankore.exe >nul 2>&1
timeout /t 3 /nobreak >nul
echo [2/5] Copying fresh files to C:\Sankore...
robocopy "%~dp0" C:\Sankore /MIR /NFL /NDL /NJH /NJS /R:3 /W:2 >nul 2>&1
del C:\Sankore\startup.log >nul 2>&1
echo [3/5] Launching Open-Sankore.exe...
cd /d C:\Sankore
Open-Sankore.exe
echo.
echo [4/5] === VERSION INFO ===
echo.
if exist C:\Sankore\startup.log (
    findstr /C:"Build:" C:\Sankore\startup.log
) else (
    echo NO LOG - crash before static initialization
)
echo.
echo [5/5] === FULL STARTUP LOG ===
echo.
if exist C:\Sankore\startup.log (
    type C:\Sankore\startup.log
) else (
    echo NO LOG FILE CREATED
)
echo.
echo === END ===
pause
