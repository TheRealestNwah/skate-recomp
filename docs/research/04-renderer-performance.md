# D3D12 renderer performance at Community Center

An unattended comparison on November 12, 2026 used ReXGlue SDK 0.10.1, a Ryzen 7 5800X, and a Radeon RX 9070 XT. A test-only controller loaded the same copied save, selected Shingo and Community Center in Free Skate, accepted the default settings, and left the skater idle. Both runs used the original USA disc, the same instrumented Port, and a 1280×720 window. Local captures confirmed the same spawn and camera before and after sampling.

| D3D12 render target path | Guest swaps/s | Longest Guest swap interval | Mean 3D-engine use |
| --- | ---: | ---: | ---: |
| ROV | 44.3 | 40.18 ms | 98.0% |
| RTV | 58.2 | 67.97 ms | 26.8% |

Guest swap rates are weighted by elapsed time across complete timing windows in each sample, covering 20.0 seconds for ROV and 25.1 seconds for RTV. They count submissions at the Guest's `VdSwap` site. GPU percentages are the mean of 25 one-second Windows 3D-engine samples filtered to the Port process. Sampling started after 37 seconds of loading and streaming. This comparison covers one run per renderer and one stationary scene. [Original GPU load report](https://github.com/TheRealestNwah/skate-recomp/issues/45)

Separate captures from the Guest output showed the EA HD bumper and intro on ROV. RTV produced completely black frames during those clips and showed the title screen afterward. Disabling direct host resolves or 16-bit gamma render targets individually still produced black movie frames on RTV. [Video playback issue](https://github.com/TheRealestNwah/skate-recomp/issues/38)
