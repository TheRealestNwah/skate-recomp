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
2. **Verify Career performance at 60 Hz.** Use the RTV path at 1280×720. After warm-up, measure the same Career spawn and repeatable skating route. Ready means 55–60 Guest swaps/s while stationary and at least 50 swaps/s averaged over the movement route, with every five-second p95 interval at or below 25 ms and no recurring gaps over 100 ms across three independent runs. These are provisional M4 thresholds; record measurements and build commit in the renderer performance research note. Three independent routes with the packaged pacing plugin and a temporary paced silent-audio diagnostic driver met these thresholds on November 22. This supports starting the manual Career playthrough on the standard 60 Hz RTV build as a diagnostic, but does not yet establish acceptable performance with a real SDL playback backend. The prior normal-SDL recheck had two passing runs out of six, and two forced DirectSound routes on November 22 averaged only 32.6 and 37.6 swaps/s during movement. Those diagnostic captures used muted audio and auxiliary samplers; they establish that the silent-driver results do not transfer to this SDL backend, but do not isolate the cause.
3. **Finish audio and controller checks.** Complete the audio streaming/lifetime and physical controller checks in the baseline. Verify 120 Hz Flick-It sampling separately at the capped 60 Hz display rate, including release behavior and pause/resume; keep it opt-in until the physical check passes. Compare action timing around a verified pause/resume on the physical controller.
4. **Run the career completion playthrough.** The maintainer continues one unmodified save to the credits and records each session in the Career log. Fix blockers as individually tracked issues.
5. **Finish Photo and Skate Reel.** Validate capture, preview, save, relaunch, and replay behavior, including render-target resolve/readback.

The paced-silent routes support beginning the manual playthrough as a diagnostic, but the DirectSound route failures mean acceptable performance with real playback is not yet established. Normal audio-backend measurements and the manual playthrough are still needed to close the performance gate, and gameplay may expose scene-specific drops.

The November 21 synthetic Career recheck found the experimental frame-pacing build averaged 58.1–59.6 Guest swaps/s over six valid movement routes. Two runs met every five-second movement threshold; four had at least one window above the p95 threshold, and three also fell below 50 swaps/s in a movement window. On November 22, three independent routes using the packaged pacing plugin and a temporary paced silent-audio diagnostic driver all met the provisional FPS, p95, and gap thresholds. This is sufficient to begin the maintainer's manual playthrough on the standard 60 Hz RTV build. These three routes do not count toward the normal-SDL acceptance set, which remains two of six passing; the default SDL backend could not run because this profiling environment had no default playback endpoint. Two forced DirectSound routes completed but fell well below the movement threshold. The GPU, draw-cost, and sampled CPU-stack profiles continue to point toward CPU-side work on the Guest main and render threads during high-complexity views; no single Guest function or Host draw stage is established as the cause. See the [November 21 recheck](docs/research/04-renderer-performance.md#november-21-m4-career-performance-gate-recheck) and [November 22 diagnostics](docs/research/04-renderer-performance.md#lower-rate-nested-binding-and-no-device-route-recheck).

- [x] Menus, skater select, and free skate in San Vanelona.
- [x] Saves. A new Career save survived quit and relaunch in the [unattended save check](docs/research/07-saves.md).
- [ ] XMA audio and its threading and streaming memory lifetimes. See the [Windows audio investigation](docs/research/08-xma-audio.md).
- [ ] Verify physical 120 Hz input for Flick-It at the capped 60 Hz display rate. The opt-in setting remains experimental until checked on a controller. See the [Windows audio investigation](docs/research/08-xma-audio.md).
- [ ] Formal Career performance gate passes with the normal SDL audio backend; provisional silent-audio routes passed and the maintainer can begin the playthrough. Record the final normal-backend result in the [Career progress log](docs/research/06-career-progress.md).
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
