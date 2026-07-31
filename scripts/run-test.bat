@echo off
echo === Open-Sankore Test Runner ===
echo.
echo [1/5] Killing any running instance...
taskkill /F /IM Open-Sankore.exe >nul 2>&1
timeout /t 3 /nobreak >nul
echo [2/5] Copying fresh files to C:\Sankore...
if not exist C:\Sankore mkdir C:\Sankore
set "SRC=%~dp0"
set "SRC=%SRC:~0,-1%"
robocopy "%SRC%" C:\Sankore /E /IS /IT /NFL /NDL /NJH /NJS /R:5 /W:3 >nul 2>&1
if not exist C:\Sankore\Open-Sankore.exe (
    echo ERROR: Copy failed, retrying without quiet mode...
    robocopy "%SRC%" C:\Sankore /E /IS /IT /R:5 /W:3
)
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
