Set-StrictMode -Version Latest

function Show-CommandStatus {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name
    )

    $command = Get-Command $Name -ErrorAction SilentlyContinue
    if ($null -eq $command) {
        Write-Host "[missing] $Name"
        return
    }

    Write-Host "[found]   $Name -> $($command.Source)"
}

function Show-PathStatus {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Label,

        [Parameter(Mandatory = $true)]
        [string]$Path
    )

    if (Test-Path $Path) {
        Write-Host "[found]   $Label -> $Path"
        return
    }

    Write-Host "[missing] $Label -> $Path"
}

Write-Host "FirstLLM environment check"
Write-Host "=========================="

Show-CommandStatus -Name "git"
Show-CommandStatus -Name "cmake"
Show-CommandStatus -Name "cl"
Show-CommandStatus -Name "ninja"
Show-CommandStatus -Name "nvcc"
Show-CommandStatus -Name "winget"

Write-Host ""
Write-Host "Common Windows install paths"
Write-Host "----------------------------"

Show-PathStatus -Label "Git" -Path "C:\Program Files\Git\cmd\git.exe"
Show-PathStatus -Label "CMake" -Path "C:\Program Files\CMake\bin\cmake.exe"
Show-PathStatus -Label "vswhere" -Path "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
Show-PathStatus -Label "CUDA root" -Path "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA"

Write-Host ""
Write-Host "If required tools are missing, read EnvironmentSetup.md."

