@echo off
echo === Open-Sankore Test Runner ===
echo.
echo [1/6] Removing old C:\Sankore and copying fresh...
rmdir /S /Q C:\Sankore >nul 2>&1
mkdir C:\Sankore
xcopy "%~dp0*" C:\Sankore\ /E /Y /Q >nul 2>&1
echo [2/6] Cleaning old log...
del C:\Sankore\startup.log >nul 2>&1
echo [3/6] Launching Open-Sankore.exe...
cd /d C:\Sankore
Open-Sankore.exe
echo.
echo [4/6] === VERSION INFO ===
echo.
if exist C:\Sankore\startup.log (
    findstr /C:"Build:" C:\Sankore\startup.log
) else (
    echo NO LOG - crash before static initialization
)
echo.
echo [5/6] === FULL STARTUP LOG ===
echo.
if exist C:\Sankore\startup.log (
    type C:\Sankore\startup.log
) else (
    echo NO LOG FILE CREATED
)
echo.
echo [6/6] === LAST CRASH EVENT ===
echo.
powershell -command "Get-WinEvent -FilterHashtable @{LogName='Application';ProviderName='Application Error';Level=2} -MaxEvents 1 | Select-Object TimeCreated, Message | Format-List"
echo.
echo === END ===
pause
