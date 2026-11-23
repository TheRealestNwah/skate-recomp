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
2. **Verify Career performance at 60 Hz.** Use the RTV path at 1280x720. After warm-up, measure the same Career spawn and repeatable skating route. Ready means 55-60 Guest swaps/s while stationary and at least 50 swaps/s averaged over the movement route, with every five-second p95 interval at or below 25 ms and no recurring gaps over 100 ms across three independent runs. These are provisional M4 thresholds; record measurements and build commit in the renderer performance research note. Three initial silent-audio routes passed, but two later uninstrumented silent-driver repeats missed. Three new uninstrumented 1280x720 windowed routes on the default WASAPI backend all missed: stationary means were 40.8, 47.7, and 49.5 swaps/s; movement means were 47.4, 46.8, and 47.5 swaps/s; worst p95s were 52, 66, and 53 ms; longest gaps were 127, 265, and 263 ms. The prior normal-SDL recheck had two passing runs out of six. Five forced DirectSound routes with the packaged pacing plugin and one stock-plugin control also missed movement thresholds. The muted diagnostics do not isolate the cause. The performance gate has now been measured on the default backend and fails; keep the full Career playthrough on hold until repeated routes meet the thresholds.
3. **Finish audio and controller checks.** Complete the audio streaming/lifetime and physical controller checks in the baseline. Verify 120 Hz Flick-It sampling separately at the capped 60 Hz display rate, including release behavior and pause/resume; keep it opt-in until the physical check passes. Compare action timing around a verified pause/resume on the physical controller.
4. **Run the career completion playthrough.** The maintainer continues one unmodified save to the credits and records each session in the Career log. Fix blockers as individually tracked issues.
5. **Finish Photo and Skate Reel.** Validate capture, preview, save, relaunch, and replay behavior, including render-target resolve/readback.

The initial paced-silent routes met the provisional thresholds, but later uninstrumented repeats missed them. Three uninstrumented default-WASAPI routes at 1280x720 also failed the stationary and movement gates. Do not treat the current candidate as ready for the full manual Career playthrough; repeatable performance on the default backend and audible playback checks remain necessary, and gameplay may expose scene-specific drops.

The November 21 synthetic Career recheck found the experimental frame-pacing build averaged 58.1-59.6 Guest swaps/s over six valid movement routes. Two runs met every five-second movement threshold; four had at least one window above the p95 threshold, and three also fell below 50 swaps/s in a movement window. On November 22, three initial routes using the paced silent-audio diagnostic driver met the provisional thresholds, but two later uninstrumented repeats missed. Three new uninstrumented 1280x720 windowed routes on the default WASAPI backend averaged 46.8-47.5 swaps/s, had worst p95s of 52-66 ms, and gaps of 127-265 ms; all failed the gate. Five forced DirectSound routes with the packaged pacing plugin and one stock-plugin control also missed at least one movement threshold. The current candidate is not ready for a full manual Career playthrough. Default WASAPI playback is now available and has been measured, though audio was muted; stationary rates were also below the 55 swaps/s minimum. The cause of the performance misses is still unknown. GPU, draw-cost, and sampled CPU-stack profiles point toward CPU-side work on the Guest main and render threads during high-complexity views, but do not establish a single Guest function or Host draw stage as the cause. See the [November 21 recheck](docs/research/04-renderer-performance.md#november-21-m4-career-performance-gate-recheck) and [November 22 diagnostics](docs/research/04-renderer-performance.md#lower-rate-nested-binding-and-no-device-route-recheck).

A separate [production Host startup timing check](docs/research/12-production-host-startup.md) did not reach a Career scene and does not count toward the Career performance gate.

- [x] Menus, skater select, and free skate in San Vanelona.
- [x] Saves. A new Career save survived quit and relaunch in the [unattended save check](docs/research/07-saves.md).
- [ ] XMA audio and its threading and streaming memory lifetimes. See the [Windows audio investigation](docs/research/08-xma-audio.md).
- [ ] Verify physical 120 Hz input for Flick-It at the capped 60 Hz display rate. The opt-in setting remains experimental until checked on a controller. See the [Windows audio investigation](docs/research/08-xma-audio.md).
- [ ] Formal Career performance gate passes with the normal SDL audio backend. Three 1280x720 WASAPI routes currently fail the stationary and movement thresholds, so hold the full playthrough until repeated routes pass. Record the passing result in the [Career progress log](docs/research/06-career-progress.md).
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
