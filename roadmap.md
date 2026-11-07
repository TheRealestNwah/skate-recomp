# Roadmap

From nothing to **1.0**: full career playable start to finish on Windows, with a native renderer and PC options. Terms are defined in [GLOSSARY.md](GLOSSARY.md); background is in [docs/research/01-initial-survey.md](docs/research/01-initial-survey.md).

## M0 — Toolchain setup

- [ ] One script installs everything needed on Windows: LLVM/Clang, CMake, Ninja, Visual Studio Build Tools (Windows SDK) and an XISO extractor.
- [ ] A local, uncommitted config file points the build at the user's own disc image and an extraction folder outside the repo.

## M1 — Dump inspection ([#3](https://github.com/TheRealestNwah/skate-recomp/issues/3))

- [ ] Extract `default.xex` and the disc file tree from the user's ISO.
- [ ] Record the media ID, XEX format (compression, encryption), imports, sections, and an archive inventory in `docs/research/`.
- [ ] Decide whether a title update is needed.

## M2 — Engine diff against Skate 3 ([#4](https://github.com/TheRealestNwah/skate-recomp/issues/4))

- [ ] Compare imports, strings and middleware signatures between the skate. and Skate 3 XEXs.
- [ ] Write up what carries over (engine, archive formats, video codec, audio).

## M3 — Title screen boot ([#5](https://github.com/TheRealestNwah/skate-recomp/issues/5))

- [ ] ReXGlue codegen over the skate. XEX: function boundaries, jump tables, setjmp and save/restore helpers.
- [ ] Stub missing kernel and XAM imports.
- [ ] The Port reaches the title screen on Xenia's GPU backend, from a clean checkout.
- [ ] CI builds the repo's own code; nothing that needs Game files runs in CI.

## M4 — Playable and stable

- [ ] Menus, skater select, and free skate in San Vanelona.
- [ ] Career playable start to finish.
- [ ] XMA audio and its threading, streaming memory lifetimes, 120 Hz input for flick-it.
- [ ] Photo and Skate Reel features (render-target resolve/readback).
- [ ] Saves.

## M5 — Native renderer

- [ ] Replace Xenia's GPU backend with a native D3D12 (and later Vulkan) renderer.
- [ ] Port materials and shaders.

## M6 — PC options → 1.0

- [ ] Any resolution and aspect ratio.
- [ ] Higher or uncapped framerate where the game logic allows it.
- [ ] Keyboard and mouse.
- [ ] An installer that checks the user's game files are original.

## After 1.0

- Linux / Steam Deck.
- Decompilation, function by function.
