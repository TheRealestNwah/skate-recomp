[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$sdk = & (Join-Path $PSScriptRoot 'setup-sdk.ps1')
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$visualStudio = & $vswhere -latest -products '*' -version '[17.0,)' `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 Microsoft.VisualStudio.Component.Windows11SDK.26100 `
    -property installationPath
if (-not $visualStudio) {
    throw 'Run scripts/setup.ps1 to install the C++ toolset and Windows SDK.'
}
& (Join-Path $visualStudio 'Common7/Tools/Launch-VsDevShell.ps1') `
    -Arch amd64 -HostArch amd64 -SkipAutomaticLocation -NoLogo
$env:Path = "$env:ProgramFiles\LLVM\bin;$env:ProgramFiles\CMake\bin;$env:LOCALAPPDATA\Microsoft\WinGet\Links;$env:Path"
& cmake -S (Join-Path $repoRoot tools) -B (Join-Path $repoRoot 'build/research-tools') -G Ninja `
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ "-DCMAKE_PREFIX_PATH=$sdk"
if ($LASTEXITCODE -ne 0) {
    throw "Research tools configuration failed (exit $LASTEXITCODE)."
}
& cmake --build (Join-Path $repoRoot 'build/research-tools')
if ($LASTEXITCODE -ne 0) {
    throw "Research tools build failed (exit $LASTEXITCODE)."
}
