# Career progress log

Tracks one unmodified Career save from a new game to the credits. The M4 item "Career playable start to finish" is checked off when this save reaches the credits. No cheats, save editing, or copied saves.

Add one row per playtest session. Log blockers as labeled GitHub issues and link them here.

| Date | Build (commit) | Last goal or area reached | Blocker | Notes |
| --- | --- | --- | --- | --- |
| 2026-11-16 | `262f483` | Flickit tutorial completion screen | | Scripted run continued the [save created in the persistence test](07-saves.md). Cleared the two ollie prompts, both diagonal flick prompts and both shuvit prompts. The game displayed Continue and Retry. |
| 2026-11-16 | `5359ccd` | Flickit tutorial completion screen | [#67](https://github.com/TheRealestNwah/skate-recomp/issues/67) | The Port exited when selecting Continue. The log reported an unregistered Guest function at `0x82369D00`. The save payload kept its pre-run hash. |
| 2026-11-16 | `bab74f9` | Pushing & Timing invitation | | Selected Continue after completing Flickit. The next lesson appeared, and the Port shut down normally. The save payload still matched its pre-run hash. |
| 2026-11-17 | `4f7c05a` | Pushing & Timing, “Hop onto one of the ledges” | | Cleared the pushing prompt and used Save Game from Pause. The save payload changed and kept its new hash through a normal quit. Continue Career after relaunch opened the Pushing & Timing invitation. |
| 2026-11-17 | `4f7c05a` | Air & Grabs invitation | | Hopped onto a ledge, saw Lesson Complete for Pushing & Timing, and selected Continue. Air & Grabs opened. The run ended before saving this progress. |
