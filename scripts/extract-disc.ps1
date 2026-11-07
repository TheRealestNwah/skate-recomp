[CmdletBinding()]
param(
    [ValidateSet('skate', 'skate3')][string]$Title = 'skate',
    [string]$Config
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot))
if (-not $Config) {
    $Config = Join-Path $repoRoot 'local.json'
}
$settings = Get-Content -Raw -LiteralPath $Config | ConvertFrom-Json
$disc = $settings.$Title
if (-not $disc.iso -or -not $disc.extracted) {
    throw "Set $Title.iso and $Title.extracted in $Config."
}
if (-not [IO.Path]::IsPathRooted($disc.iso) -or -not [IO.Path]::IsPathRooted($disc.extracted)) {
    throw 'Disc and extraction paths must be absolute.'
}
$iso = (Get-Item -LiteralPath $disc.iso).FullName
$destination = [IO.Path]::GetFullPath($disc.extracted).TrimEnd('\', '/')
$parent = Split-Path -Parent $destination
if ($parent -ne (Split-Path -Parent $iso)) {
    throw 'The extraction folder must be directly next to its ISO.'
}
if ($destination -eq $repoRoot -or $destination.StartsWith("$repoRoot\", [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Game files must be extracted outside the repository.'
}
if (Test-Path -LiteralPath $destination) {
    throw "Destination already exists. Choose a new extraction folder to avoid overwriting it: $destination"
}
$staging = "$destination.partial"
if (Test-Path -LiteralPath $staging) {
    throw "An incomplete extraction exists at $staging. Inspect it before retrying."
}
$extractorDir = Join-Path $repoRoot 'build/tools/extract-xiso/build-202609111233'
$extractor = @(Get-ChildItem -LiteralPath $extractorDir -Recurse -File -Filter 'extract-xiso.exe')
if ($extractor.Count -ne 1) {
    throw 'Run scripts/setup.ps1 first.'
}

# Options must precede the ISO. Run outside the repo even if the extractor ignores a destination.
Push-Location -LiteralPath $parent
try {
    & $extractor[0].FullName -x -d $staging $iso
    if ($LASTEXITCODE -ne 0) {
        throw "Extraction failed (exit $LASTEXITCODE). Incomplete files remain at $staging."
    }
} finally {
    Pop-Location
}
if (-not (Test-Path -LiteralPath (Join-Path $staging 'default.xex'))) {
    throw "No default.xex found in $staging. The destination was not promoted."
}
Move-Item -LiteralPath $staging -Destination $destination
Write-Host "Extracted $Title to $destination."
