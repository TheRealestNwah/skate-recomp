[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$winget = (Get-Command winget.exe -CommandType Application).Source

function Install-WinGetPackage {
    param(
        [Parameter(Mandatory)][string]$Id,
        [string[]]$InstallerArguments = @()
    )

    & $winget install --id $Id --exact --source winget --architecture x64 --no-upgrade `
        --accept-source-agreements --accept-package-agreements @InstallerArguments
    # UPDATE_NOT_APPLICABLE and PACKAGE_ALREADY_INSTALLED both mean no work is needed.
    if ($LASTEXITCODE -notin @(0, 0x8A15002B, 0x8A150061)) {
        throw "WinGet failed to install $Id (exit $LASTEXITCODE)."
    }
}

Install-WinGetPackage LLVM.LLVM
Install-WinGetPackage Kitware.CMake
Install-WinGetPackage Ninja-build.Ninja

$buildToolsId = 'Microsoft.VisualStudio.2022.BuildTools'
$cppWorkload = 'Microsoft.VisualStudio.Workload.VCTools'
$cppToolset = 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64'
$windowsSdk = 'Microsoft.VisualStudio.Component.Windows11SDK.26100'
$vsInstaller = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer'
$vswhere = Join-Path $vsInstaller 'vswhere.exe'
$instanceQuery = @('-products', '*', '-version', '[17.0,)', '-latest', '-property', 'installationPath')
$requiredComponents = @('-requires', $cppWorkload, $cppToolset, $windowsSdk)
$vsArguments = "--passive --norestart --add $cppWorkload --add $cppToolset --add $windowsSdk"

$readyInstance = if (Test-Path -LiteralPath $vswhere) { & $vswhere @instanceQuery @requiredComponents }
if (-not $readyInstance) {
    $instance = if (Test-Path -LiteralPath $vswhere) {
        & $vswhere -products Microsoft.VisualStudio.Product.BuildTools -version '[17.0,18.0)' -latest -property installationPath
    }
    if ($instance) {
        $installer = Join-Path $vsInstaller 'setup.exe'
        $process = Start-Process -FilePath $installer -WindowStyle Hidden -Wait -PassThru `
            -ArgumentList "modify --installPath `"$instance`" $vsArguments"
        if ($process.ExitCode -ne 0) {
            throw "Visual Studio Installer exited with $($process.ExitCode)."
        }
    } else {
        Install-WinGetPackage $buildToolsId -InstallerArguments @('--override', "$vsArguments --wait")
    }
}
$readyInstance = if (Test-Path -LiteralPath $vswhere) { & $vswhere @instanceQuery @requiredComponents }
if (-not $readyInstance) {
    throw 'Visual Studio with the C++ toolset and Windows SDK 26100 could not be verified.'
}
Write-Host "Using Visual Studio C++ tools at $readyInstance."

$extractorTag = 'build-202609111233'
$extractorDir = Join-Path $repoRoot "build/tools/extract-xiso/$extractorTag"
$extractorZip = Join-Path $extractorDir 'extract-xiso-Win64_Release.zip'
$extractorHash = 'fc891dfa72f08e3838a2f8b7ec34a8e4d61678da1cc8fee879677f00bf114ce3'
$extractorUrl = "https://github.com/XboxDev/extract-xiso/releases/download/$extractorTag/extract-xiso-Win64_Release.zip"

New-Item -ItemType Directory -Path $extractorDir -Force | Out-Null
if (-not (Test-Path -LiteralPath $extractorZip) -or
    (Get-FileHash -LiteralPath $extractorZip -Algorithm SHA256).Hash -ne $extractorHash) {
    Invoke-WebRequest -UseBasicParsing -Uri $extractorUrl -OutFile $extractorZip
}
if ((Get-FileHash -LiteralPath $extractorZip -Algorithm SHA256).Hash -ne $extractorHash) {
    throw 'The extract-xiso download did not match the expected SHA-256.'
}
Expand-Archive -LiteralPath $extractorZip -DestinationPath $extractorDir -Force
$extractor = Get-ChildItem -LiteralPath $extractorDir -Recurse -File -Filter 'extract-xiso.exe'
if (@($extractor).Count -ne 1) {
    throw "Expected one extract-xiso.exe under $extractorDir."
}
& $extractor.FullName -v
if ($LASTEXITCODE -ne 0) {
    throw "extract-xiso could not run (exit $LASTEXITCODE)."
}

$localConfig = Join-Path $repoRoot 'local.json'
if (-not (Test-Path -LiteralPath $localConfig)) {
    Copy-Item -LiteralPath (Join-Path $repoRoot 'local.json.example') -Destination $localConfig
    Write-Host "Fill in the disc and extraction paths in $localConfig."
}
Write-Host "extract-xiso is installed at $($extractor.FullName)."
Write-Host 'Open a new terminal and follow the PATH verification steps in CLAUDE.md.'
