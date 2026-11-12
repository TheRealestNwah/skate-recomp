[CmdletBinding()]
param(
    [switch]$HostOnly,
    [ValidateRange(1, 64)][int]$Jobs = 6,
    [switch]$FastCodegen
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$sdk = & (Join-Path $PSScriptRoot 'setup-sdk.ps1')
& (Join-Path $PSScriptRoot 'enter-toolchain.ps1')
$buildDirectory = Join-Path $repoRoot 'build/host'
$generated = ''

if (-not $HostOnly) {
    $buildDirectory = Join-Path $repoRoot 'build/port'
    $configPath = Join-Path $repoRoot 'local.json'
    $config = Get-Content -Raw -LiteralPath $configPath | ConvertFrom-Json
    if (-not [IO.Path]::IsPathRooted($config.skate.extracted)) {
        throw 'Set an absolute skate.extracted path in local.json.'
    }
    $gameRoot = [IO.Path]::GetFullPath($config.skate.extracted).TrimEnd('\', '/')
    if ($gameRoot -eq $repoRoot -or $gameRoot.StartsWith("$repoRoot\", [StringComparison]::OrdinalIgnoreCase)) {
        throw 'The extracted Game files must be outside the repository.'
    }
    $xex = Join-Path $gameRoot 'default.xex'
    if (-not (Test-Path -LiteralPath $xex)) {
        & (Join-Path $PSScriptRoot 'extract-disc.ps1') -Config $configPath
    }
    $expectedXex = 'fb1b41031ff5cf2385a16f37ef3af2f303f35fcc5fdda8dd2095ebdbefe4b095'
    if ((Get-FileHash -LiteralPath $xex -Algorithm SHA256).Hash -ne $expectedXex) {
        throw 'This configuration requires the original skate. USA disc XEX with media ID 7D3C6C51.'
    }

    $codegen = Join-Path $sdk 'bin/rexglue.exe'
    $codegenDirectory = Join-Path $repoRoot 'build/recomp'
    if ($FastCodegen) {
        $codegenSource = Join-Path $repoRoot 'tools/fast-codegen'
        $patchHash = (Get-FileHash -LiteralPath (Join-Path $codegenSource 'rexglue-gap-fill.patch')).Hash
        $sourceHash = (Get-FileHash -LiteralPath (Join-Path $codegenSource 'CMakeLists.txt')).Hash
        $codegenBuild = Join-Path $repoRoot "build/tools/fast-codegen-$($sourceHash.Substring(0, 8))-$($patchHash.Substring(0, 8))"
        & cmake -S $codegenSource -B $codegenBuild -G Ninja -DCMAKE_BUILD_TYPE=Release `
            -DCMAKE_CXX_COMPILER=clang++ "-DCMAKE_PREFIX_PATH=$sdk"
        if ($LASTEXITCODE -ne 0) {
            throw "Fast codegen configuration failed (exit $LASTEXITCODE)."
        }
        & cmake --build $codegenBuild --parallel $Jobs --target rexglue
        if ($LASTEXITCODE -ne 0) {
            throw "Fast codegen build failed (exit $LASTEXITCODE)."
        }
        $codegen = Join-Path $codegenBuild 'bin/rexglue.exe'
        $codegenDirectory = Join-Path $repoRoot 'build/recomp-fast'
    }
    New-Item -ItemType Directory -Path $codegenDirectory -Force | Out-Null
    $manifest = Join-Path $codegenDirectory 'skateport_manifest.toml'
    $gamePath = ConvertTo-Json -InputObject ($gameRoot.Replace('\', '/')) -Compress
    $xexPath = ConvertTo-Json -InputObject ($xex.Replace('\', '/')) -Compress
    $hints = ConvertTo-Json -InputObject ((Join-Path $repoRoot 'config/skate.toml').Replace('\', '/')) -Compress
    $manifestContents = @"
[project]
name = "skateport"
sdk_version = "0.10.1"
game_root = $gamePath

[entrypoint]
file_path = $xexPath
out_directory_path = "default"
includes = [$hints]
"@
    [IO.File]::WriteAllText($manifest, $manifestContents)
    & $codegen --log-file (Join-Path $codegenDirectory 'codegen.log') codegen $manifest
    if ($LASTEXITCODE -ne 0) {
        throw "ReXGlue codegen failed (exit $LASTEXITCODE)."
    }
    $generated = Join-Path $codegenDirectory 'default'
}

& cmake -S $repoRoot -B $buildDirectory -G Ninja -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_CXX_COMPILER=clang++ "-DCMAKE_PREFIX_PATH=$sdk" "-DSKATE_GENERATED_DIR=$generated"
if ($LASTEXITCODE -ne 0) {
    throw "CMake configuration failed (exit $LASTEXITCODE)."
}
& cmake --build $buildDirectory --parallel $Jobs
if ($LASTEXITCODE -ne 0) {
    throw "Build failed (exit $LASTEXITCODE)."
}
