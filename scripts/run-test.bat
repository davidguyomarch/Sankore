@echo off
echo === Open-Sankore Test Runner ===
echo.

echo [1/6] Killing any running instance...
taskkill /F /IM Open-Sankore.exe >nul 2>&1
REM Wait and retry until the exe is no longer locked (handles slow shutdown).
set /a _tries=0
:killwait
timeout /t 2 /nobreak >nul
tasklist /FI "IMAGENAME eq Open-Sankore.exe" 2>nul | find /I "Open-Sankore.exe" >nul
if not errorlevel 1 (
    set /a _tries+=1
    taskkill /F /IM Open-Sankore.exe >nul 2>&1
    if %_tries% LSS 5 goto killwait
)

if not exist C:\Sankore mkdir C:\Sankore

echo [2/6] Removing the old binary so a failed copy can't be mistaken for success...
del /F /Q C:\Sankore\Open-Sankore.exe >nul 2>&1
if exist C:\Sankore\Open-Sankore.exe (
    echo ERROR: could not delete C:\Sankore\Open-Sankore.exe — it is still locked.
    echo        Close every Open-Sankore window and run this script again.
    pause
    exit /b 1
)

echo [3/6] Copying fresh files to C:\Sankore...
set "SRC=%~dp0"
set "SRC=%SRC:~0,-1%"
REM /IS /IT: recopy same/tweaked files. Do NOT hide file list so copy errors show.
REM /XF *.pdb: the 50+ MB debug-symbol file is useless for a functional test and
REM the VirtIO/9p network share rejects it (ERROR 223, file too large), which
REM made robocopy loop forever. Symbols are never needed to run the app.
robocopy "%SRC%" C:\Sankore /E /IS /IT /XF *.pdb /NJH /NJS /R:5 /W:3 >nul
REM robocopy exit codes >= 8 mean a real failure.
if %ERRORLEVEL% GEQ 8 (
    echo ERROR: robocopy reported a failure ^(exit %ERRORLEVEL%^). Retrying verbosely...
    robocopy "%SRC%" C:\Sankore /E /IS /IT /XF *.pdb /R:5 /W:3
)
if not exist C:\Sankore\Open-Sankore.exe (
    echo ERROR: Open-Sankore.exe was not copied. Aborting.
    pause
    exit /b 1
)

echo [4/6] Deployed binary timestamp ^(must match the build you expect^):
dir C:\Sankore\Open-Sankore.exe | findstr /C:"Open-Sankore.exe"

del C:\Sankore\startup.log >nul 2>&1

echo [5/6] Launching Open-Sankore.exe...
cd /d C:\Sankore
Open-Sankore.exe

echo.
echo [6/6] === VERSION INFO ===
echo.
if exist C:\Sankore\startup.log (
    findstr /C:"Build:" C:\Sankore\startup.log
) else (
    echo NO LOG - crash before static initialization
)
echo.
echo === FULL STARTUP LOG ===
echo.
if exist C:\Sankore\startup.log (
    type C:\Sankore\startup.log
) else (
    echo NO LOG FILE CREATED
)
echo.
echo === END ===
pause
