# Roadmap

From nothing to **1.0**: full career playable start to finish on Windows, with a native renderer and PC options. Terms are defined in [GLOSSARY.md](GLOSSARY.md); background is in [docs/research/01-initial-survey.md](docs/research/01-initial-survey.md).

## M0 — Toolchain setup

- [x] One script installs everything needed on Windows: LLVM/Clang, CMake, Ninja, Visual Studio Build Tools (Windows SDK) and an XISO extractor.
- [x] A local, uncommitted config file points the build at the user's own disc image and an extraction folder outside the repo.

## M1 — Dump inspection ([#3](https://github.com/TheRealestNwah/skate-recomp/issues/3))

- [x] Extract `default.xex` and the disc file tree from the user's ISO.
- [x] Record the media ID, XEX format (compression, encryption), imports, sections, and an archive inventory in [the dump inspection](docs/research/02-dump-inspection.md).
- [x] Decide whether a title update is needed. Begin with the disc build.

## M2 — Engine diff against Skate 3 ([#4](https://github.com/TheRealestNwah/skate-recomp/issues/4))

- [x] Compare imports, strings and middleware signatures between the skate. and Skate 3 XEXs.
- [x] Write up what carries over (engine, archive formats, video codec, audio) in [the engine comparison](docs/research/03-engine-diff.md).

## M3 — Title screen boot ([#5](https://github.com/TheRealestNwah/skate-recomp/issues/5))

- [x] ReXGlue codegen over the skate. XEX: function boundaries, jump tables, setjmp and save/restore helpers.
- [x] Resolve kernel and XAM imports and audit the upstream SDK stubs and fixed fallbacks.
- [x] The Port reaches the title screen on Xenia's GPU backend, from a clean checkout.
- [x] CI builds the Host and research tools without Game files.

## M4 — Playable and stable

This is the current focus. A [baseline playtest](docs/research/05-m4-baseline.md) checks each open item first. Defects become issues. Work then follows the order below unless the baseline changes it.

### M4 completion plan

1. **Hold the game at 60 Hz.** The launcher explicitly requests 60 Hz by default; the 30 Hz fallback remains opt-in for diagnosis. Higher refresh rates stay in M6.
2. **Verify Career performance at 60 Hz.** Use the RTV path at 1280x720. After warm-up, measure the same Career spawn and repeatable skating route. Ready means 55-60 Guest swaps/s while stationary and at least 50 swaps/s averaged over the movement route, with every five-second p95 interval at or below 25 ms and no recurring gaps over 100 ms across three independent runs. The November 23 experimental pacing candidate passed four default-WASAPI routes. Four matched routes using the current default `build/port` stock plugin produced only one all-threshold pass; the other three missed one or more limits. The formal default-build performance gate fails this control set. November 22 candidate lows and their session-to-session variation remain unexplained; audible playback and physical controller checks remain separate.
3. **Finish audio and controller checks.** Complete the audio streaming/lifetime and physical controller checks in the baseline. Verify 120 Hz Flick-It sampling separately at the capped 60 Hz display rate, including release behavior and pause/resume; keep it opt-in until the physical check passes. Compare action timing around a verified pause/resume on the physical controller.
4. **Run the career completion playthrough.** The maintainer continues one unmodified save to the credits and records each session in the Career log. Fix blockers as individually tracked issues.
5. **Finish Photo and Skate Reel.** Validate capture, preview, save, relaunch, and replay behavior, including render-target resolve/readback.

Four independent default-WASAPI routes on November 23 passed every threshold using the experimental pacing candidate. Across four matched stock-plugin routes, only one passed every threshold; the other three missed at least one limit. The formal default-build performance gate fails this control set. The November 22 candidate lows and larger session-to-session difference remain unexplained. The full manual Career playthrough still follows the separate audio and physical-controller checks, and gameplay may expose scene-specific drops.

Earlier testing found mixed results: the November 21 candidate averaged 58.1-59.6 swaps/s across six movement routes, with two runs meeting every threshold; later silent-audio, WASAPI, and DirectSound routes missed thresholds. The November 23 WASAPI candidate runs passed, but only one of four matched stock-plugin routes passed every threshold; the default-build performance gate fails. GPU, draw-cost, and CPU-stack profiles have not identified a single bottleneck or explained the session-to-session variation. See the [renderer performance research](docs/research/04-renderer-performance.md) for historical failures and current candidate/control measurements.

A separate [production Host startup timing check](docs/research/12-production-host-startup.md) did not reach a Career scene and does not count toward the Career performance gate.

- [x] Menus, skater select, and free skate in San Vanelona.
- [x] Saves. A new Career save survived quit and relaunch in the [unattended save check](docs/research/07-saves.md).
- [ ] XMA audio and its threading and streaming memory lifetimes. See the [Windows audio investigation](docs/research/08-xma-audio.md).
- [ ] Verify physical 120 Hz input for Flick-It at the capped 60 Hz display rate. The opt-in setting remains experimental until checked on a controller. See the [Windows audio investigation](docs/research/08-xma-audio.md).
- [ ] Formal Career performance gate passes with the normal SDL audio backend and default stock plugin. The experimental pacing candidate passed three 1280x720 default-WASAPI routes on November 23, but only one of three matched default stock-plugin routes passed every threshold; the formal default-build performance gate fails. See the [renderer performance research](docs/research/04-renderer-performance.md).
- [ ] Career playable start to finish. One unmodified save reaches the credits, tracked in the [Career progress log](docs/research/06-career-progress.md).
- [ ] Photo and Skate Reel features (render-target resolve/readback).

## M5 — Native renderer

- [ ] Replace Xenia's GPU backend with a native D3D12 (and later Vulkan) renderer.
- [ ] Port materials and shaders.

## M6 — PC options → 1.0

- [ ] Any resolution and aspect ratio.
- [ ] Higher or uncapped framerate where the game logic allows it. Refresh rates above 60 Hz and the opt-in `-ExperimentalGpuWaitBackoff` launch option are frozen until this milestone. Simulation and input timing at higher refresh rates still need testing. See [the renderer measurements](docs/research/04-renderer-performance.md).
- [ ] Keyboard and mouse.
- [ ] An installer that checks the user's game files are original.

## After 1.0

- Linux / Steam Deck.
- Decompilation, function by function.
