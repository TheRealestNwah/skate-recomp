# CLAUDE.md

Research and tooling toward a native PC port of **skate.** (Xbox 360, 2007) via static recompilation. See `README.md` for the pitch and `docs/research/` for findings.

## Status

Docs-only so far: no code, build or tests yet. Update this file when code lands.

## Layout

- `docs/research/` — numbered research notes (`01-initial-survey.md`, ...). Claims cite sources; reasoning is marked **[Inference]**.
- `.github/workflows/ci.yml` — CI: markdownlint on all `*.md`, plus a guard that fails if game files are tracked.
- `.markdownlint.json` — lint config (line length, inline HTML and first-line-heading rules off).

## Lint

CI runs `markdownlint-cli2` over `**/*.md`. To run it locally:

```bash
npx markdownlint-cli2 "**/*.md"
```

## Gotchas

- **Never commit game files or anything derived from them** (XEX, ISO, GOD, `.big`, `.arena`, extracted assets, dumps). `.gitignore` covers `/game/`, `/dump/`, `/extracted/` and `/private/`; keep local copies there. CI rejects tracked `.xex/.iso/.god/.xcp/.big/.arena` files.
- Planned toolchain is ReXGlue SDK (Clang only). The closest precedent is the Skate 3 recomp; see the survey doc.
