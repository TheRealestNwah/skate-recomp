# skate. Recompilation

A work-in-progress native Windows port of **skate.** (Xbox 360, 2007) by EA Black Box.

The build uses [ReXGlue](https://github.com/rexglue/rexglue-sdk) to translate the original Xbox 360 executable ahead of time into C++. The resulting Port runs on Windows with ReXGlue's runtime and Xenos D3D12 backend.

## Status

The original USA disc **boots to the title screen** from a clean Windows checkout. M4 playtesting has reached **Free Skate in San Vanelona** after creating a fresh profile. Gameplay stability and career progression are still in progress.

| Milestone | Status |
| --- | --- |
| M0 · Windows toolchain | Complete |
| M1 · Disc and executable inspection | Complete |
| M2 · Engine comparison with Skate 3 | Complete |
| M3 · Native title-screen boot | Complete |
| M4 · Playable and stable | In progress |

## You need your own copy of the game

Use your own Xbox 360 disc dump. The build currently supports the original USA disc with media ID `7D3C6C51`. Store the ISO and its extraction folder outside the checkout, next to each other. Keep disc images, executable files, assets, and Generated code local.

## Setup (Windows)

On x64 Windows 11 with WinGet, Git, and Python 3.13, run the setup script from the repository root.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\setup.ps1
```

Set `skate.iso` and `skate.extracted` in the generated `local.json` to the absolute paths of your ISO and extraction folder. Then build and launch the Port.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build-port.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\run-port.ps1
```

The build extracts the disc if needed and generates the guest code locally. The first build can take several minutes. See [CLAUDE.md](CLAUDE.md) for toolchain details, boot logs, and the CI checks.

## Roadmap

See the [roadmap](roadmap.md) for the path from title-screen boot to a playable career, a native renderer, and PC options. The [research notes](docs/research/) document the disc inspection and engine comparison.

## Layout

| Path | Purpose |
| --- | --- |
| [`config/`](config/) | ReXGlue analysis hints for the original USA disc |
| [`src/`](src/) | Windows Host app and boot import audit |
| [`scripts/`](scripts/) | Toolchain setup, disc extraction, build, and launch |
| [`tools/`](tools/) | Disc and executable inspection tools and tests |
| [`docs/research/`](docs/research/) | Findings about the game and toolchain |
| [`GLOSSARY.md`](GLOSSARY.md) | Project terminology |

## AI disclosure

This project is developed with [Claude Code](https://claude.com/claude-code) and [OpenAI Codex](https://openai.com/codex/). These assistants contribute code, tools, and documentation. The maintainer ([@TheRealestNwah](https://github.com/TheRealestNwah)) sets the direction, tests the project, and makes release decisions.

## Support

Everything on my GitHub is free of charge and open source. If you find it useful and want to leave a tip or buy me a coffee, you can do that at [ko-fi.com/morrowheat23](https://ko-fi.com/morrowheat23). It's appreciated, never expected.

## License

MIT for this repository's own code and docs. See [LICENSE](LICENSE). The license does not cover the game, its files, or code generated from them, none of which are distributed here.
