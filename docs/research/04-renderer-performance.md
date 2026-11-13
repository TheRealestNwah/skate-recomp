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
