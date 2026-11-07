$ErrorActionPreference = 'Stop'
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
if (-not (Test-Path -LiteralPath $vswhere)) {
    throw 'Run scripts/setup.ps1 to install the C++ toolset and Windows SDK.'
}
$visualStudio = & $vswhere -latest -products '*' -version '[17.0,)' `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 Microsoft.VisualStudio.Component.Windows11SDK.26100 `
    -property installationPath
if (-not $visualStudio) {
    throw 'Run scripts/setup.ps1 to install the C++ toolset and Windows SDK.'
}
& (Join-Path $visualStudio 'Common7/Tools/Launch-VsDevShell.ps1') `
    -Arch amd64 -HostArch amd64 -SkipAutomaticLocation -NoLogo
$env:Path = "$env:ProgramFiles\LLVM\bin;$env:ProgramFiles\CMake\bin;$env:LOCALAPPDATA\Microsoft\WinGet\Links;$env:Path"
