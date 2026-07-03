Set-StrictMode -Version Latest

function Add-PathIfExists {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path
    )

    if (!(Test-Path $Path)) {
        return
    }

    $paths = $env:Path -split ';'
    if ($paths -contains $Path) {
        return
    }

    $env:Path = "$Path;$env:Path"
    Write-Host "Added to PATH: $Path"
}

function Import-VsDevEnvironment {
    $vswhere = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
    if (!(Test-Path $vswhere)) {
        return
    }

    $installationPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if ([string]::IsNullOrWhiteSpace($installationPath)) {
        return
    }

    $vsDevCmd = Join-Path $installationPath "Common7\Tools\VsDevCmd.bat"
    if (!(Test-Path $vsDevCmd)) {
        return
    }

    Write-Host "Loading MSVC environment from: $vsDevCmd"
    $environmentLines = cmd /c "`"$vsDevCmd`" -arch=x64 -host_arch=x64 >nul && set"
    foreach ($line in $environmentLines) {
        $index = $line.IndexOf('=')
        if ($index -le 0) {
            continue
        }

        $name = $line.Substring(0, $index)
        $value = $line.Substring($index + 1)
        Set-Item -Path "Env:$name" -Value $value
    }
}

Add-PathIfExists -Path "C:\Program Files\Git\cmd"
Add-PathIfExists -Path "C:\Program Files\CMake\bin"
Add-PathIfExists -Path "C:\Program Files\Ninja"
Add-PathIfExists -Path "$env:LOCALAPPDATA\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe"

$cudaRoot = "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA"
if (Test-Path $cudaRoot) {
    $cudaVersions = @(Get-ChildItem $cudaRoot -Directory | Sort-Object Name -Descending)
    if ($cudaVersions.Count -gt 0) {
        Add-PathIfExists -Path (Join-Path $cudaVersions[0].FullName "bin")
    }
}

Import-VsDevEnvironment

Write-Host "FirstLLM environment script finished."
Write-Host "Try: git --version; cmake --version; cl"
