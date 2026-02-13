@echo off
:: Swive CLI - PATH ȯ�溯�� ��� ��ũ��Ʈ (������ ���� �ʿ�)
:: Usage: install_path.bat [Release|Debug]

setlocal enabledelayedexpansion

set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=Release

:: ���� ��ũ��Ʈ ��ġ �������� bin ��� ����
set SCRIPT_DIR=%~dp0
set BIN_DIR=%SCRIPT_DIR%..\bin\x64\%CONFIG%

:: ���� ��η� ��ȯ
pushd "%BIN_DIR%" 2>nul
if errorlevel 1 (
    echo Error: Directory not found: %BIN_DIR%
    echo Please build the project first.
    exit /b 1
)
set BIN_DIR=%CD%
popd

echo.
echo ========================================
echo  Swive CLI PATH Installer
echo ========================================
echo.
echo Configuration: %CONFIG%
echo Install path:  %BIN_DIR%
echo.

:: swive.exe ���� Ȯ��
if not exist "%BIN_DIR%\swive.exe" (
    echo Error: swive.exe not found in %BIN_DIR%
    echo Please build the swive project first.
    exit /b 1
)

:: ������ ���� Ȯ��
net session >nul 2>&1
if errorlevel 1 (
    echo This script requires administrator privileges.
    echo Right-click and select "Run as administrator"
    pause
    exit /b 1
)

:: ���� �ý��� PATH ��������
for /f "tokens=2*" %%a in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path 2^>nul') do set SYSPATH=%%b

:: �̹� ��ϵǾ� �ִ��� Ȯ��
echo !SYSPATH! | findstr /i /c:"%BIN_DIR%" >nul
if not errorlevel 1 (
    echo.
    echo PATH already contains: %BIN_DIR%
    echo No changes made.
    goto :verify
)

:: PATH�� �߰�
echo Adding to system PATH...
setx /M PATH "%SYSPATH%;%BIN_DIR%"

if errorlevel 1 (
    echo Error: Failed to update PATH
    exit /b 1
)

echo.
echo Successfully added to system PATH!
echo.
echo NOTE: Please restart your terminal or log out/in for changes to take effect.

:verify
echo.
echo ----------------------------------------
echo Verifying installation...
echo ----------------------------------------
"%BIN_DIR%\swive.exe" version

echo.
echo ========================================
echo  Installation complete!
echo ========================================
echo.
echo Usage examples:
echo   swive build MyProject.ssproject
echo   swive run bin/Debug/MyProject.ssasm
echo   swive exec MyProject.ssproject
echo.

pause
exit /b 0
