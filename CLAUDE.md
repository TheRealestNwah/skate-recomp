# CLAUDE.md

Research and tooling toward a native PC port of **skate.** (Xbox 360, 2007) via static recompilation. Codex reads this file too (through `AGENTS.md`), so it holds the rules for every agent.

- `README.md` — the pitch.
- `roadmap.md` — milestones to 1.0. Keep it current as work lands.
- `GLOSSARY.md` — the project's vocabulary. Use its terms.
- `docs/research/` — findings.

## Status

The original skate. USA disc boots to the title screen from a clean Windows checkout using stock ReXGlue SDK 0.10.1. M4 playtesting has verified fresh profile creation, skater selection, and a Free Skate session. CI builds the Host and research tools without Game files.

## Layout

- `docs/research/` — numbered research notes (`01-initial-survey.md`, ...). Claims cite sources; reasoning is marked **[Inference]**.
- `scripts/setup.ps1` — installs the Windows toolchain.
- `scripts/extract-disc.ps1` — extracts a locally configured disc outside the checkout.
- `scripts/build-tools.ps1` — builds the headless XEX inspector against the pinned SDK.
- `scripts/build-port.ps1` — builds the Host and, with the disc configured, the Port.
- `scripts/run-port.ps1` — starts the Port and captures a local boot log.
- `src/` — Host app and boot import audit.
- `config/skate.toml` — ReXGlue analysis hints for the supported disc image.
- `tools/` — XEX inspection, disc inventory and parser tests.
- `local.json.example` — template for paths to your own disc images and extraction folders.
- `.github/workflows/ci.yml` — CI checks Markdown, PowerShell syntax and the local config template, plus a guard that fails if game files are tracked.
- `.markdownlint.json` — lint config (line length, inline HTML and first-line-heading rules off).

## Windows setup

Run from the repository root on x64 Windows 11 with WinGet, Git and Python 3.13 available. The installers may request administrator approval.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\setup.ps1
```

The script installs LLVM/Clang, CMake and Ninja via WinGet. It uses an existing Visual Studio 2022 or later instance with the C++ toolset and Windows SDK 26100 when available, and otherwise sets up Visual Studio 2022 Build Tools. It also installs the [pinned XboxDev extract-xiso release](https://github.com/XboxDev/extract-xiso/releases/tag/build-202609111233) under the ignored `build/tools/` directory and checks its SHA-256 before unpacking.

Edit the generated `local.json` with absolute paths. Set `skate.iso` to your disc image and `skate.extracted` to an extraction folder next to it, outside the checkout. The `skate3` paths are for comparison research only.

Open a new terminal after installing. The LLVM installer may leave its bin directory off PATH. Add it for the current PowerShell session if needed, then verify the tools.

```powershell
if (-not (Get-Command clang++ -ErrorAction SilentlyContinue)) {
    $env:Path = "$env:ProgramFiles\LLVM\bin;$env:Path"
}
clang++ --version
cmake --version
ninja --version
```

The setup script verifies the Visual Studio components and runs `extract-xiso -v`.

The extractor's license is saved alongside its executable. This product includes software developed by in <in@fishtank.com>.

## Local disc inspection

From the repository root, after filling in `local.json`:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\extract-disc.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build-tools.ps1
$config = Get-Content -Raw local.json | ConvertFrom-Json
.\build\research-tools\inspect_xex.exe (Join-Path $config.skate.extracted 'default.xex')
python -B tools/inspect_disc.py --output build/skate-disc.json
```

The build script installs [upstream ReXGlue SDK 0.10.1](https://github.com/rexglue/rexglue-sdk/releases/tag/v0.10.1) and preserves its BSD license with the installed SDK. The headless inspector saves its report and decoded image in `.inspection/` next to the extracted XEX. Keep that directory local.

For Skate 3, pass `-Title skate3` to the extraction script, point `inspect_xex.exe` at `(Join-Path $config.skate3.extracted 'default.xex')` and run the inventory with `--title skate3 --output build/skate3-disc.json`.

## Port build and boot

After Windows setup, fill in the skate. paths in `local.json`. From the repository root, run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build-port.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\run-port.ps1
```

The build extracts the disc when `default.xex` is missing, verifies the original USA XEX for media ID `7D3C6C51`, and writes Generated code under `build/recomp/`. The first codegen pass took about eight minutes on the Ryzen 7 5800X with the current hints. Gap-fill can take several minutes without console output. [Gap-fill investigation](https://github.com/TheRealestNwah/skate-recomp/issues/13)

The Port uses the SDK's Xenos D3D12 backend. Wait for the skate. logo and **Press START to continue**. The launch script prints the path to the local boot log under `build/boot/`. The verified run identified 63 SDK stubs and 33 fixed fallbacks across 350 imports, with zero unresolved imports. The log lists the gaps by library and ordinal and records stub calls made during startup. Keep logs and screenshots local.

Pass `-Keyboard` to `run-port.ps1` to enable the keyboard preset. W pushes with the right foot, S pushes with the left foot, and A and D steer. T and G provide left-stick up and down. Enter maps to START, Space maps to A for menu selection, and Backspace maps to B for braking and backing out of menus. The arrow keys control the D-pad. Hold Shift with an arrow key to move the right stick for Flick-It.

To compile the Host and inspection tools without a disc:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build-port.ps1 -HostOnly
```

## Lint and tests

```bash
npx markdownlint-cli2 "**/*.md" "#build/**"
python -B -m unittest discover -s tools -p 'test_*.py'
```

## Game files (most important rule)

- **Never commit Game files or Generated code** (see `GLOSSARY.md`): disc images, XEX, `.big`/`.arena` archives, extracted assets, recompiled C++, screenshots or videos of the game. CI rejects common game-file extensions, but the rule is broader than the guard.
- The user's disc image is at `D:\Emulation\Games\Xbox 360\skate\Skate. (USA).iso`. Extract into `D:\Emulation\Games\Xbox 360\skate\extracted\`, outside the repo, and write any other generated output outside the repo or into gitignored `/build/`.
- The build finds these paths through a local, gitignored config file. Commit a `.example` version with placeholder paths.
- Never download game files: no title updates, dumps or other people's XEXs. If a title update turns out to be needed, stop and ask the user.

## Code sources and licenses

- Our code is MIT. Upstream ReXGlue SDK (BSD-style, derived from Xenia) and XenonRecomp (MIT) are fine to depend on, with their notices kept.
- **skate3recomp has no license.** Read it for reference, but never copy code from it. The same applies to any repo without a compatible license.

## Working rules

- Work on a branch, open a PR, then `gh pr merge <n> --auto --squash --delete-branch`. Docs-only changes may go straight to `main`. Never push code to `main` directly, force-push, push tags or cut releases.
- PR and commit titles are short and imperative. Put `Closes #N` in the PR body for each issue it resolves.
- Log each bug you find as its own labeled GitHub issue. Planned work is tracked in `roadmap.md`, not issues.
- One agent session per checkout. Parallel sessions each use their own git worktree.
- **Stop and ask the user before:** downloading anything game-related, copying code from an unlicensed repo, changing repo settings or visibility, any destructive git operation, or after roughly a day stuck on one blocker.

## Gotchas

- ReXGlue is Clang-only.
- Planned toolchain and the Skate 3 precedent are covered in `docs/research/01-initial-survey.md`.
