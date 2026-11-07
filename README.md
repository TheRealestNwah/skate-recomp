# skate-recomp

Research and tooling toward a native PC port of **skate.** (EA Black Box, 2007) via static recompilation of the Xbox 360 executable, with possible decompilation work later.

> **No game code or assets are distributed here.** Anyone using this project must supply their own legally dumped copy of the Xbox 360 game. Never commit XEX files, disc images, extracted assets, or anything derived from them.

## Status

Early research, working toward the first milestone: booting to the title screen. See [`roadmap.md`](roadmap.md) for the path to 1.0 and [`docs/research/`](docs/research/) for findings.

## Layout

- `docs/research/` — findings on the toolchain, the game, and the plan
- `roadmap.md` — milestones from first boot to 1.0
- `GLOSSARY.md` — the project's vocabulary
- `.github/workflows/` — CI (markdown lint and a game-file guard for now; build/tests once code lands)

## AI disclosure

This project is built with AI coding assistants: [Claude Code](https://claude.com/claude-code) (Anthropic) and [Codex](https://openai.com/codex/) (OpenAI). They write the code, tools and documentation. The maintainer ([@TheRealestNwah](https://github.com/TheRealestNwah)) decides what it should do, tests it, and makes the release decisions. Commits written with an AI assistant carry a `Co-Authored-By` trailer, so the git history shows which changes were AI-written.

## Support

Everything on my GitHub is free of charge and open source. If you find it useful and want to leave a tip or buy me a coffee, you can do that at [ko-fi.com/morrowheat23](https://ko-fi.com/morrowheat23). It's appreciated, never expected.

## License

MIT for this repository's own code and docs. See [LICENSE](LICENSE). The license does not cover the game, its files, or code generated from them, none of which are distributed here.
