[CmdletBinding()]
param(
    [switch]$Wait,
    [switch]$Keyboard
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$port = Join-Path $repoRoot 'build/port/skateport.exe'
if (-not (Test-Path -LiteralPath $port)) {
    throw 'Build the Port with scripts/build-port.ps1 first.'
}
$config = Get-Content -Raw -LiteralPath (Join-Path $repoRoot 'local.json') | ConvertFrom-Json
$game = (Get-Item -LiteralPath $config.skate.extracted).FullName
if (-not (Test-Path -LiteralPath (Join-Path $game 'default.xex'))) {
    throw 'Run scripts/extract-disc.ps1 before launching the Port.'
}

$boot = (New-Item -ItemType Directory -Path (Join-Path $repoRoot 'build/boot') -Force).FullName
$log = Join-Path $boot ('boot-' + (Get-Date -Format 'yyyyMMdd-HHmmss-fff') + '.log')
$portArguments = @(
    '--game_data_root', "`"$game`"",
    '--user_data_root', "`"$boot\user`"",
    '--update_data_root', "`"$boot\updates`"",
    '--cache_root', "`"$boot\cache`"",
    '--log_file', "`"$log`"",
    '--log_level', 'debug',
    '--log_flush_interval', '1'
)
if ($Keyboard) {
    $portArguments += @(
        '--mnk_mode=true',
        '--keybind_a=Semicolon,Space,W,Shift+W',
        '--keybind_x=L,S,Shift+S',
        '--keybind_lstick_left=A,Shift+A',
        '--keybind_lstick_right=D,Shift+D',
        '--keybind_lstick_up=T',
        '--keybind_lstick_down=G',
        '--keybind_dpad_up=Up',
        '--keybind_dpad_down=Down',
        '--keybind_dpad_left=Left',
        '--keybind_dpad_right=Right',
        '--keybind_rstick_up=Shift+Up',
        '--keybind_rstick_down=Shift+Down',
        '--keybind_rstick_left=Shift+Left',
        '--keybind_rstick_right=Shift+Right'
    )
}
Write-Host "Boot log is $log."
$process = Start-Process -FilePath $port -WorkingDirectory (Split-Path -Parent $port) `
    -ArgumentList $portArguments -PassThru -Wait:$Wait
if ($Wait -and $process.ExitCode -ne 0) {
    throw "The Port exited with code $($process.ExitCode). See $log."
}
Write-Host "Port process ID is $($process.Id)."
