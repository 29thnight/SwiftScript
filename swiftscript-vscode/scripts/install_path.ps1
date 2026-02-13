# Swive CLI - PATH ȯ�溯�� ��� ��ũ��Ʈ (PowerShell)
# Usage: .\install_path.ps1 [-Config Release|Debug] [-User]

param(
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Release",
    
    [switch]$User  # ����� PATH���� �߰� (������ ���� ���ʿ�)
)

$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Swive CLI PATH Installer" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# ��ũ��Ʈ ��ġ ���� bin ���
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$BinDir = Join-Path (Split-Path -Parent $ScriptDir) "bin\x64\$Config"

# ��� ���� Ȯ��
if (-not (Test-Path $BinDir)) {
    Write-Host "Error: Directory not found: $BinDir" -ForegroundColor Red
    Write-Host "Please build the project first." -ForegroundColor Yellow
    exit 1
}

$BinDir = (Resolve-Path $BinDir).Path

Write-Host "Configuration: $Config"
Write-Host "Install path:  $BinDir"
Write-Host "Scope:         $(if ($User) { 'User' } else { 'System' })"
Write-Host ""

# swive.exe Ȯ��
$SwiveExe = Join-Path $BinDir "swive.exe"
if (-not (Test-Path $SwiveExe)) {
    Write-Host "Error: swive.exe not found in $BinDir" -ForegroundColor Red
    Write-Host "Please build the swive project first." -ForegroundColor Yellow
    exit 1
}

# PATH ���� ����
if ($User) {
    $Target = [System.EnvironmentVariableTarget]::User
    $TargetName = "User"
} else {
    $Target = [System.EnvironmentVariableTarget]::Machine
    $TargetName = "System"
    
    # ������ ���� Ȯ��
    $IsAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
    if (-not $IsAdmin) {
        Write-Host "Error: System-wide installation requires administrator privileges." -ForegroundColor Red
        Write-Host "Run PowerShell as Administrator, or use -User flag for user-only installation:" -ForegroundColor Yellow
        Write-Host "  .\install_path.ps1 -User" -ForegroundColor Gray
        exit 1
    }
}

# ���� PATH ��������
$CurrentPath = [Environment]::GetEnvironmentVariable("Path", $Target)

# �̹� ��ϵǾ� �ִ��� Ȯ��
if ($CurrentPath -split ";" | Where-Object { $_ -ieq $BinDir }) {
    Write-Host "PATH already contains: $BinDir" -ForegroundColor Yellow
    Write-Host "No changes made."
} else {
    # PATH�� �߰�
    Write-Host "Adding to $TargetName PATH..." -ForegroundColor Green
    $NewPath = $CurrentPath.TrimEnd(";") + ";" + $BinDir
    [Environment]::SetEnvironmentVariable("Path", $NewPath, $Target)
    
    # ���� ���ǿ��� ����
    $env:Path = $env:Path + ";" + $BinDir
    
    Write-Host "Successfully added to $TargetName PATH!" -ForegroundColor Green
}

Write-Host ""
Write-Host "----------------------------------------" -ForegroundColor Gray
Write-Host "Verifying installation..." -ForegroundColor Gray
Write-Host "----------------------------------------" -ForegroundColor Gray

& $SwiveExe version

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Installation complete!" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Usage examples:" -ForegroundColor White
Write-Host "  swive build MyProject.ssproject" -ForegroundColor Gray
Write-Host "  swive run bin/Debug/MyProject.ssasm" -ForegroundColor Gray
Write-Host "  swive exec MyProject.ssproject" -ForegroundColor Gray
Write-Host ""

if (-not $User) {
    Write-Host "NOTE: Restart your terminal for PATH changes to take effect." -ForegroundColor Yellow
}
