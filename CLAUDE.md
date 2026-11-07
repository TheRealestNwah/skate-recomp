# CLAUDE.md

Research and tooling toward a native PC port of **skate.** (Xbox 360, 2007) via static recompilation. Codex reads this file too (through `AGENTS.md`), so it holds the rules for every agent.

- `README.md` — the pitch.
- `roadmap.md` — milestones to 1.0. Keep it current as work lands.
- `GLOSSARY.md` — the project's vocabulary. Use its terms.
- `docs/research/` — findings.

## Status

Windows toolchain setup is available. The Port build and tests are tracked in M3.

## Layout

- `docs/research/` — numbered research notes (`01-initial-survey.md`, ...). Claims cite sources; reasoning is marked **[Inference]**.
- `scripts/setup.ps1` — installs the Windows toolchain.
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

## Lint

```bash
npx markdownlint-cli2 "**/*.md"
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
