# Codex goal: M0–M3 (title screen boot)

The prompt below starts Codex on milestones M0 through M3 of [roadmap.md](../roadmap.md). Paste everything inside the fence.

```text
Goal: get skate. (Xbox 360, 2007) to Title screen boot on this Windows PC via ReXGlue static recompilation — milestones M0 through M3 in roadmap.md.

Read first: AGENTS.md → CLAUDE.md (rules), GLOSSARY.md (terms), roadmap.md (milestones), docs/research/01-initial-survey.md (background). CLAUDE.md's rules on Game files, licenses and stopping to ask override anything here.

Machine: Windows 11, Ryzen 7 5800X, Radeon RX 9070 XT. Python 3.13 and Git are installed. Clang, CMake, Ninja and Visual Studio are not. The user's disc image is D:\Emulation\Games\Xbox 360\skate\Skate. (USA).iso. A Skate 3 disc image for comparison is at D:\Emulation\Games\Xbox 360\Skate 3 (USA, Europe) (En,Fr,De,Es,It,Nl).iso. Extract into folders next to each ISO, never into the repo.

Work in order. Each milestone lands as its own PR (or a few small ones), auto-merged once CI passes:

M0 Toolchain: write a setup script (winget) that installs LLVM/Clang, CMake, Ninja, VS Build Tools + Windows SDK, and an XISO extraction tool. Add a gitignored local config (with a committed .example) that holds the ISO and extraction paths. Document setup in CLAUDE.md.

M1 Dump inspection (#3): extract the disc, then record media ID, XEX header info (compression/encryption, imports, sections), the disc file tree, and an archive inventory in docs/research/02-dump-inspection.md. Say whether a title update looks needed. Don't fetch one.

M2 Engine diff (#4): compare skate. and Skate 3 XEXs (imports, strings, middleware signatures, archive formats, video codec) and write docs/research/03-engine-diff.md.

Checkpoint: after M2, post a short report on issue #5 covering findings, risks, and your plan for M3. Then continue without waiting.

M3 Title screen boot (#5): set up the project on upstream ReXGlue SDK; run codegen against the skate. XEX, fixing function boundaries, jump tables, setjmp and save/restore helpers in config; stub missing kernel/XAM imports; run on Xenia's GPU backend. Extend CI to build the repo's own code (no Game files in CI). Log each bug or unexpected blocker as its own labeled issue.

Done when: from a clean checkout, the setup script + docs build the Port against the user's ISO, the Port window reaches the title screen, and a boot log lists every stubbed or unimplemented kernel call. Describe the result in the M3 PR; keep screenshots local. Update roadmap.md checkboxes and CLAUDE.md build/test commands as you go.

Stop and ask instead of proceeding if: you'd need to download any game file (including title updates); you'd need code from skate3recomp or another unlicensed repo; you'd need to change repo settings, push code to main, force-push, or tag/release; or you've been stuck on one blocker for about a day.
```
