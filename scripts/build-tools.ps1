[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$sdk = & (Join-Path $PSScriptRoot 'setup-sdk.ps1')
& (Join-Path $PSScriptRoot 'enter-toolchain.ps1')
& cmake -S (Join-Path $repoRoot tools) -B (Join-Path $repoRoot 'build/research-tools') -G Ninja `
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ "-DCMAKE_PREFIX_PATH=$sdk"
if ($LASTEXITCODE -ne 0) {
    throw "Research tools configuration failed (exit $LASTEXITCODE)."
}
& cmake --build (Join-Path $repoRoot 'build/research-tools')
if ($LASTEXITCODE -ne 0) {
    throw "Research tools build failed (exit $LASTEXITCODE)."
}
