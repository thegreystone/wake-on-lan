#Requires -Version 5.1
<#
.SYNOPSIS
    Installs a pre-built wol release binary.
.PARAMETER Version
    Release tag to install, e.g. v1.0.0. Defaults to the latest release.
.PARAMETER Dir
    Directory to install into. Defaults to %LOCALAPPDATA%\Programs\wol.
.EXAMPLE
    .\install.ps1
.EXAMPLE
    .\install.ps1 -Version v1.0.0 -Dir C:\Tools
#>
param(
    [string]$Version = "",
    [string]$Dir     = "$env:LOCALAPPDATA\Programs\wol"
)

$ErrorActionPreference = "Stop"

$Repo   = "thegreystone/wake-on-lan"
$Target = "wol.exe"

$Arch = $env:PROCESSOR_ARCHITEW6432
if (-not $Arch) { $Arch = $env:PROCESSOR_ARCHITECTURE }
$Suffix = switch ($Arch) {
    "AMD64" { "windows-x86_64" }
    "ARM64" { "windows-arm64"  }
    default {
        Write-Error "No pre-built binary for Windows/$Arch — please build from source: build.bat"
        exit 1
    }
}

if (-not $Version) {
    Write-Host "Fetching latest release..."
    $Release = Invoke-RestMethod "https://api.github.com/repos/$Repo/releases/latest"
    $Version = $Release.tag_name
}

$Binary = "wol-$Version-$Suffix.exe"
$Url    = "https://github.com/$Repo/releases/download/$Version/$Binary"

Write-Host "Downloading $Binary..."
$TmpFile = [System.IO.Path]::Combine([System.IO.Path]::GetTempPath(), $Binary)
Invoke-WebRequest -Uri $Url -OutFile $TmpFile

if (-not (Test-Path $Dir)) {
    New-Item -ItemType Directory -Path $Dir | Out-Null
}
Copy-Item $TmpFile "$Dir\$Target" -Force
Remove-Item $TmpFile

Write-Host "Installed $Target $Version to $Dir\$Target"

$UserPath = [Environment]::GetEnvironmentVariable("PATH", "User")
if ($UserPath -notlike "*$Dir*") {
    [Environment]::SetEnvironmentVariable("PATH", "$UserPath;$Dir", "User")
    Write-Host "Added $Dir to user PATH — restart your shell to apply."
}
