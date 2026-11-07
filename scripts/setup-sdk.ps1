[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$archive = Join-Path $repoRoot 'build/rexglue-sdk-0.10.1-win-amd64.zip'
$destination = Join-Path $repoRoot 'build/tools/rexglue-0.10.1'
$sdk = Join-Path $destination 'win-amd64'
$sha256 = '26dbcd19e6976a7ad349917bffc58c1a0f21ad43d71a963d16b623c8e70d1f4d'
New-Item -ItemType Directory -Path (Split-Path -Parent $archive) -Force | Out-Null
if (-not (Test-Path -LiteralPath $archive)) {
    Invoke-WebRequest -UseBasicParsing -OutFile $archive -Uri `
        'https://github.com/rexglue/rexglue-sdk/releases/download/v0.10.1/rexglue-sdk-0.10.1-win-amd64.zip'
}
if ((Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash -ne $sha256) {
    throw "ReXGlue SDK hash mismatch. Inspect $archive before retrying."
}
if (-not (Test-Path -LiteralPath (Join-Path $sdk 'bin/rexglue.exe'))) {
    Expand-Archive -LiteralPath $archive -DestinationPath $destination -Force
}
$license = Join-Path $sdk 'licenses/ReXGlue-LICENSE.txt'
if (-not (Test-Path -LiteralPath $license)) {
    Invoke-WebRequest -UseBasicParsing -OutFile $license -Uri `
        'https://raw.githubusercontent.com/rexglue/rexglue-sdk/v0.10.1/LICENSE'
}
if ((Get-FileHash -LiteralPath $license -Algorithm SHA256).Hash -ne
    '8e065be1da2ff9a16b1f063d4636d8b67e6a654bb90583ea4332e66ac421bb18') {
    throw 'The upstream ReXGlue license download failed verification.'
}
Write-Output $sdk
