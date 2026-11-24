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

### GPU command worker spin limit

Windows thread CPU times showed that `GPU Commands` was the busiest thread in the repeat samples. A local ReXGlue GPU plugin build varied the iteration limit before its worker waits for another command submission. Each run used the same cached pipeline descriptions, muted audio, the 75 µs Guest backoff, and the same Community Center route. CPU times cover 25 seconds. Each configuration ran once at each refresh rate.

| Guest refresh | Worker spin limit | Process CPU time | GPU Commands CPU time | Guest swaps/s | p99 upper bounds | Longest gap |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| 60 Hz | 500 (SDK default) | 30.05 s | 8.44 s | 60.0 | 19 ms | 25.88 ms |
| 60 Hz | 32 | 30.81 s | 8.92 s | 60.0 | 19–20 ms | 19.76 ms |
| 60 Hz | 0 | 29.77 s | 8.70 s | 60.0 | 19 ms | 18.88 ms |
| 90 Hz | 500 (SDK default) | 37.89 s | 13.19 s | 90.0 | 13–14 ms | 13.57 ms |
| 90 Hz | 32 | 36.27 s | 12.73 s | 90.0 | 13 ms | 13.81 ms |
| 90 Hz | 0 | 36.75 s | 12.61 s | 90.0 | 13–14 ms | 13.95 ms |

The worker still used 8.70 seconds at 60 Hz and 12.61 seconds at 90 Hz with the spin limit set to zero. CPU use shifted by less than one second on that thread in every comparison. The shipping build uses the stock SDK plugin.

### GPU command worker profile

An instruction-pointer sample of the `GPU Commands` thread at 90 Hz captured 4,000 locations, of which 1,461 mapped into the ReXGlue GPU plugin. These samples include time when the thread was blocked, so the counts cannot be interpreted as CPU time. The most frequent locations inside the plugin were the base register writer (182 samples), its D3D12 override (118), D3D12 draw binding updates (116), and sequential register packet decoding (87).

The same Free Skate route tested `d3d12_submit_on_primary_buffer_end=false` with the stock SDK plugin. The Guest swap rate stayed at 60.0/s and 90.0/s at the respective refresh rates. Process CPU time rose from 30.05 to 31.17 seconds at 60 Hz and from 39.05 to 39.42 seconds at 90 Hz. These are single comparisons with a 25-second sample per configuration. The launcher keeps the SDK submission setting.

## Host presentation during the November 19 playtest follow-up

The stock ReXGlue 0.10.1 achievement toast attaches an ImGui dialog at launch. The dialog stays attached when its queue is empty, which keeps the Presenter repainting at the monitor's cadence and can display the same Guest output multiple times. For one scripted route in the Suburbs on a 165 Hz display, PresentMon recorded 3,198 Host presents in 20 seconds with the stock overlay and 1,198 with the overlay disabled in a diagnostic build. Both runs used RTV at the default 60 Hz Guest setting. The Host now attaches the SDK toast renderer when a notification arrives and detaches it after the queue finishes. Menus and the F3 overlay can still request their own redraws. [RTV stutter report](https://github.com/TheRealestNwah/skate-recomp/issues/118) · [PresentMon console](https://github.com/GameTechDev/PresentMon/blob/main/README-ConsoleApplication.md)

A test build queued two notification events together through the SDK achievement manager. PresentMon recorded 60 Host presents/s before the events, roughly 160/s for nine seconds while the notifications were drawn, then 60/s again. Guest swaps remained near 60/s. A repeat of the physical-controller playtest is still needed to compare the reported stutter.

For the gameplay-speed investigation, a standalone probe drove the SDK clock from 16 concurrent threads. One 501 ms wall-clock window measured 609 ms of Guest time. ReXGlue 0.10.1 reads the Host counter before taking the clock mutex. A late sample can move the stored Host timestamp backward, making the next update count part of the same time again. In the scripted gameplay run, five-second Guest clock samples stayed within 0.1% of wall time. A separate probe resumed a copy of the Career save and verified two pause/resume cycles. In the windows around those cycles, the Guest clock stayed within 0.002% of wall time and the Guest submitted 59.2–60.0 swaps/s. Gameplay actions and animation speed still need direct timing measurements. [Gameplay speed report](https://github.com/TheRealestNwah/skate-recomp/issues/119)

### Unattended Career follow-up

An automated controller resumed a copy of the same Career save and repeated eight pushes in the Suburbs at a 60 Hz Guest display rate. The frame-rate drop reproduced with synthetic input, both fullscreen and in a 1280×720 window. Moving Host presentation to the UI thread, muting audio, disabling RTV draw extent estimation on the CPU, and disabling D3D12 submission at the end of each primary buffer each reproduced the drop in independent runs. These short runs covered the same scripted sequence. Frame rates during the sequence ranged from about 21 to 33 Guest swaps/s across the trials.

The ROV control also fell from about 60 in the menus to 32.9 Guest swaps/s over a five-second interval during the same push sequence. The longest interval in that window was 63.95 ms. Captures confirmed movement in the Suburbs, and the vertical-blank interrupt rate stayed near 60 Hz. This run used the launcher’s normal draw extent setting for ROV.

A diagnostic at the Guest graphics interrupt handler counted about 60 vertical-blank interrupts per second while the Guest submitted 27–34 frames per second. The Guest timebase remained close to wall time in those windows. Several five-second windows had a longest interval between swaps of 42–66 ms. The Guest produces frames slowly and unevenly even with steady vertical-blank delivery. The unattended Host presentation trace did not cover the full movement sample, so the comparison uses Guest swap timings. [RTV stutter report](https://github.com/TheRealestNwah/skate-recomp/issues/118)

In the Replay Editor, with the speed set to 100%, the playhead advanced approximately 1.95 seconds on a displayed 26-second timeline over 2.01 seconds of wall time. This estimate uses two captured playhead positions and the displayed clip length. It measures replay playback.

A separate probe held the Guest display at 60 Hz and changed only whether the Port applied its 120 Hz Flickit override. At 120 Hz, the skater landed by about 0.67 seconds after the scripted stick flick both before and after a verified pause and resume in the station corridor. In the retail 60 Hz input control, both corridor ollies were still airborne near one second after the flick. A second 120 Hz run reproduced the early landing outdoors. A collision invalidated the second run’s post-resume ollie. These measurements are bounded by the screenshot cadence. The input override is a lead for action timing, and these trials leave overall simulation speed and the reported intermittent pause slowdown open. The stock input control still dropped below 60 Guest swaps/s. [Gameplay speed report](https://github.com/TheRealestNwah/skate-recomp/issues/119)

A repeat used a copied save for each rate and verified Career navigation before issuing three identical scripted flicks in the station corridor. The first started from the same corridor view in both runs. At 120 Hz, the landings appeared around 0.6–0.7 seconds after the flick. At 60 Hz, two captures still showed the skater airborne around one second, and the third showed a grounded skater by about 1.2 seconds. Both rates retained the same approximate timing across one pause and resume. The shipping default returns to the retail 60 Hz input worker rate. The experimental 120 Hz override can still be enabled at launch. [Input timing audit](08-xma-audio.md#flickit-input)

The verification build linked the production input timing object and ran with the new default. All three corridor ollies were still airborne around 0.94 seconds after the flick. One preceded a captured pause menu, and two followed resume. Guest frame submissions still reached 32.0/s in Career while vertical-blank delivery stayed near 60 Hz. A sampling probe attributed sustained polling on the Guest swap thread to the command-ring space check at return address `0x82CB1250` during both the 60 FPS menu windows and the slower Career windows. [RTV stutter report](https://github.com/TheRealestNwah/skate-recomp/issues/118)

A local SDK experiment published the primary ring read pointer at packet boundaries after crossing the game's requested 16-dword interval. With the intermediate writebacks enabled, the scripted pushes from the station produced 35.0 Guest swaps/s in a five-second movement window. The same build with intermediate writebacks disabled produced 39.6 swaps/s in its movement window. The largest gaps in those windows were 49.49 ms and 48.33 ms, respectively. Captures confirmed the same starting corridor and end of the push route, and both runs maintained about 60 vertical-blank interrupts/s. This pair did not show an improvement from earlier read-pointer publication. The writeback change remains test-only. [RTV stutter report](https://github.com/TheRealestNwah/skate-recomp/issues/118)

### Command processor frame budget

A diagnostic ReXGlue build timed the command worker while it processed the menu, the station corridor in Career, and the same eight scripted pushes. Times are wall-clock milliseconds per processed swap over complete one-second windows. `IssueDraw` includes copy and resolve commands from the Guest, and D3D12 submission time includes recording the deferred command list. The measured phases have nested calls and can overlap.

| RTV phase | Processed swaps/s | Draw commands/frame | `WAIT_REG_MEM` ms/frame | `IssueDraw` ms/frame | D3D12 submission ms/frame |
| --- | ---: | ---: | ---: | ---: | ---: |
| Main menu | 60.1 | 379 | 13.24 | 1.44 | 0.99 |
| Career, stationary | 31.4 | 2,742 | 13.16 | 10.94 | 2.77 |
| Career, eight pushes | 33.1 | 2,618 | 12.52 | 10.53 | 2.69 |
| Career, after pushes | 40.8 | 2,158 | 8.67 | 9.04 | 2.45 |

The command worker was idle for 0.32 ms per frame on the push route. No D3D12 submission-fence sleeps occurred in any of the measured RTV windows. The slow Career frames added draw and submission work while retaining the substantial command packet wait cost. The timing measures work and waits on the Host command worker. [RTV stutter report](https://github.com/TheRealestNwah/skate-recomp/issues/118)

The ROV control averaged 34.9 processed swaps/s during its five movement windows. It spent 11.57 ms per frame in `WAIT_REG_MEM` and 10.68 ms handling draw commands. ROV also recorded 20 D3D12 fence waits in those windows, averaging 0.71 ms per frame with a longest single wait of 12.50 ms. During the coast, the longest ROV fence wait reached 27.10 ms. [ROV throughput report](https://github.com/TheRealestNwah/skate-recomp/issues/45)

#### Guest memory polling

Almost all measured `WAIT_REG_MEM` time came from Host sleeps while waiting for Guest memory values. In the RTV control with the detailed sleep probe, the guest packets requested 1 ms between polls and the Windows calls averaged 2.08 ms during the pushes. A scratch SDK build capped each sleep with a 250 µs high-resolution timer and kept checking the same predicate. It retained vertical-blank delivery and took about 0.58 ms per sleep in that run. The shorter intervals caused the worker to recheck the same predicates more often.

In the first comparison, the control averaged 28.5 processed swaps/s and 28 output gaps of at least 40 ms during six complete movement windows. The 250 µs cap averaged 36.9 swaps/s and five such gaps across six windows. PresentMon reported dropped ETW events in both runs and did not produce a usable display trace. These counts describe command-processor output. The experiment has no effect on normal builds.

The reverse-order repeat ran without PresentMon. The 250 µs run averaged 38.4 swaps/s with two output gaps of at least 40 ms. The stock control averaged 59.7 swaps/s with zero such gaps. The captures matched, and the movement windows had 2,615 and 2,628 draw commands per frame, respectively. Draw handling took 10.01 ms/frame with the cap and 5.24 ms/frame in the fast stock run. D3D12 submission time also fell from 2.54 to 1.33 ms/frame. These results reproduce both slow and fast execution at similar draw counts. The wait cap reduced individual sleep duration, but the comparison did not isolate its effect from the change in command throughput between runs.

Two further muted stock runs added Windows command-thread CPU time, cycle counts, and timer resolution. They averaged 59.4 and 57.0 swaps/s during the pushes, with zero gaps of 40 ms or longer in either sample. The reported system timer resolution held at 1.000 ms through the movement windows, including a brief 46 FPS dip that processed about 3,400 draws per frame. The sustained 30 FPS state did not recur in either run, so there is no matched slow-state CPU cycle or timer measurement. What changes the cost per draw between runs remains open.

Later traces captured 29.9 and 52.9 swaps/s in two separate runs at the Career spawn, before showing any notification. The slow window processed 2,665 draws/frame and the faster window 2,724. The slower run used 62.1 million command-worker cycles/frame versus 46.1 million in the faster run. Draw handling took 11.80 versus 7.56 ms/frame, and memory waits took 12.44 versus 6.51 ms/frame. Windows reported 1.000 ms timer resolution through both windows. The measured average cycles per unit of worker CPU time were similar. This comparison confirms higher worker cost when draw count is similar, with individual draw and texture workloads still uncontrolled across boots.

### Same-session Career drop

Two muted RTV runs used copied Career saves at the default 60 Hz Guest display and retail input rate. Each stayed in Career while scripted input alternated pushes, turns, and coasting. The first dipped from 59.3 to 45.1 processed swaps/s across two push sequences. Worker CPU time per frame rose from 8.80 to 14.10 ms as the skater moved through different scenery.

The second run captured a drop from 58.4 to 32.1 swaps/s within one Career session, then recovered to 58.1 after another turn. Captures place the earlier stretch near the station and the later slow stretch on the nearby street. The rows below aggregate complete one-second windows for the same worker thread and process.

| Segment | Processed swaps/s | Draws/frame | Worker CPU ms/frame | Draw handling ms/frame | Guest memory wait ms/frame |
| --- | ---: | ---: | ---: | ---: | ---: |
| Coast near station | 58.4 | 1,881 | 11.41 | 6.64 | 5.54 |
| Push after turning toward street | 32.1 | 3,212 | 16.59 | 11.06 | 13.02 |
| Push after the next turn | 58.1 | 2,022 | 11.96 | 6.92 | 5.15 |

The slow push processed 71% more draw commands per frame and spent about 7.5 ms more per frame in `WAIT_REG_MEM`, almost all of it sleeping while rechecking Guest memory. Draw handling rose by about 4.4 ms per frame, with similar time per draw across the two windows. D3D12 fence waits measured 0 ms, the Guest vertical-blank signal stayed near 60 Hz, and the reported system timer resolution stayed at 1 ms. The route changed the view and the command workload. This explains the immediate cost of the measured low-30s stretch. The earlier run-to-run cost changes at similar draw counts and irregular Host display hitches remain open. [Stutter report](https://github.com/TheRealestNwah/skate-recomp/issues/118)

#### Polling delay switched in the same run

Two muted RTV trials followed the street route, then alternated stock sleeps and the 250 µs polling cap in six 11-second intervals without moving the controller. The second trial reversed the order. The worker logged each change. The analysis skipped incomplete windows and the first 1.25 seconds after changing the setting. The probe used a scratch SDK build.

The first trial processed about 2,100 draws/frame and ranged from 35.1–37.4 swaps/s on stock sleeps to 36.0–39.3 swaps/s with the shorter cap. The second processed about 3,000 draws/frame in its first three intervals. It measured 27.7 swaps/s with the cap, 27.4 on stock sleeps, then 26.4 with the cap. The cap reduced individual Windows sleeps from about 1.9 ms to 0.7–0.8 ms, while increasing the number of polls. Long output gaps continued. The matched low-frame-rate intervals show little throughput benefit from the cap. [Stutter report](https://github.com/TheRealestNwah/skate-recomp/issues/118)

In a later muted route, a per-address trace accounted for nearly all measured memory-wait time at one `WAIT_REG_MEM` packet site. The command processor waited for the whole word at physical address `0x1fc9b004` to become zero. The packet address ends in `06` because its low two bits encode the word's byte order. The site averaged 12.10 ms/frame in a 30.0 swaps/s push segment and 11.92 ms/frame in a 27.6 swaps/s push segment. The same site also appeared in faster views. [Stutter report](https://github.com/TheRealestNwah/skate-recomp/issues/118)

A probe in the Guest vertical-blank callback recorded the zero store at virtual address `0xffc9a004`. The runtime's 4 KiB physical mapping translates that address to `0x1fc9b004`, matching the word in the wait packet. The callback cleared the word about 60 times per second in the 60 FPS menu and continued to clear it in Career. The command worker waits on the game's vertical-blank handshake at this site.

A diagnostic build signalled a Windows event after the callback's zero store. The worker could wake on that signal while keeping the original packet predicate and a timed fallback. Two same-process tests alternated the event wake and stock polling in opposite orders. In the more closely matched repeat, both modes processed about 2,200 draws/frame. Event wake measured 58.95–59.65 swaps/s and 4.52–5.77 ms of memory wait per frame. Stock measured 56.98–59.12 swaps/s and 5.55–6.22 ms of memory wait. Both modes had zero output gaps of at least 40 ms in those windows. These test windows were already close to 60 FPS.

The same repeat then restored stock polling and alternated normal and above-normal Windows priority on the command worker. At about 2,200 draws/frame, the two normal-priority segments averaged 58.6 and 57.5 swaps/s. The above-normal segments averaged 59.7 and 57.5 swaps/s. All four had zero output gaps of at least 40 ms. The command worker's logger and Windows both verified the priority changes.

A stationary spawn view subsequently settled at 39.8 swaps/s before switching modes. The six alternating intervals in the same process processed 2,689–2,719 draws/frame. Stock polling produced 38.37, 45.03 and 42.31 swaps/s. Event wake produced 45.18, 51.80 and 48.87 swaps/s. Memory wait averaged 7.97–10.52 ms/frame on stock and 4.49–6.32 ms/frame on event wake. Neither mode produced a gap of at least 40 ms in the sampled windows. Raising worker priority separately again gave overlapping results at the same draw count.

In the reverse-order repeat, the more closely matched windows processed 2,807–2,869 draws/frame. Stock ran at 34.24–35.29 swaps/s and event wake ran at 41.23–45.56 swaps/s. Memory wait dropped from 12.13–12.64 to 6.53–8.02 ms/frame. The first stock interval handled 3,304 draws/frame, so the matched ranges exclude it.

An eight-interval run then compared stock polling, a plain 2 ms sleep, and the event with both 2 ms and 1 ms fallbacks. The scene processed 2,691–2,723 draws/frame throughout.

| Wait policy | Processed swaps/s across two intervals | Memory wait ms/frame |
| --- | ---: | ---: |
| Stock polling | 45.78–48.42 | 7.01–7.52 |
| Plain 2 ms sleep | 49.27–51.82 | 6.68–7.24 |
| Event with 2 ms fallback | 55.78–57.69 | 4.09–4.55 |
| Event with 1 ms fallback | 58.69–58.78 | 4.45–4.72 |

The event variants increased throughput beyond the plain-sleep control. Every measured interval had zero command-worker output gaps of at least 40 ms. The event prototype leaves the checked memory value and the Guest's vertical-blank callback unchanged. It uses the observed physical address and is local to the diagnostic build.

Two lighter trials used the stock runtime and D3D12 draw implementation with per-frame packet counts in place of detailed profiling. Their stationary observations ranged from 50.3 to 60.0 swaps/s and never crossed the respective thresholds for starting the slow comparison. A follow-up used an event owned by the graphics plugin and learned the physical address from the Guest callback. It alternated modes in a stationary view of about 2,100 draw packets/frame. Stock polling ran at 58.51–59.58 swaps/s. The learned-address event ran at 59.76–59.87 swaps/s and recorded 481–545 wake-ups per interval. All six intervals completed with no gaps of at least 40 ms. This verifies the address handoff and event in the lighter build. The larger gain at low rates was measured in the detailed diagnostic build.

A further validation relinked the normal Host and Guest build with scripted input and the callback notification needed by the wake-up patch. It used the same runtime DLL as the normal launcher, the normal D3D12 draw code, and only per-swap and draw-packet counters in the command worker. All runs used 60 Hz RTV, muted audio, and a copy of the same Career save. The first stock run briefly measured 33.9 and 35.5 swaps/s in consecutive one-second windows at the spawn. The six subsequent street observations averaged 48.6–60.0 swaps/s. By the time the run alternated modes, stock averaged 58.75–59.87 and wake-up averaged 59.63–60.00 swaps/s, with about 2,500 draw packets per frame. This comparison covers the recovered state.

Two stationary twelve-interval comparisons kept the skater at the spawn. One started with a copied Port shader cache. The other started with a fresh Port cache and reversed the mode order. Both maintained about 60 swaps/s in each mode after loading, at about 2,700 draw packets per frame. The longest swap gap in the copied-cache intervals was 20.89 ms on stock and 18.73 ms with wake-up. The fresh-cache stock intervals included a 33.99 ms gap. These comparisons measured command-worker output at the same view and did not enter a sustained low-rate state.

A route survey sampled ten additional views. It ended with about 4,000 draw packets per frame in the alternating comparison, still near 60 swaps/s in both modes. A separate test restricted only the Port process to two logical CPUs. This produced unstable performance at around 2,300 draw packets per frame. Stock intervals ranged from 9.33 to 46.31 swaps/s and wake-up from 29.58 to 57.68 swaps/s. Later intervals in both modes overlapped around 28–36 swaps/s. This was an artificial scheduling constraint and the process's affinity was restored on exit.

A timing-only follow-up measured the delay from the callback notification to the graphics worker observing the cleared flag. It counted waits that saw a newer clear within 16 ms. In complete alternating windows near 60 swaps/s, stock polling averaged 1.553 ms across 2,095 waits. Direct wake-up averaged 0.014 ms across 2,153 waits. Each mode had one measured window with a swap gap over 40 ms. The patch substantially reduces the delay at the wait site. Its effect on the sustained 30 FPS drop in the normal build is still unverified.

A reverse-order repeat let the two-CPU constraint settle for 28 seconds before switching modes. At 2,694–2,723 draw packets per frame, stock produced 26.96–29.57 swaps/s and wake-up produced 26.87–29.28 swaps/s. The measured delay after the flag clear averaged 1.463 ms on stock and 0.747 ms on wake-up. Both modes still had 169–182 ms worst swap gaps across the intervals. In this artificial low-rate state, the shorter wait produced overlapping throughput and stutter measurements.

Four further Career entries alternated stock polling and event wake in 4.2-second intervals from the stationary spawn. Two began with stock and two began with wake-up. Each mode contributed 64 complete one-second windows after filtering switches. Draw-packet averages ranged from about 2,450 to 3,004 per frame across runs. Stock averaged 59.74–59.99 swaps/s and event wake averaged 59.94–60.00 swaps/s. No selected window fell to 38 swaps/s or below, and neither mode recorded a swap gap of at least 40 ms in those windows.

Windows reported a boot at 10:12:25 local time, after the earlier brief low-30s measurements and before the afternoon comparisons. RTSS still had a global 160 FPS limit. The trials on either side used the same game save and launcher refresh setting. The post-boot measurements describe a separate system session.

Rechecking the two pre-reboot captures located the low windows while the skater was still at the spawn. Four one-second windows ranged from 32.0 to 35.5 swaps/s with approximately 3,500–3,600 draw packets per frame and maximum gaps of 34.8–37.0 ms. A fifth spawn window fell to 21.8 swaps/s and included a 158 ms gap. Other pre-reboot windows with similar draw counts reached roughly 55–60 swaps/s. Similar scene complexity accompanied both frame rates, and these captures did not record where the graphics worker was waiting.

Two same-process comparisons alternated stock polling, event wake, the Windows deadline timer, and the combination. Both held the skater at the spawn and retained the 60 Hz mode. The measured view averaged about 2,700 draw packets per frame. All four modes delivered about 60 swaps/s. The longest command-worker swap gaps and vertical-blank counts below combine complete windows from both runs.

| Mode | Vertical-blank intervals over 18 ms | Longest swap gap |
| --- | ---: | ---: |
| Stock | 104 of 2,182 | 33.50 ms |
| Event wake | 39 of 2,179 | 18.90 ms |
| Deadline timer | 0 of 2,418 | 33.80 ms |
| Both | 0 of 2,241 | 17.60 ms |

The timer improved when vertical blanks arrived. Event wake reduced how long the command worker waited after the flag cleared. With both enabled, the longest frame gap in this stationary comparison approached the normal 16.67 ms frame interval.

Two muted skating runs reversed which modes were active during a scripted series of turns, pushes, and ollies. All four modes stayed near 60 swaps/s, and RTSS also stayed near 60 FPS. The combined mode had zero vertical-blank intervals over 18 ms out of 2,360, with a maximum swap gap of 33.1 ms. Stock had 13 of 2,361 over 18 ms and a maximum swap gap of 34.0 ms. One deadline-only window had a 63.2 ms swap gap. The skater and camera took different paths in the two runs. Draw-packet averages varied by mode, so these results describe the sampled routes.

Restricting the Port process to two logical CPUs reproduced a sustained low rate at approximately 2,700 draw packets per frame. Stock delivered 34.08 swaps/s with a longest gap of 180.2 ms. With event wake and the deadline timer together, the rate was 33.31 swaps/s and the longest gap was 177.5 ms. Event wake alone delivered 31.04 swaps/s and the deadline timer alone delivered 25.78 swaps/s. This artificial CPU constraint shows that the combined timing changes leave long stalls and low throughput under heavy scheduling pressure.

A later two-CPU run kept the stock timing and alternated the Windows priority of the vertical-blank worker between `Normal` and `AboveNormal`. At approximately 3,423 draw packets per frame, the average rate rose from 25.11 to 43.92 swaps/s. Vertical-blank intervals over 18 ms decreased from 275 of 1,049 to 34 of 1,160. One raised-priority phase averaged 59.10 swaps/s. A later phase with the same settings and still-regular vertical blanks fell to 28.87 swaps/s, with only 13 of 552 interrupt intervals over 18 ms. Longest swap gaps reached roughly 170 ms at each priority. Raising the priority improved interrupt timing under the artificial CPU constraint, and the output continued to drop at times even when interrupts were regular.

Reversing the priority order on the same two logical CPUs produced about 2,450 draw packets per frame. On stock timing, raising the priority reduced the count of vertical-blank intervals over 18 ms from 258 of 1,168 to 10 of 1,044. Output held at 27.52 swaps/s at normal priority and 26.95 at the raised priority. Each priority had 17 one-second windows with a swap gap of at least 40 ms. Timely vertical blanks alone were insufficient to sustain normal output under that constraint.

An individual frame trace captured a 230.93 ms gap during skating, 0.58 seconds after switching back to stock timing. The graphics worker spent 199.25 ms inside `D3D12CommandProcessor::IssueSwap` and 14.78 ms waiting on the vertical-blank flag. The Guest frame trace independently measured a 231.49 ms gap. This capture places most of the delay inside the swap path. The reason for the delay within that path is still open.

Four subsequent scripted runs removed the diagnostic worker counters and held each mode fixed for the entire launch. The order was stock, combined, combined, stock. Stock averaged 59.86 Guest swaps/s, and the combined patch averaged 59.87 across 22 complete five-second windows per mode. Stock had one window with a gap of at least 40 ms, with a maximum gap of 49.65 ms. The combined patch had two such windows, with a maximum gap of 52.21 ms. Across 288 one-second RTSS samples covering the measured route, the lowest rate was 57.09 FPS. This comparison found no measurable benefit to throughput or intermittent stutter in the normal 60 Hz configuration. It did not reproduce the sustained low-rate state.

The project’s fast code generator emitted the notification hook immediately after the Guest store that clears the flag. Removing the hook call left the generated callback identical to the existing one. An isolated relink inserted that callback into the existing generated build, and the muted Career run confirmed it reached the graphics plugin. The measured phase averaged 59.96 Guest swaps/s across six windows, with a longest gap of 33.16 ms. This verifies the code generation hook and runtime call in the test build.

A fixed-stock trace added separate timings for the Guest image refresh, the presenter mutex, command submission, and DXGI `Present`. During the scripted route at roughly 1,855 draw packets per frame, the graphics worker and RTSS each averaged 59.99 FPS. The longest of 77 selected one-second windows was a 35.1 ms frame gap. The route did not reproduce the earlier stall inside `IssueSwap`. A separate 384.85 ms Career loading gap spent 0.70 ms inside `IssueSwap`, consistent with the earlier captures of a distinct loading stall.

A follow-up stayed at the Career spawn for 75 seconds, then changed only the Port process’s CPU affinity. Unrestricted output averaged 59.56 swaps/s at 2,593 draw packets per frame. The two-CPU phase averaged 25.69 swaps/s at 2,539 packets per frame and returned to about 58–60 in the final five seconds with the restriction still in place. Of 188 frame gaps of at least 40 ms in that phase, 140 spent most of the time outside the measured vertical-blank wait and `IssueSwap`. The worst gap was 233.36 ms, including 31.87 ms at the vertical-blank wait and 0.70 ms inside `IssueSwap`. An additional one-CPU control lowered output further. Restoring affinity returned output to 59.80 swaps/s. This artificial slowdown spent most of its longest frame outside the swap path where the separate 231 ms skating hitch was observed.

A command-queue trace repeated the two-CPU restriction twice in one Career session. It measured 21.67 and 33.05 swaps/s under the restriction and returned to 60.00 and 60.01 after restoring CPU access. The second restricted phase and the preceding unrestricted phase had essentially the same draw count, 2,455–2,456 packets per frame. The median hitch in the two restricted phases spent 32.0 and 19.2 ms waiting on an empty Guest command ring. One 207.0 ms hitch spent 162.7 ms waiting on that ring, 36.2 ms waiting for the vertical-blank flag, and 0.7 ms in `IssueSwap`. The instrumented D3D12 submission-fence wait measured 0 ms in that frame. The graphics worker had no Guest commands to process for most of that particular delay. The trace does not determine which Guest thread held up command production.

Raising the priority of the Guest thread that submits `VdSwap` produced inconsistent results at about 2,700 draw packets per frame on two logical CPUs. The two raised-priority phases measured 55.05 and 6.14 swaps/s, and the intervening normal-priority phases measured 31.33 and 26.33. The last raised-priority phase recorded several synchronous DXGI `Present` calls of 170–374 ms. The 374.14 ms call returned success and ran on the graphics worker. The surrounding swap took 374.96 ms, with 0.65 ms in the Guest image submission callback. This trace identifies a separate presentation stall under the artificial constraint. The earlier 231 ms skating capture lacks this level of timing detail.

Reversing the order at a matched load of about 2,440 draw packets per frame reproduced the regressions. The two raised-priority phases fell to 6.68 and 5.63 swaps/s, compared with 27.27 and 24.90 at normal priority. Output recovered to 59.39 swaps/s when the test restored CPU access. Raising the Guest render thread’s priority is unsuitable as a workaround for this slowdown.

Forcing presentation onto the UI thread in the reversed test yielded 20.87 and 20.41 swaps/s at normal render-thread priority and 8.09 and 8.00 after raising it. RTSS agreed within 0.18 FPS. The trace recorded no DXGI `Present` call above the 12 ms logging threshold. Long command-worker gaps persisted, reaching 1.35 seconds in a raised-priority phase. Restoring CPU access returned output to 59.13 swaps/s. Moving presentation can prevent the graphics worker from waiting inside `Present` while leaving the induced command-production problem unresolved.

A later unrestricted 75-second watch at the same Career spawn caught the frame rate moving between 32.95 and 60.10 FPS in RTSS, including five complete samples at 38 FPS or below. RTSS averaged 46.43 FPS, closely matching 46.45 graphics-worker swaps/s at about 2,704 draw packets per frame. The longest measured worker gap was 36.5 ms. In the slow-window cadence samples, 654 intervals were between 30 and 40 ms, with no intervals reaching 40 ms. This reproduces intermittent low output in the 60 Hz configuration alongside separate captures of longer stutter events.

Within that unrestricted session, the five worker windows at 38 swaps/s or below had a median of 36.35 swaps/s and 2,727 draw packets per frame. The eleven windows at 55 swaps/s or above had a median of 57.09 swaps/s and 2,711 packets per frame. In the slower windows, the median command-worker cost rose from 44.68 to 58.66 million CPU cycles per frame, the sampled draw cost rose from 2.79 to 3.74 µs per packet, and the vertical-blank wait rose from 5.52 to 11.29 ms per frame. Both draw processing and waiting accounted for additional time at similar draw counts.

Guest-side timing measured the graphics driver's ring-space wait separately from preparing and calling `VdSwap`. The initial unrestricted spawn stayed near 60, with about 12.0 ms in the ring-space wait per frame. In 28 slow windows under a two-CPU restriction, the median rate was 16.22 swaps/s and the wait averaged 54.25 ms per frame. The ring-space wait accounted for most of 287 of 288 Guest frame gaps of at least 40 ms during the restriction. The `VdSwap` import itself averaged under 1 µs per frame. The original Guest code checks a word at the base of its graphics state for ring availability. The SDK writes the primary command-ring read pointer 60 bytes later, so those locations need separate timing.

A follow-up at the unrestricted spawn captured one-second stretches at 31.89 and 33.93 Guest swaps/s. The ring-space wait averaged 22.48 ms/frame in those two stretches, compared with 11.55 ms/frame in 60 faster windows. The Guest clock ran 0.020% and 0.012% faster than wall time in the two slow samples. The graphics worker advanced the ring-space word by executing `EVENT_WRITE_SHD`. Its physical address was 60 bytes before the primary ring read-pointer writeback. Under the two-CPU control, 274 slow waits observed the same value as the instrumented GPU write, and the median delay from write to Guest release was 0.186 ms. Most of the measured wait elapsed before the write arrived.

A test-only wake-up let the render thread wait for that write after sustained polling. Windows could return early or time out after 1 ms, and the Guest continued to check its original ring-space condition on every return. Standalone checks exercised a timeout, an advance before waiting, and a concurrent advance. In the unrestricted comparison, render-thread CPU time fell from 18.65 to 6.56 ms/frame while the measured frame rates were 51.91 and 50.46 swaps/s. Draw counts varied across those unrestricted phases. In the two-CPU comparison at about 2,250 draw packets per frame, render CPU time fell from 20.36 to 4.47 ms/frame, output rose from 42.15 to 48.16 swaps/s, and frame gaps at or above 40 ms fell from 73 to 24 across comparable sampled durations. This wake-up reduced CPU time with measurable pacing gains under the artificial restriction. [Windows address-wait contract](https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitonaddress)

The clean-build comparison generated the ring-space hooks through the Port's normal code generator and checked their placement against the original Guest routines. It removed the command-worker profiling counters and used the launcher’s runtime DLL. Only the Guest render-thread wake-up was enabled for the candidate. The scripted route averaged 53.43 Guest swaps/s on stock and 56.08 with the wake-up. RTSS reached a minimum of 30.07 FPS on stock and 42.68 with the wake-up. The longest Guest gaps were 81.08 and 37.55 ms. An external sampler measured the render thread at 971.9 and 350.2 CPU ms per second over 87 complete intervals per launch. The two routes ran in separate processes with different ambient game activity.

A follow-up sampled one in 64 D3D12 draws to measure the cost by operation. All 93 complete spawn windows reached at least 55 swaps/s, with about 2,709 draw packets per frame. Sampled draws averaged 2.44 µs, including 0.46 µs for bindings, 0.45 µs for vertex memory, and 0.43 µs for texture requests. This supplies a fast-state reference for the instrumented stages.

The same draw sampler reached a heavier unrestricted street view averaging 36.81 worker swaps/s and 36.77 FPS in RTSS. In 55 slow windows, the average was 35.21 swaps/s and 3,666 draws/frame. A sampled draw took 3.19 µs, including 0.67 µs updating bindings, 0.55 µs requesting textures, 0.52 µs preparing vertex memory, and 0.37 µs processing primitives. In 18 faster windows from that view, output averaged 41.21 swaps/s at 3,848 draws/frame, and sampled draw cost fell to 2.79 µs. Binding and texture costs fell by about 0.10 and 0.09 µs per sampled draw, respectively. The slower windows had a lower draw count and higher cost per draw.

A prototype copied contiguous runs of shader float constants in one operation. The standalone check compared the resulting bytes against the per-register implementation for every contiguous mask and 100,000 random masks. In the slow Career comparison at about 2,710 draw packets per frame, sampled float upload time fell from 0.227 to 0.163 µs per draw, while average output was 37.89 swaps/s with stock copies and 36.84 with the prototype. The prototype remained in the diagnostic build.

An independent comparison verified each D3D12 binding model at startup. Bindless output averaged 35.63 swaps/s at 2,791 draw packets per frame in the selected view. Bindful averaged 32.73 swaps/s at 2,707 packets per frame in its selected view. In the slow windows, binding work averaged 0.90 µs per sampled draw with bindless resources and 1.21 µs with bindful resources. The separate launch comparison favored the existing bindless setting on this machine.

Two additional unrestricted runs alternated stock, event wake, the deadline timer, and both timing changes. At about 2,700 draw packets per frame, stock averaged 57.36 swaps/s, event wake 59.34, the deadline timer 58.21, and both 59.90. In the lighter repeat at about 1,620 packets per frame, the rates were 59.74, 60.00, 60.01, and 60.01. The lowest selected one-second window was 51.18 swaps/s. These comparisons measured small gains near 60 Hz and did not enter the earlier low-30s state.

An unrestricted street-view test began the comparison after two consecutive stock samples below 38 swaps/s. The same Career session alternated stock with event wake plus the deadline timer, without moving the skater during the comparison. Stock averaged 33.40 swaps/s across 35 complete windows at about 3,828 draw packets per frame. The combined changes averaged 37.42 swaps/s across 34 windows at about 3,801 packets per frame. Flag wait fell from 12.21 to 9.39 ms per frame, and the worker's mean delay after the flag clear fell from about 1.53 ms to about 0.012 ms. The slowest window improved from 30.01 to 30.98 swaps/s. Neither mode recorded a gap of at least 40 ms in the selected windows. The combined timing changes improved throughput in this natural low-rate state, with occasional low-30s output still present.

A second unrestricted street-view test alternated all four timing modes after two consecutive stock windows below 38 swaps/s. Draw counts stayed within 3,535–3,550 packets per frame. RTSS averaged 36.61 FPS on stock, 38.17 with event wake, 35.57 with the deadline timer, and 40.49 with both. The graphics worker measured 36.91, 37.93, 35.66, and 40.51 swaps/s respectively. Flag wait averaged 10.60, 9.50, 11.35, and 8.30 ms per frame. The number of complete RTSS samples at or below 38 FPS fell from 17 of 24 on stock to 3 of 25 with both changes. The worker still spent about 16 ms of CPU time per frame in each mode.

#### Combined frame pacing

A heavier unrestricted view alternated stock, the Guest render-thread wake-up, the graphics-worker wake-up plus deadline timer, and all three changes. The selected phases processed roughly 4,000 draw packets per frame. Guest output averaged 39.12, 44.49, 47.55, and 50.85 swaps/s respectively, across 26–27 one-second windows per mode. Enabling all three reduced render-thread CPU time from 24.87 to 6.51 ms/frame and ring-space wait from 18.82 to 13.30 ms/frame. The stock windows contained six frame gaps of at least 40 ms. The fully enabled windows contained zero, with a maximum gap of 37.21 ms. RTSS tracked the graphics worker within 0.60 FPS in each phase. This was an instrumented, fixed-view comparison.

A separate clean-build comparison used the real launcher runtime and removed the per-draw and command-worker profiling. The whole scripted route averaged 46.65 Guest swaps/s on stock and 52.05 with all three timing changes. RTSS averaged 46.77 and 52.78 FPS, with 22 and 7 one-second samples at or below 38 FPS out of 73 samples per launch. RTSS still reached 30.07 FPS with the changes. The longest measured Guest gap fell from 65.95 to 38.68 ms. Render CPU use fell from 961.0 to 339.6 CPU ms per second across 87 external samples per launch. The fixed-view comparison above controls for scene differences within one process, while these clean launches verify the combined code without the extra profiling.

A muted follow-up with all three changes confirmed eight pause and resume cycles from the Career spawn by capturing the pause menu and the restored street HUD. The four complete Guest timing windows before pausing ranged from 59.00 to 60.00 swaps/s, with a longest gap of 35.06 ms. The four complete windows after resuming ranged from 56.00 to 59.80 swaps/s, with a longest gap of 39.47 ms. The capture checks menu state and frame delivery. The reported slowdown of gameplay actions remains unresolved. [Gameplay speed report](https://github.com/TheRealestNwah/skate-recomp/issues/119)

The `-ExperimentalFramePacing` build compiled the three changes into the local D3D12 package. The two affected generated Guest functions matched their stock counterparts after removing the three new calls. A muted boot of the packaged executable verified the loaded plugin path, frame submission, and the native Guest ring wait. The regular and experimental builds passed the host test suite.

The experimental plugin also corrects the completion wait for queue operations issued outside a submission. In a bounded standalone D3D12 test, the original condition left the event unarmed even after the fence advanced. The corrected condition arms the event after the successful signal and the wait completes. The measured in-Career hitch samples above logged zero time in the submission-fence wait.

The scripted Career Host loaded the same packaged plugin by SHA-256. At a requested 60 Hz, it averaged 59.82 Guest swaps/s across 11 complete skating windows, with a longest gap of 35.06 ms. RTSS averaged 59.86 FPS across 73 samples and ranged from 58.07 to 60.10 FPS. Loading the Career save still produced a 484 ms swap gap before the street HUD appeared.

A repeat with the same scripted Host and the installed stock plugin averaged 50.81 Guest swaps/s across 11 complete skating windows and had a longest gap of 38.56 ms. RTSS averaged 50.68 FPS across 71 samples, with 12 samples at or below 38 FPS and a minimum of 30.07 FPS. External samples measured 977.8 render CPU milliseconds per second on stock and 288.6 with the packaged plugin. The separate launches used the same route and could have different traffic and camera workloads.

A second 60 Hz run verified the SHA-256 of the rebuilt opt-in package. It averaged 57.94 Guest swaps/s across 11 complete skating windows. RTSS averaged 58.13 FPS across 73 samples and reached 30.04 FPS once. One Guest window contained a 121.61 ms swap gap. The rebuilt package can still encounter both low output and a longer skating hitch.

A longer route with all three pacing changes and graphics-worker instrumentation logged four skating gaps over 40 ms. The largest was 66.41 ms, including 24.03 ms across 30 texture allocation calls and 14.42 ms in graphics wait packets. A second gap spent 20.58 ms across 34 allocation calls. The other two gaps recorded zero texture allocation time and about 15 ms of graphics waits each. The four gaps spent at most 1.67 ms refreshing and presenting the output image in the measured callback. RTSS reached 30.04 FPS and averaged 46.50 over 109 route samples. This trace identified several contributors and captured a different long frame from the 121.61 ms gap.

A later diagnostic route traced the same renderer with direct presentation and then with presentation on the UI thread in a separate launch. The direct run recorded one 15.72 ms `Present` call during skating and reached a 58.43 ms maximum worker gap. In the UI-thread run, a burst of skating gaps reached 210.42 ms. A 177.85 ms gap contained 95.81 ms of empty-ring idle time and 0.91 ms refreshing the display. Windows sampled the Guest render thread ready in five of seven snapshots during that gap, and the graphics worker ready in six of seven. The Guest render thread gained 9.5 million CPU cycles across those samples. This capture identifies periods when the two threads were runnable and waiting for CPU time. [Windows thread state](https://learn.microsoft.com/en-us/dotnet/api/system.diagnostics.threadstate)

Two muted stationary runs restricted the paced Port to four logical CPUs and added one benchmark worker at normal priority on each CPU. Each run alternated the Port process between `Normal` and `AboveNormal` in opposite orders. At roughly 2,400–2,500 draws/frame, the four normal-priority phases produced 12.37–26.03 swaps/s. The graphics worker spent 14.49–48.82 ms/frame waiting for commands to arrive. The four raised-priority phases produced 58.20–58.70 swaps/s, with 0.36–0.53 ms/frame waiting for commands. The two modes recorded a total of 655 and one frame gaps of at least 40 ms, respectively, over about 67–68 seconds of complete windows each. The tests restored process priority and affinity and stopped the benchmark workers. This reproduces command starvation under controlled CPU competition. The source of the runnable delays during unrestricted play remains unverified.

A separate stationary Career run switched the graphics worker's empty-ring wait between the SDK's initial polling loop and immediately waiting on the existing write-pointer event. The run kept process priority fixed and placed two benchmark workers on four logical CPUs shared with the Port. At approximately 2,450 draws per frame, the polling phases delivered 32.76 and 39.54 swaps/s and recorded 98 gaps of at least 40 ms over 29.2 seconds. The immediate-wait phases delivered 30.24 and 36.85 swaps/s and recorded 135 such gaps over 29.4 seconds. The diagnostic verified zero spins during the immediate-wait phases. This change remained in the diagnostic build.

#### D3D12 draw costs

A finer draw probe captured 30.0 processed swaps/s on a muted street pass with 2,891 draw commands/frame. `IssueDraw` took 13.35 ms/frame. Within it, updating shader bindings took 2.64 ms, requesting textures 2.85 ms, checking and requesting vertex buffers 2.09 ms, and processing primitives 1.46 ms. The timings include diagnostic overhead and describe this pass. [Stutter report](https://github.com/TheRealestNwah/skate-recomp/issues/118)

A prototype intersected the shader's used vertex slots with the already-valid slots before iterating. A standalone check agreed with the original selection over 10,000 masks. In a same-process six-interval comparison at about 2,550 draws/frame, stock averaged 27.9–29.6 swaps/s and the prototype averaged 28.4–29.5 swaps/s. Vertex-buffer handling remained roughly 2 ms/frame in both modes. The change stayed in the diagnostic build.

#### SDK Guest clock ordering

A standalone ReXGlue 0.10.1 regression held an earlier Host counter sample until a second thread could update the Guest clock. The stock code counted 60 Guest ticks over 40 Host ticks. Sampling the Host counter after taking the clock mutex counted 40 Guest ticks. A separate 16-thread stress test with the real Windows counter measured a Guest clock 2.4–7.6% fast across five stock windows. The candidate stayed within 0.001% of wall time. The change is a local SDK patch. This isolates a clock race under contention, and the intermittent Career pause slowdown still needs a gameplay reproduction. [Gameplay speed report](https://github.com/TheRealestNwah/skate-recomp/issues/119)

#### Long frames and pipeline creation

An individual-frame probe timed the pipeline queue drain in `EndSubmission`, which an upstream report identified as a source of hitches in another game. The Skate Career route produced four gaps of at least 40 ms with a copied shader cache and four with a fresh Port cache. The longest gap in each set was 50.64 and 52.76 ms, respectively. The longest measured pipeline drain during Career was 0.13 ms. The two worst frames spent 26.19 and 32.05 ms in draw processing and 18.02 and 10.90 ms waiting on Guest memory. The fresh-cache run retained the machine's existing driver cache. [ReXGlue pipeline report](https://github.com/rexglue/rexglue-sdk/issues/557)

Entering Career produced a distinct loading hitch in both runs. The command worker recorded gaps of approximately 532 and 541 ms. Draw processing accounted for over 500 ms in each, and the adjacent timing windows showed most of that draw time in texture requests. [Stutter report](https://github.com/TheRealestNwah/skate-recomp/issues/118)

A follow-up split texture work into D3D12 resource creation, shared-memory residency, and upload command recording. In a 468.40 ms loading frame, texture requests accounted for 412.76 ms. Resource creation took 353.00 ms, residency took 53.49 ms, and upload command recording took 1.80 ms. The worst measured in-Career gap on that run was 47.29 ms and included 0 ms of texture resource creation. This distinguishes the loading hitch from the measured live gameplay gap.

A finer trace recorded a 500.30 ms loading gap. The graphics worker made 578 `CreateCommittedResource` calls for textures during that frame and spent 361.85 ms in those calls. The longest call was 1.50 ms. The next burst made 37 calls totaling 22.55 ms in a 49.73 ms gap. The loading bottleneck accumulated across hundreds of allocations on the graphics worker.

A standalone D3D12 comparison on the same Radeon RX 9070 XT allocated 578 synthetic textures per pass. Across four passes each, committed textures took 308.95–335.77 ms. Placing the same textures in one heap took 13.57–17.90 ms, including the size queries and heap creation. The benchmark measures CPU allocation time in isolation. [D3D12 placed resources](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-createplacedresource)

A muted Career prototype placed eligible textures in 64 MiB heaps with a 512 MiB cap and used committed allocations as a fallback. The matching loading frame issued 2,850 draws and created 578 textures in both runs. The gap fell from 500.30 to 132.77 ms. Four textures used committed allocations in the prototype. Placement and fallback calls together took 17.96 ms, while shared-memory residency still took 56.75 ms. The diagnostic build gives each texture a unique offset and holds each heap until the texture cache is destroyed.

A repeat with the same setup captured 578 texture creations in a 117.09 ms gap with 2,851 draws. Four textures used the committed fallback again. Placement and committed calls took 15.54 ms combined, and shared-memory residency took 48.87 ms.

A reusable version held retired placements until the GPU submission fence completed and coalesced reclaimed ranges. A standalone test passed 150,000 randomized range operations. In matched muted launches with the same plugin and all three pacing changes active, the longest Career loading gap fell from 483.95 ms with committed resources to 150.00 ms with the heap allocator. During subsequent skating, the longest gap fell from 77.86 to 36.88 ms, while RTSS averaged 58.44 FPS with committed resources and 55.44 with the heap allocator. The first allocator-enabled launch hit a Guest access violation during startup. The retry and the committed control entered Career and completed the route. The cause of the startup fault remains unverified.

Eight additional muted boots reached frame submission, four with texture heaps enabled and four with committed textures. A route with lower texture cache limits exercised reuse and exited normally. The Windows D3D12 debug layer was unavailable. A separate GPU check copied and verified distinct texture contents across 20 fence-ordered reuses of the same heap range.

The experimental package exposes this allocator through `-ExperimentalTextureHeaps`. Muted boots of the rebuilt executable verified frame submission and the native ring wake in both allocator modes. The shared-heap allocation path ran when requested.

In a longer rendering trace, committed allocation took 337.50 ms across 612 textures on the largest loading frame. The shared-heap run spent 8.49 ms on 574 placements and 2.32 ms on four committed fallbacks in its main texture burst. Its longest skating gap reached 319.32 ms, including 224.14 ms with an empty command ring, 77.99 ms processing draws, and zero texture allocation time. Another skating gap of 90.29 ms included 57.87 ms refreshing the displayed image. These captures separate the allocation improvement from the remaining command-production and presentation stalls.

Disabling tiled D3D12 shared memory made the plugin allocate a 512 MiB buffer at startup. With texture heaps enabled in both launches, the 578-texture frame spent 44.45 ms preparing shared memory with tiled backing and 59.99 ms with the full buffer. The corresponding frame gaps were 116.67 and 150.96 ms. RTSS averaged 52.93 and 50.44 FPS on the two separately launched routes. The full-buffer run recorded fewer long skating gaps and more low-FPS samples, with different camera and ambient workloads. This option showed no repeatable loading benefit in the sampled comparison.

A finer shared-memory trace measured 47.13 ms in the 578-texture loading frame. Upload buffer requests took 17.66 ms, copying Guest bytes into those buffers took 15.18 ms, and allocating sparse backing took 10.33 ms. Marking and protecting pages took 3.61 ms. Merging and scanning the ranges and acquiring the lock together accounted for 0.09 ms.

A diagnostic pair increased upload pages from the stock 2 MiB to 16 MiB. Both loading frames issued 2,850 draws and created 578 textures. Time obtaining upload buffers fell from 37.87 to 26.10 ms and the frame gap fell from 133.39 to 117.08 ms. Copying Guest bytes took about 13.6 ms in both runs. Reversing the order gave gaps of 99.91 ms at 16 MiB and 150.34 ms at 2 MiB. The 2 MiB repeat created 22 more textures and issued 111 fewer draws during that frame.

A four-size sweep created 578 textures per loading frame. The 2, 4, 8, and 16 MiB runs recorded gaps of 133.46, 117.36, 166.17, and 99.86 ms, respectively. Buffer requests accounted for 21.17, 8.42, 50.46, and 6.63 ms. All four copied Guest bytes in 15.36–16.60 ms. The 2 MiB run processed 2,676 draws in that frame and the other three processed 2,850. The largest sampled shared GPU memory total for the Port after loading ranged from 102.2 MiB at 2 MiB pages to 122.2 MiB at 16 MiB pages. The Windows counter covers the entire process.

Two repeat launches per size counted upload-buffer resources directly. All six loading frames created 578 textures, copied 50.75 MiB of texture data, and issued 2,850–2,851 draws. The two 2 MiB runs each created 24 upload buffers during the loading frame, spent 18.53–29.48 ms obtaining upload buffers, and recorded 150.11–183.66 ms frame gaps. The 4 MiB runs each created 12 buffers and recorded 183.82–184.71 ms gaps. The 16 MiB runs each created three buffers, spent 8.78–9.12 ms obtaining upload buffers, and recorded 116.91–133.65 ms gaps. The upload pool peaked at 60 MiB with 2 MiB pages and 80 MiB with 16 MiB pages. The experimental texture-heap option selects 16 MiB pages for D3D12 shared memory.

Two additional 32 MiB runs allocated three upload pages (96 MiB capacity). One matched the 578-texture workload and spent 7.81 ms obtaining upload buffers, with a 133.83 ms frame gap. The other loaded 597 textures and spent 24.22 ms on requests, with a 150.48 ms gap. The matched 32 MiB request time overlaps the 16 MiB measurements, while its pool is larger and its frame gap did not improve on the 16 MiB runs. This supports retaining 16 MiB as the opt-in size.

#### Vertical-blank scheduling

A Windows prototype switched between the SDK's 1 ms polling sleep and a high-resolution timer set to the next Guest vertical-blank deadline. Both modes requested the same 60 Hz rate. In the first muted same-view run, stock recorded 117 intervals over 18 ms out of 1,621 vertical blanks. The deadline timer recorded three out of 1,640. Command-worker output ranges were 39.3–45.2 swaps/s with stock timing and 42.7–47.2 swaps/s with the timer. None of those measured windows contained a 40 ms swap gap. This improves the regularity of vertical-blank delivery in the test. Its effect on visible hitches remains unverified. [ReXGlue pacing report](https://github.com/rexglue/rexglue-sdk/issues/556)

A second run reversed the order and reproduced the interrupt result. Stock had 138 intervals over 18 ms out of 1,560 vertical blanks. The deadline timer had one out of 1,638. Command-worker output overlapped between modes. The last stock interval also had heavier draw processing and 10 swap gaps of at least 40 ms, so the comparison cannot isolate the timer's effect on those gaps.

A clean candidate converted the deadline using the SDK's Guest-to-Host tick ratio and used the original sleep if the high-resolution timer was unavailable. The muted Career route recorded 13 vertical-blank intervals over 18 ms out of 6,125, and one over 20 ms. Output still fell to 31.0 swaps/s in a view processing about 4,030 draws/frame. The same run recorded four output gaps of at least 40 ms. The Windows SDK patch improves interrupt regularity in the measured scenes. Frame production and occasional long frames remain separate concerns.

#### 30 Hz Guest display experiment

EA Black Box's executive producer described 60 Hz physics and doubled rendering throughput for the sequel, which suggests a 30 FPS render target for the original game. The Port experiment changes the Guest video mode and vertical-blank rate directly. The corrected launch verified 30 Hz in both the process arguments and the interrupt trace. It held 29.4–30.0 Guest swaps/s during the sampled Career sequence. Captured ollies before and after pause were broadly similar to a faster 60 Hz control using the retail input worker rate. The first attempted 30 Hz run still passed 60 Hz and was used only as a second 60 Hz control. These captures cover one trick and one Career corridor. [Producer interview](https://www.gamedeveloper.com/design/new-tricks-scott-blackwood-talks-i-skate-i-and-i-skate-2-i-)

A longer street pass at 30 Hz measured 29.9, 30.0 and 30.0 swaps/s during the three push sequences. The last sequence processed about 3,362 draw commands/frame. Two sequences had no swap gaps of 40 ms or longer. The first had one 67 ms gap. The launcher exposes this mode as an experimental opt-in. [Launcher change](https://github.com/TheRealestNwah/skate-recomp/pull/122)

A muted comparison at an explicitly requested 30 Hz used the same scripted route as the 60 Hz stock and combined timing tests. Stock averaged 30.00 Guest swaps/s and the event wake plus deadline timer averaged 29.98, across 11 complete five-second windows per mode. Their longest gaps were 64.41 ms and 67.23 ms, respectively. RTSS ranges were 29.61–30.07 FPS on stock and 29.16–30.07 with both changes. The optional 30 Hz mode continued to run with the combined timing changes, and the sampled route still had occasional missed frame intervals.

A 30 Hz repeat verified the opt-in plugin's SHA-256 and used all three pacing changes. It averaged 30.00 Guest swaps/s across ten complete skating windows. Two windows had a gap of at least 40 ms, with a maximum of 65.91 ms. RTSS averaged 30.01 FPS across 70 samples and ranged from 29.53 to 30.07 FPS.

A later 30 Hz and 60 Hz display comparison targeted only the Port process, with GPU-duration and input tracing disabled. PresentMon reported lost Windows trace events in both runs and produced no usable display CSV. The pacing figures above describe Guest and command-worker output.

#### D3D12 presents during achievement notifications

A diagnostic runtime assigned a sequence number to each Guest frontbuffer update and counted successful D3D12 present calls using that sequence number. The muted 60 Hz Career run queued two synthetic achievement notifications from a stationary view. One 1.004-second window registered 138 presents, 31 with a new Guest image and 107 with the previous image. In another window, the 143 presents included 37 new images and 106 repeats. Complete windows before and after the notifications had zero repeated presents and averaged 29.6 and 31.9 new images/s, respectively. The probe measures timing at the successful API call. [Stutter report](https://github.com/TheRealestNwah/skate-recomp/issues/118)

A local SDK prototype skipped redundant UI paints within 33 ms of the last successful paint when the overlay still had the same Guest image. New Guest images and forced window paints remained eligible for the next UI tick. A second stationary Career run recorded two repeat presents across the nine complete one-second windows that included UI paints. The two runs reached different frame production rates before the notifications appeared. This confirms the prototype can suppress redundant presents, with game frame production and display latency still needing matched measurements.

A same-process test alternated stock painting and the 33 ms prototype across four notification batches. The stationary view processed 2,700–2,713 draw commands per frame. Both stock batches recorded 160 presents/s, of which 55–58 carried new Guest images. Both prototype batches recorded zero repeats in the measured windows. The longest gap between presents of new images rose from 37.55 ms on stock to 48.05 ms with the prototype. Command-worker output had zero gaps of at least 40 ms in either mode. The prototype remains local.

A second prototype let an already-available new Guest image bypass the UI refresh wait, while keeping the 33 ms limit on repeats. In a four-batch comparison it again recorded zero repeats. The two prototype batches had longest new-image gaps of 37.97 and 37.37 ms. The stock batches recorded 39.08 and 39.50 ms. Fresh presentation rates varied across the batches, from 43.4–51.6/s with the prototype and 49.5–56.0/s on stock. The matched notification trigger reproduces the presentation-count effect independently of fresh-image throughput.

A further run read the active Port's RTSS statistics directly alongside the D3D12 trace. The machine's RTSS global profile set a 160 FPS limit. During the notification burst, RTSS reported 159.45–160.59 FPS, and complete notification windows counted 36–45 new Guest images and 115–124 repeated images per second. After the notifications cleared, RTSS returned to the 40s with zero repeat presents and briefly reached 34.92 FPS in the same Career session. The patched Guest clock continued to match wall time at the logged precision. The sampler used read-only access to the RTSS statistics.

A muted action-timing run repeated the notifications while separately recording the Port window. RTSS peaked at 160.59 FPS with 15 samples at or above 150 FPS. Complete one-second notification windows commonly held 59–60 new Guest images and 100–101 repeats. Two scripted ollies during the notification period reached their landing poses around 1.10 seconds after the flick.

#### Repeated pause and resume at 60 Hz

A muted run with the retail input rate repeated five pairs of scripted ollies in the station corridor. Each pair bracketed a pause confirmed by a menu capture. The takeoff and airborne poses had similar timing before and after resume through about 0.94 seconds after the flick. A pedestrian partly obscured one pre-pause capture sequence. The sampled poses showed no consistent slowdown after resume, and five-second Guest clock samples across the test stayed within 0.007% of wall time. The captures end before a precise landing comparison and leave the intermittent slowdown in the original playtest unresolved. [Gameplay speed report](https://github.com/TheRealestNwah/skate-recomp/issues/119)

A second comparison built two runtime DLLs from the same pinned SDK source. The candidate changed only when the Host counter is sampled in the Guest clock update. Both muted runs verified which DLL the Port loaded and completed five captured pause/resume pairs. The 12 control samples during the pairs ranged from 1.000077 to 1.006221 times wall clock. All 13 candidate ratios rounded to 1.000000 at six decimal places. The extended captures reached the landing at roughly 1.1 seconds in both runs, with no consistent shift after resume. The measured clock race occurs in Career and the candidate corrects the sampled drift. The larger reported speed changes remain unreproduced in this test. [Gameplay speed report](https://github.com/TheRealestNwah/skate-recomp/issues/119)

A further muted run sampled stock timing before, during, and after a two-CPU restriction. The restriction reduced Guest output to roughly 12–22 swaps/s, and complete clock samples in the low period measured 1.015–1.030 times wall time. It also made frontbuffer readbacks take up to 287 ms, and obstacles or camera movement obscured several landing captures. The run did not verify the pause menu under load. The measurements show modest Guest clock drift during the induced slowdown, with action and pause timing still unresolved for that condition. [Gameplay speed report](https://github.com/TheRealestNwah/skate-recomp/issues/119)

A follow-up captured the Port window through Windows Graphics Capture in a separate process to avoid readbacks on the game's rendering thread during the action samples. Unrestricted output reached roughly 43–49 swaps/s, and the sampled ollies landed near 1.10 seconds after the stick flick. Under a temporary two-CPU restriction output fell to 15.9–23.1 swaps/s during the actions. One verified ollie was still in its early lift pose at 1.45 seconds before any pause. After a verified pause and resume, two other sampled ollies remained in the air at 1.45–1.48 seconds. Restoring CPU access returned the sampled landings to about 1.10 seconds. Several restricted trials overlapped or lacked a verified input event. The exact simulation rate and the unforced 30 FPS case remain open. [Windows capture frame timing](https://learn.microsoft.com/en-us/uwp/api/windows.graphics.capture.direct3d11captureframe) · [Gameplay speed report](https://github.com/TheRealestNwah/skate-recomp/issues/119)

Three additional muted runs retained full CPU access and recorded two ollies on each side of a verified pause and resume. The 60 Hz control produced roughly 56–60 Guest swaps/s. A graphics-worker-only limit produced 29.0–29.1 swaps/s while the Guest still requested 60 Hz. A third run requested a 30 Hz Guest mode and produced about 30.0 swaps/s. The sampled ollies in all three runs reached their landing pose around 1.10 seconds after the stick flick. This test isolates low graphics output from the artificial CPU pressure that delayed visible actions. [Gameplay speed report](https://github.com/TheRealestNwah/skate-recomp/issues/119)

A further pair used the same scripted Host and pinned runtime with the installed stock renderer and the experimental pacing renderer. The driver logged when the Guest first read each stick flick. Under the same temporary two-CPU mask, the stock renderer delivered the four measured flicks in 16–37 ms, yet three visible landings took about 2.1–2.6 seconds after delivery and one remained airborne near 2.9 seconds. The pacing build delivered the four flicks in 0–13 ms and reached the sampled landing poses in about 1.1–1.45 seconds. Stock Guest clock samples ran up to about 2.5% fast under the constraint. The faster Host renderer reduced the visible slowdown in this artificial workload. The separate launches had different camera views and ambient draws. [Gameplay speed report](https://github.com/TheRealestNwah/skate-recomp/issues/119)

A run with the official stock ROV plugin kept full processor access and naturally produced consecutive 32.3 and 36.4 Guest swaps/s windows at the Career spawn. The action samples before and after a verified pause landed around 1.10 seconds after input delivery. During the sampled actions, five-second Guest output windows ranged from about 36.7 to 39.1 swaps/s and the stock Guest clock stayed within 0.017% of wall time. A stock RTV search held 58.4–60.0 across the two sampled windows at each of nine viewpoints. The reported 2× gameplay speed remains unreproduced. [Gameplay speed report](https://github.com/TheRealestNwah/skate-recomp/issues/119)

ReXGlue v0.11.0 still samples the Host counter before obtaining the mutex in `LockAndUpdateGuestClock`. A standalone reproduction using that function again counted 60 Guest ticks over 40 Host ticks and counted 40 after moving the sample under the lock. The comparison used an injected counter and the current Port still pins v0.10.1. [ReXGlue v0.11.0 clock source](https://github.com/rexglue/rexglue-sdk/blob/v0.11.0/src/base/clock.cpp)

#### Reported Guest resolution

The 60 Hz control and a muted run that requested a 960×540 Guest video mode both produced 1,280×720 Career frontbuffer captures. The SDK launch arguments were verified. The title retained its frontbuffer dimensions, so this setting comparison provides no measurement of rendering the scene at a lower internal resolution.

#### November 21 M4 Career performance gate recheck

A bounded synthetic-controller route resumed a copied Career save on the same Windows machine, used RTV at a requested 60 Hz, muted audio, waited for the Suburbs HUD, sampled a stationary spawn, and then ran eight scripted skating segments. Each launch used a fresh copy of the test save. The control used the stock Xenos plugin. The candidate used the packaged experimental frame-pacing plugin with the vertical-blank deadline, graphics-worker wake, and Guest ring wake enabled. Plugin hashes were checked before candidate route measurements. Guest swaps came from the Port's five-second `-FrameTiming` windows; the captures did not use a physical controller.

| Build | Stationary Guest swaps/s | Movement route mean | Lowest five-second movement window | Worst movement-window p95 | Longest movement-window gap |
| --- | ---: | ---: | ---: | ---: | ---: |
| Stock control | 52.5 | 54.7 | 41.3 | 49 ms | 52 ms |
| Experimental pacing, run 1 | 58.4 | 58.4 | 45.6 | 35 ms | 36 ms |
| Experimental pacing, run 2 | 58.3 | 59.6 | 58.6 | 20 ms | 36 ms |
| Experimental pacing, run 3 | 59.8 | 58.9 | 50.4 | 35 ms | 38 ms |
| Experimental pacing, run 4 | 59.2 | 59.6 | 57.3 | 21 ms | 35 ms |
| Experimental pacing, run 5 | 57.9 | 58.8 | 48.4 | 35 ms | 37 ms |
| Experimental pacing, run 6 | 59.4 | 58.1 | 49.3 | 34 ms | 53 ms |

The pacing candidate averaged 58.1–59.6 swaps/s over the movement route, compared with 54.7 on stock. Two of its six runs kept every stationary and movement window at or above the FPS thresholds and with p95 intervals at or below the provisional 25 ms threshold. Runs 1, 3, 5 and 6 each had a movement window above the p95 threshold; runs 1, 5 and 6 also fell below 50 swaps/s in a movement window. Run 5 additionally had a stationary window at p95 33 ms. No sampled movement window had a gap above 100 ms. A separate test raised the Port process to Above Normal priority; its stationary windows fell to 54.8 and 48.2 swaps/s and a movement window fell to 32.3, so process priority is not a useful fix. The pacing candidate improves average output but does not pass the current repeatability gate. The 60 Hz stock control also fell below the gate. These are short synthetic routes, not Career playthrough results; physical controller response and input-to-action timing remain unverified.

Two one-run tuning trials changed the Guest ring poll count before the event wait from 256 to 64 and 1,024. At 64, stationary windows averaged 48.4 swaps/s and the movement route averaged 56.7, with a minimum window of 54.2. At 1,024, stationary windows averaged 58.5 but the movement route averaged 52.7 and fell to 42.3 in one window. Both variants had several p95 windows above 25 ms. Neither improved on the default 256-poll candidate, which remains unchanged.

#### GPU load during a slow Career route

A further muted 60 Hz run used the same copied Career save, RTV path, warmed cache and eight-segment synthetic route. The loaded Xenos plugin hash matched the packaged frame-pacing candidate. Windows per-process GPU Engine counters produced 49 one-second sample sets. The 3D engine averaged 34.1% over the movement route and reached only 26–32% in several windows where Guest output fell to 32–43 swaps/s and p95 intervals rose to 35–36 ms. The engine did not approach saturation during the slow tail, so reducing graphics workload is not currently supported as the next fix. The swap-reporting thread also remained below 45% of one core in these windows. A focused ETW sample of the packaged candidate, described below, found about 90% of its process CPU samples on the Guest main and render threads, with kernel wait code as the sampled leaf in a minority of stacks. Keep RTV, the ring poll count, and the pacing settings fixed while isolating Guest draw and Host D3D12 stage costs on the same route phases.

Two local diagnostic builds then profiled draw packets per route phase and alternated the Guest vblank-gate wait between the existing sleep and an event wait, reversing the phase order in the second run. In both runs, phases with about 1,300–2,300 draw packets/frame generally held 58–60 Guest swaps/s. Phases above roughly 2,500 packets/frame varied widely; the slowest phases processed about 3,000–3,400 packets/frame and fell to about 42 swaps/s. The profiling wrapper measures one draw packet in 64; scaling that sample gives an estimated 385–439 ms/s spent in draw execution during several high-packet phases. The estimate is not a direct total and the diagnostic plugin differs from the packaged candidate. Event waiting did not consistently remove the slow phases after reversing the order. Together with the separate GPU counter run, the evidence points to CPU-side D3D12 draw/command work at complex views, while leaving the specific expensive draw stage unconfirmed. A prior `IssueDraw` breakdown found shader binding, texture requests, and vertex-buffer handling as the largest measured stages; the vertex-slot mask prototype did not improve route performance. Static inspection ruled out repeated shader microcode analysis as a likely cost: `AnalyzeUcode` returns immediately once each shader has been analyzed. An earlier contiguous float-constant upload prototype reduced that substage's sampled cost but did not improve route output, and a separate binding comparison favored the existing bindless path. No measured code change currently improves the high-complexity route.

In a 75-second heavy fixed-view window from the draw-stage probe, the 1-in-64 sampler recorded 159,622 draws. Scaling its weighted stage durations by the sampling rate estimated 420 ms of draw CPU time per wall-clock second. Binding, texture requests, and vertex-memory handling accounted for about 88, 71, and 69 ms/s, respectively; no one stage dominated. The same diagnostic window averaged 36.6 Guest swaps/s across 15 five-second windows. These estimates reinforce the mixed-stage result and are too intrusive to serve as a performance acceptance run.

#### Lower-rate nested binding and no-device route recheck

The ignored low-rate D3D12 draw probe was extended with nested timers around sampler-parameter derivation and active texture-key validation. During a 75-second fixed-view capture, it sampled 5,935 draws. Sampler parameter derivation averaged 0.273 µs per sampled draw and texture-key validation 0.041 µs, compared with 0.893 µs for the full binding stage. This view averaged about 2,700 draws per frame, below the earlier 3,600-draw slow view, so the numbers should not be generalized to the most complex scenes. Texture-key validation is too small here to justify caching; sampler derivation is a possible sub-millisecond-per-frame saving at this draw count, but no cache prototype is warranted without a trace from the slowest view and an end-to-end gain target. An additional 75-second diagnostic capture held a low-output spawn view at 2,713 draws per frame. In its 20 windows at or below 38 swaps/s, output averaged 30.74 swaps/s. The sampled draw stages averaged 0.290 us for sampler parameter derivation and 0.041 us for texture-key validation, close to the earlier values. The route did not reach the 3,300-draw threshold for the intended complex-view sample, so it is not evidence about the slowest scene. These small, stable substage costs do not explain the low output in this fixed view and do not justify a cache prototype.

A second draw-stage capture searched six scripted street views for at least 3,300 draws per frame; none exceeded about 2,750. In the final 75-second fixed view, frame-rate bands processed nearly identical draw counts, while sampled stage costs increased in the slower bands:

| Guest rate band | Windows | Mean swaps/s | Draws/frame | Total us/draw | Binding us/draw | Texture us/draw | Sampler derivation us/draw |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| At least 55 swaps/s | 44 | 59.66 | 2,721 | 2.759 | 0.774 | 0.378 | 0.255 |
| 38-55 swaps/s | 24 | 45.48 | 2,719 | 3.196 | 0.877 | 0.459 | 0.263 |
| At most 38 swaps/s | 5 | 33.06 | 2,721 | 3.418 | 1.004 | 0.523 | 0.273 |

These 1-in-64 sampled diagnostics associate lower output with higher CPU time across several draw stages at a stable draw count. Binding and texture costs changed most; sampler derivation stayed nearly flat. The trace does not isolate why those stage costs changed, and its instrumentation makes the numbers unsuitable for acceptance measurements.

At the time of these initial silent-driver runs, the default SDL playback endpoint was unavailable. A later November 22 session opened the default Windows WASAPI endpoint successfully; see the normal-SDL route results below. A temporary, uncommitted diagnostic audio driver discarded samples and paced Guest callbacks at the SDK's 256-sample / 48 kHz cadence, allowing three independent synthetic Career routes to complete with the packaged 60 Hz RTV pacing plugin. The three movement routes averaged 59.64–59.93 Guest swaps/s; every measured five-second window stayed at or above 58.2 swaps/s, p95 was at most 19 ms, and the longest recorded gap was at most 35.4 ms. The stationary windows also stayed above 58 swaps/s. Later repeat runs did not reproduce that performance (see the November 22 paced-silent follow-up). The initial three passes therefore do not establish repeatable readiness for the maintainer's full manual Career playthrough. These routes do not count toward the formal gate: the prior normal-SDL recheck had two passing runs out of six, and the paced silent driver does not reproduce an actual Windows audio device or its scheduling interactions.

#### November 22 forced DirectSound Career route recheck

To test the SDL-backed path despite the unavailable default playback endpoint, two additional synthetic Career routes forced SDL to use DirectSound on the connected playback device. Both used the packaged 60 Hz pacing plugin, RTV at 1280�720, the same copied Career save, and the scripted eight-segment route. Game audio was muted. The route harness also sampled GPU, render-thread, and PresentMon counters, so these runs are diagnostic and are not formal acceptance measurements.

| Run | Stationary Guest swaps/s | Movement route mean | Lowest five-second movement window | Worst movement-window p95 | Longest movement-window gap |
| --- | ---: | ---: | ---: | ---: | ---: |
| DirectSound 1 | 39.5�44.5 | 32.6 | 25.2 | 88 ms | 226 ms |
| DirectSound 2 | 29.4�30.3 | 37.6 | 24.3 | 84 ms | 229 ms |
| DirectSound 3, paced plugin, no auxiliary samplers | 33.5�45.4 | 49.1 | 41.0 | 41 ms | 313 ms |
| DirectSound 4, stock plugin, no auxiliary samplers | 30.6�30.7 | 33.8 | 29.8 | 58 ms | 122 ms |
| DirectSound 5, paced plugin, thread-name sampler | 44.8�51.2 | 49.8 | 32.4 | 54 ms | 269 ms |
| DirectSound 6, paced plugin, 1280x720 window | 49.6�51.9 | 46.9 | 27.9 | 49 ms | 185 ms |

The first three paced-plugin runs missed the provisional movement thresholds, and the first two had repeated movement windows with p95 intervals above 25 ms. The third repeated the same route without GPU, CPU, or PresentMon sampling; it still missed the 50 swaps/s route mean and 25 ms p95 thresholds. A fourth no-sampler control used the stock Xenos plugin: in this single matched comparison the packaged pacing plugin averaged 49.1 swaps/s versus 33.8 for stock, though both missed the gate. A fifth paced-plugin run used a lightweight thread-name sampler and averaged 49.8 swaps/s, with a worst p95 of 54 ms and a 269 ms longest gap. In its movement phases, the busiest `GPU Commands` thread used 52�67% of one logical core; `Main XThread` used 31�42%, `render_thread` 30�37%, and `RWAudioCore Dac` 10�22%. No sustained single-thread saturation was visible; this sample does not establish a single-thread CPU bottleneck. A sixth uninstrumented-by-CPU-counters run used a 1280x720 window on the same route; it averaged 46.9 swaps/s, with a worst p95 of 49 ms and a longest gap of 185 ms. The low result persists at the M4 target resolution, so fullscreen scaling does not explain it. The results show that acceptable performance with the forced DirectSound backend is not established. The captures do not isolate the cause: game audio was muted, two runs used auxiliary samplers, and the sample is small. The formal performance gate remains open. The explicit-device SDL callback prototype was discarded after it crashed before Career and contributes no performance evidence.

#### Sampled CPU stacks from the packaged candidate

A focused PerfView ETW capture sampled the packaged frame-pacing candidate during the eight-phase synthetic Career route. The trace contains 82,991 CPU sample events, reports zero lost events, and attributes 5,819 samples to `skateport.exe` (PID 27988). Of those process samples, 2,693 (46.3%) were on the Guest main XThread, 2,559 (44.0%) on `render_thread`, and 558 (9.6%) on a Job Manager worker. The remaining threads account for nine samples. Most sampled stacks include recompiled Guest code; the most common individual Guest entry accounts for 139 samples (2.4%), followed by 128 (2.2%). This does not point to one isolated Guest function dominating CPU time. Kernel wait code appears as the sampled leaf in a minority of stacks.

The Release executable has no debug symbols. A linker map associates the sampled addresses with generated `sub_XXXXXXXX` Guest function entries, but those names are guest addresses rather than semantic function names. The trace therefore narrows the activity to the Guest main and render threads without identifying which Host draw substage is expensive. The per-second thread-time counter for `render_thread` used 25.8–31.6% of one logical core across the route phases; the last three movement phases averaged 52.0, 57.0, and 54.5 RTSS FPS while the render thread used 30.8%, 31.6%, and 29.1%. This diagnostic run does not show continuous CPU saturation on `render_thread` during its slowest phases, so the Guest main thread and synchronization remain candidates alongside draw-stage cost. The instrumented route also fell to about 46.9 Guest swaps/s in several movement windows and reached p95 intervals up to 35 ms; profiling overhead makes this a diagnostic capture, not an acceptance run. The nested binding timings and later three-run silent-audio recheck are described above. The current candidate is not ready for the full manual Career playthrough: the later paced-silent routes were not repeatable, and subsequent normal SDL route measurements also missed the performance gate.

#### November 22 paced-silent route follow-up

After the DirectSound captures, two more synthetic Career routes used the same packaged pacing plugin, paced silent-audio diagnostic driver, copied save, `Clock2` pacing configuration, and eight-segment script. Both ran without auxiliary samplers. The previously passing silent-driver results did not repeat:

| Run | Movement route mean | Lowest five-second movement window | Worst movement-window p95 | Longest movement-window gap |
| --- | ---: | ---: | ---: | ---: |
| Paced silent, no sampler 1 | 35.0 | 26.5 | 86 ms | 243 ms |
| Paced silent, no sampler 2 | 52.0 | 35.3 | 51 ms | 172 ms |

Both routes missed at least one provisional movement threshold. A buffered thread-name sample between these runs also missed the movement rate threshold, averaging 42.6 swaps/s; it is diagnostic only. These captures show that the earlier three passing silent-driver routes do not establish repeatable Career performance, even with audio callbacks paced and discarded. The cause of the large run-to-run change remains unknown. Later default-backend measurements are now available and also fail the formal gate. The packaged pacing candidate cannot currently be called playthrough-ready. Keep the full Career playthrough on hold until repeatable routes meet the M4 thresholds.

#### November 22 default SDL WASAPI Career route recheck

The default SDL backend opened `Headphones (BTD 600)` through WASAPI at two channels and 48 kHz. Three independent muted synthetic Career routes used the packaged 60 Hz pacing plugin, RTV at 1280x720 windowed, the same copied save, `Clock2` pacing configuration, and the eight-segment input script. No auxiliary performance samplers ran. The game audio was muted, but the SDL playback device and callback were active.

| Run | Stationary mean | Movement route mean | Lowest five-second movement window | Worst movement-window p95 | Longest movement-window gap |
| --- | ---: | ---: | ---: | ---: | ---: |
| WASAPI windowed 1 | 40.8 | 47.4 | 33.6 | 52 ms | 127 ms |
| WASAPI windowed 2 | 47.7 | 46.8 | 28.2 | 66 ms | 265 ms |
| WASAPI windowed 3 | 49.5 | 47.5 | 28.6 | 53 ms | 263 ms |

All three stationary samples averaged below the required 55 swaps/s. All three movement routes missed the provisional p95 and gap limits; two also averaged below 50 swaps/s. The two additional fullscreen WASAPI captures also missed the p95 and gap limits, with means of 51.8 and 47.4 swaps/s. This resolves the earlier lack of an available default endpoint for measurement, and confirms the formal Career performance gate fails on the real SDL playback path at the M4 target resolution. Audio was muted during measurement, so audible playback behavior still needs the manual check. Keep the full Career playthrough on hold until repeatable default-backend routes meet the M4 thresholds. Two default-WASAPI `Clock1` controls used the same windowed target and pacing plugin but disabled the experimental Host wake overrides. Their stationary means were 51.7 and 50.2 swaps/s. Their movement means were 49.5 and 51.0 swaps/s, worst p95s were 57 and 43 ms, and longest gaps were 292 and 212 ms. Both missed the stationary threshold and the p95 and gap limits. Disabling the `Clock2` wake changes did not restore passing performance, but these two runs do not establish the cause of the drop.

#### Additional default WASAPI route repeats

Two further windowed WASAPI routes repeated the `Clock2` candidate with the same packaged plugin, copied save, 1280x720 target, and eight-segment script. A matched `Clock1` control disabled all three experimental wake changes. The test process explicitly passed the harness environment to the Port, and all three runs completed without auxiliary samplers.

| Run | Stationary mean | Movement route mean | Lowest five-second movement window | Worst movement-window p95 | Longest movement-window gap |
| --- | ---: | ---: | ---: | ---: | ---: |
| Clock2 repeat 1 | 35.7 | 50.7 | 32.8 | 43 ms | 145 ms |
| Clock2 repeat 2 | 51.6 | 48.7 | 30.6 | 46 ms | 220 ms |
| Clock1 control | 50.9 | 46.9 | 35.3 | 46 ms | 172 ms |

All three routes failed at least one pacing limit, and the second `Clock2` repeat and `Clock1` control also fell below the 50 swaps/s movement-average threshold. The control was close to the candidate repeats in movement rate, p95, and longest gap. This comparison shows no repeatable route-level benefit from the wake changes; route-to-route variation remains substantial, so it does not isolate the cause or change the M4 readiness decision.

#### November 23-24 experimental candidate and stock-plugin controls

Three independent muted synthetic Career routes repeated the packaged experimental 60 Hz pacing build on the default two-channel WASAPI endpoint at 48 kHz. A fourth matched candidate route (`recheck57`) followed the later stock-plugin repeats. Each used RTV at 1280x720 windowed, a fresh copy of the same save, the `Clock2` settings, and the eight-segment input script. No auxiliary samplers ran. The plugin and runtime SHA-256 hashes matched across all four candidate launches; recheck57 loaded plugin SHA-256 `76CD5BF2B3D6286365BC10F5D153642F091F5028A028E70B2A0C8C059A3D3DF7` and runtime SHA-256 `BD8E1F6782B19151D09B3638ECA53D9C8C1F268D32AF7A57DCF64D1B6307A982`. Each route produced two complete stationary timing windows and 19 movement windows.

| Run | Stationary mean | Movement route mean | Lowest five-second movement window | Worst movement-window p95 | Longest movement-window gap |
| --- | ---: | ---: | ---: | ---: | ---: |
| WASAPI candidate repeat 1 | 60.0 | 59.99 | 59.8 | 18 ms | 33.20 ms |
| WASAPI candidate repeat 2 | 60.0 | 59.98 | 59.8 | 18 ms | 33.86 ms |
| WASAPI candidate repeat 3 | 60.0 | 59.96 | 59.8 | 18 ms | 33.63 ms |
| WASAPI candidate recheck57 | 59.9 | 59.93 | 59.2 | 19 ms | 50.34 ms |

All four routes in this earlier candidate set met the provisional M4 thresholds. Four matched stock-plugin routes (`recheck50`, `recheck54`, `recheck55`, and `recheck56`) used the same scripted harness, windowed 1280x720 RTV target, default WASAPI endpoint, copied Career save, and eight-segment route. Each loaded the stock Xenos plugin with SHA-256 `359BCA690BEA07C5CF9BA2F045132B394A1ACC7B10901BDBCA3F3389A052DA43`, matching the current `build/port/rexgpu-xenos.dll`.

| Run | Stationary mean | Movement route mean | Lowest five-second movement window | Worst movement-window p95 | Longest movement-window gap |
| --- | ---: | ---: | ---: | ---: | ---: |
| Stock WASAPI recheck50 | 52.9 | 55.42 | 38.8 | 35 ms | 49.38 ms |
| Stock WASAPI recheck54 | 59.9 | 59.57 | 57.4 | 20 ms | 54.9 ms |
| Stock WASAPI recheck55 | 58.3 | 56.79 | 42.3 | 36 ms | 169.06 ms |
| Stock WASAPI recheck56 | 59.5 | 59.47 | 56.8 | 31 ms | 49.7 ms |

Only one of the four stock runs met every threshold in this earlier comparison, while all four candidate routes in that set passed. The default-build performance gate therefore fails this control set; stock performance varied substantially across otherwise matched routes. The candidate-stock-candidate sequence from recheck50/51 still shows the candidate passing on both sides of the first stock miss, but neither it nor the later stock runs explains the large session-to-session variation. The November 22 low routes also remain unexplained. Audio was muted and input was synthetic; audible playback and physical controller checks remain separate M4 items.


#### November 24 frame-pacing wake-flag ablation

The experimental pacing executable was measured with each wake flag enabled alone and in pairs. All routes used the same muted synthetic Career harness, the default WASAPI endpoint, windowed RTV at 1280x720, a copied save, and the eight-segment input script. Each run had three stationary windows and 19 movement windows. `Graphics-worker wake` is `SKATE_LOCAL_VBLANK_EVENT`; `VBlank deadline` is `SKATE_LOCAL_VBLANK_DEADLINE`; `Guest ring wake` is `SKATE_LOCAL_GUEST_RING_WAKE`. Unlisted flags were set to zero. No auxiliary samplers ran.

| Enabled flags | Stationary mean | Movement mean | Lowest five-second movement window | Worst movement-window p95 | Longest movement-window gap |
| --- | ---: | ---: | ---: | ---: | ---: |
| Graphics-worker wake | 56.47 | 57.16 | 41.9 | 35 ms | 66.91 ms |
| VBlank deadline | 60.0 | 58.48 | 34.8 | 36 ms | 51.23 ms |
| Guest ring wake | 56.2 | 59.64 | 57.8 | 19 ms | 48.83 ms |
| Graphics-worker wake + deadline | 53.47 | 57.55 | 46.9 | 34 ms | 249.84 ms |
| Graphics-worker wake + Guest ring wake | 56.2 | 52.75 | 32.7 | 35 ms | 66.81 ms |
| VBlank deadline + Guest ring wake | 52.23 | 58.48 | 46.6 | 35 ms | 66.12 ms |

The first Guest-ring-wake-only capture met the per-route thresholds: 56.2 swaps/s stationary, 59.64 movement mean, 57.8 lowest movement window, 19 ms worst movement p95, and 48.83 ms longest movement gap. Its telemetry-free result was not reproduced in a second route with system counters enabled: recheck64 averaged 51.53 stationary and 47.59 during movement, with a 30.0 lowest movement window, 37 ms worst p95, and 50.63 ms longest gap. Recheck64 aggregate CPU averaged 85.74% stationary and 88.27% in movement; Port CPU was about 12-13%, and GPU 3D about 38% stationary and 33% moving. This is a failed repeat under substantial aggregate CPU load; it does not isolate whether the flag or system load caused the drop. The graphics-worker-wake-only and deadline-only routes, and all three two-change routes, also missed at least one threshold. Guest-ring wake is not a validated configuration and cannot count toward the formal gate.

A full-candidate route immediately before the ablation (`recheck57`) passed: 59.9 stationary, 59.93 movement, lowest window 59.2, worst p95 19 ms, longest gap 50.34 ms. Five later full-candidate routes did not repeat it. A stock-plugin control (`recheck59`) ran between the first and second of these later candidate captures:

| Run | Stationary mean | Movement mean | Lowest five-second movement window | Worst movement-window p95 | Longest movement-window gap |
| --- | ---: | ---: | ---: | ---: | ---: |
| Full candidate recheck58 | 59.6 | 58.08 | 43.6 | 35 ms | 66.06 ms |
| Stock plugin recheck59 | 32.5 | 51.4 | 31.2 | 35 ms | 50.16 ms |
| Full candidate recheck60 | 57.13 | 52.19 | 32.3 | 35 ms | 36.3 ms |
| Full candidate recheck61 | 57.2 | 44.47 | 30.8 | 36 ms | 83.37 ms |
| Full candidate recheck62 | 59.07 | 52.03 | 30.0 | 35 ms | 67.41 ms |
| Full candidate recheck63 | 52.93 | 52.40 | 31.1 | 35 ms | 66.62 ms |

Recheck63 included Windows CPU and GPU-engine counters. The aggregate `Processor(_Total)\% Processor Time` counter averaged 94.04% during its stationary phase and 86.73% during movement; processor performance stayed near 115.9% of base, while the Port GPU 3D engine averaged 35.87% stationary and 33.19% during movement. A simultaneous process-counter snapshot showed MsMpEng at 241%, vmmemWSL at 163%, and skateport at 178% of one logical CPU; process loads sum across cores and can exceed 100%. After the game exited, the aggregate CPU counter read 96.4%, but subsequent per-core samples averaged 72-82%, and per-process counters showed about 10-12 logical-CPU equivalents of activity. These Windows counters do not reconcile exactly. They indicate substantial background CPU activity, but do not prove saturation or causality. No GPU clock or temperature was recorded. The Guest-ring-wake-only route passed once but failed its telemetry-enabled repeat under high aggregate CPU load, so it is not a validated configuration. All five later full-candidate routes missed at least one threshold, the five stock routes remain inconsistent, and the Career performance gate is not met. Audio was muted and input synthetic; audible playback and physical controller checks remain separate.
