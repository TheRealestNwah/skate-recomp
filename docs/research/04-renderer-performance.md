# D3D12 renderer performance at Community Center

An unattended comparison on November 12, 2026 used ReXGlue SDK 0.10.1, a Ryzen 7 5800X, and a Radeon RX 9070 XT. A test-only controller loaded the same copied save, selected Shingo and Community Center in Free Skate, accepted the default settings, and left the skater idle. Both runs used the original USA disc, the same instrumented Port, and a 1280×720 window. Local captures confirmed the same spawn and camera before and after sampling.

| D3D12 render target path | Guest swaps/s | Longest Guest swap interval | Mean 3D-engine use |
| --- | ---: | ---: | ---: |
| ROV | 44.3 | 40.18 ms | 98.0% |
| RTV | 58.2 | 67.97 ms | 26.8% |

Guest swap rates are weighted by elapsed time across complete timing windows in each sample, covering 20.0 seconds for ROV and 25.1 seconds for RTV. They count submissions at the Guest's `VdSwap` site. GPU percentages are the mean of 25 one-second Windows 3D-engine samples filtered to the Port process. Sampling started after 37 seconds of loading and streaming. This comparison covers one run per renderer and one stationary scene. [Original GPU load report](https://github.com/TheRealestNwah/skate-recomp/issues/45)

## VP6 playback on RTV

With the SDK's default settings, RTV produced black frames during the EA HD bumper and intro. A readback immediately after the movie draw contained visible video. The subsequent resolve selected a depth target at EDRAM tile 720 as the owner of the 720 color tiles starting at tile 0. An unclipped depth-only draw with an 8192-pixel scissor had claimed all 2,048 tiles through address wraparound.

Setting `execute_unclipped_draw_vs_on_cpu=true` lets the SDK estimate the draw's extent from its vertices. The EA HD bumper and intro displayed on RTV with this option enabled. The launcher now enables it for RTV. The explicit ROV option retains the original settings. [Video playback issue](https://github.com/TheRealestNwah/skate-recomp/issues/38)

On November 13, the updated RTV configuration averaged 59.9 Guest swaps/s across 45.1 seconds of complete timing windows at the same Community Center spawn. The longest interval was 54.31 ms. This run did not collect GPU counters.

A second stationary run averaged 60.0 Guest swaps/s and 34.5% 3D-engine use, with a longest interval of 20.78 ms. In a separate comparison, the scripted controller pushed eight times from the spawn before sampling. The skater coasted during the measurement. Local captures showed the same starting camera and a similar stopping position on both paths.

| D3D12 render target path | Guest swaps/s | Longest Guest swap interval | Mean 3D-engine use |
| --- | ---: | ---: | ---: |
| ROV | 45.0 | 66.45 ms | 97.6% |
| RTV with draw extent estimation | 59.9 | 50.44 ms | 38.2% |

The title-screen attract movie also displayed moving video on RTV with draw extent estimation. One run repeated the same displayed frame from the 55-second sample through the 95-second sample while Guest swaps continued near 60/s. An RTV repeat and an ROV control both showed distinct frames through 95 seconds. The cause of the intermittent stall and movie audio remain unverified. [Attract movie stall](https://github.com/TheRealestNwah/skate-recomp/issues/53)

## Higher guest refresh rates

With RTV and draw extent estimation, the same scripted pushes and coast averaged 84.9 Guest swaps/s at a requested guest refresh rate of 90 Hz, with 54.1% mean 3D-engine use. At a requested 120 Hz, the run averaged 73.7 Guest swaps/s with 47.2% mean 3D-engine use. The 120 Hz run approached its target in menus.

A repeat test captured the skater at roughly 450 ms intervals after the same eight pushes. At both 60 and 90 Hz the skater crossed the road and hit the curb within the same capture interval. The 90 Hz run averaged 89.5 Guest swaps/s and the 60 Hz run averaged 60.0. This checks the timing of one short Free Skate sequence. Career, Flick-It timing and movie audio still need verification at higher refresh rates.

## Guest graphics wait

In a local sample of the Guest swap thread at 120 Hz, 1,890 of 4,000 instruction pointers were in the graphics polling routine at `0x82CB4AA0`. The Guest uses Xenon's `db16cyc` delay instructions in this routine. ReXGlue emits no host delay for them. The Port can briefly wait on a high-resolution Windows timer after repeated polls, then issue the next Guest poll normally.

In a 25-second Free Skate comparison with the same executable at 60 Hz, the setting reduced process CPU time from 49.41 to 30.33 seconds. Guest swap rate was 60.0/s with the setting off and 59.9/s with it on. The longest Guest swap interval in the sample was 21.46 ms with it off and 49.34 ms with it on. The launcher keeps this setting opt-in.

The same test sequence also ran with higher guest refresh rates. CPU time covers the full 25-second sampling window and Guest swap rates cover the complete timing windows within it.

| Guest refresh request | Backoff | Process CPU time | Guest swaps/s | Longest swap interval |
| --- | --- | ---: | ---: | ---: |
| 90 Hz | Off | 54.64 s | 88.0 | 25.24 ms |
| 90 Hz | On | 39.14 s | 89.9 | 22.23 ms |
| 120 Hz | Off | 52.45 s | 78.0 | 19.95 ms |
| 120 Hz | On | 41.00 s | 94.3 | 18.76 ms |

### Guest wait tuning

Two muted Free Skate comparisons on November 14 tested shorter timer requests against the current 75 µs backoff. Each run followed the same eight pushes and coast at Community Center. The second comparison also recorded the p99 upper bound for each complete five-second timing window. CPU time covers the 25-second sample, and the p99 column shows the range across the four complete windows in the second run.

| Timer request | Polls before waiting | Guest refresh | Guest swaps/s, both runs | Process CPU time, both runs | p99 upper bounds, second run | Longest gaps, both runs |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| 75 µs | 256 | 60 Hz | 59.9, 60.0 | 33.50 s, 31.16 s | 19 ms | 49.99 ms, 32.76 ms |
| 40 µs | 512 | 60 Hz | 60.0, 60.0 | 34.33 s, 32.00 s | 19 ms | 20.49 ms, 19.53 ms |
| 25 µs | 1,024 | 60 Hz | 60.0, 60.0 | 32.78 s, 31.17 s | 19 ms | 19.46 ms, 18.93 ms |
| 75 µs | 256 | 90 Hz | 89.4, 90.0 | 44.39 s, 39.94 s | 13 ms | 24.43 ms, 13.52 ms |
| 40 µs | 512 | 90 Hz | 85.4, 90.0 | 42.27 s, 39.08 s | 13–14 ms | 24.89 ms, 13.66 ms |
| 25 µs | 1,024 | 90 Hz | 89.7, 89.9 | 40.86 s, 40.64 s | 13–14 ms | 21.72 ms, 21.80 ms |

Both shorter waits had smaller worst gaps in the 60 Hz samples. The 90 Hz results varied by run, and the 75 µs setting had the lowest p99 bounds in the second comparison. The Port retains the current backoff parameters while testing other sources of CPU load.
