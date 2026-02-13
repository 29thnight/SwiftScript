# Swive CLI - PATH ȯ�溯�� ���� ��ũ��Ʈ (PowerShell)
# Usage: .\uninstall_path.ps1 [-User]

param(
    [switch]$User
)

$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Swive CLI PATH Uninstaller" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# PATH ���� ����
if ($User) {
    $Target = [System.EnvironmentVariableTarget]::User
    $TargetName = "User"
} else {
    $Target = [System.EnvironmentVariableTarget]::Machine
    $TargetName = "System"
    
    $IsAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
    if (-not $IsAdmin) {
        Write-Host "Error: System-wide uninstallation requires administrator privileges." -ForegroundColor Red
        Write-Host "Run PowerShell as Administrator, or use -User flag:" -ForegroundColor Yellow
        Write-Host "  .\uninstall_path.ps1 -User" -ForegroundColor Gray
        exit 1
    }
}

# ���� PATH ��������
$CurrentPath = [Environment]::GetEnvironmentVariable("Path", $Target)
$PathParts = $CurrentPath -split ";" | Where-Object { $_ -ne "" }

# swive ���� ��� ã��
$SwivePaths = $PathParts | Where-Object { $_ -match "Swive" -or $_ -match "swive" }

if ($SwivePaths.Count -eq 0) {
    Write-Host "No Swive paths found in $TargetName PATH." -ForegroundColor Yellow
    exit 0
}

Write-Host "Found Swive paths in $TargetName PATH:" -ForegroundColor Yellow
$SwivePaths | ForEach-Object { Write-Host "  - $_" -ForegroundColor Gray }
Write-Host ""

$Confirm = Read-Host "Remove these paths? (y/N)"
if ($Confirm -ne "y" -and $Confirm -ne "Y") {
    Write-Host "Cancelled." -ForegroundColor Yellow
    exit 0
}

# PATH���� ����
$NewPathParts = $PathParts | Where-Object { $_ -notmatch "Swive" -and $_ -notmatch "swive" }
$NewPath = $NewPathParts -join ";"

[Environment]::SetEnvironmentVariable("Path", $NewPath, $Target)

Write-Host ""
Write-Host "Successfully removed from $TargetName PATH!" -ForegroundColor Green
Write-Host "Restart your terminal for changes to take effect." -ForegroundColor Yellow
