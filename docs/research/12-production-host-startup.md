# Production Host startup timing

## November 22, 2026

A separate launch used the production `skateport.exe` from the packaged frame-pacing build, the 60 Hz RTV windowed configuration at 1280x720, the default WASAPI endpoint, muted audio, and an isolated copy of the test save. The Port emitted 79 five-second Guest timing windows from 21:17:23 to 21:23:53. Across these windows, output averaged 57.75 swaps/s and reached 50.4 at its lowest; 14 windows were below 55 swaps/s, 25 had p95 intervals above 25 ms, and eight had a longest gap above 100 ms (maximum 158.83 ms). Several stretches held 60 swaps/s with p95 at or below 21 ms, but the full capture was not consistently within the provisional thresholds.

This production-launcher check did not reach a usable Career scene: the visible game remained in startup movie/intro content during the capture. These startup measurements are not comparable to the synthetic Career route table and do not count toward the formal Career gate. The separate default-WASAPI Career route results in [renderer performance research](04-renderer-performance.md#november-22-default-sdl-wasapi-career-route-recheck) remain the readiness evidence; the full Career playthrough is still on hold.
