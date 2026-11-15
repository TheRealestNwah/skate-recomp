# M4 baseline playtest

One structured playtest that checks each open M4 item before any fixes. Results replace the blanks below. Each defect becomes its own labeled GitHub issue, linked from its row. **[Inference]** The M4 order in [the roadmap](../../roadmap.md) is provisional until this is filled in.

## Setup

- Build: current `main`, stock ReXGlue SDK 0.10.1, original USA disc.
- Input: an Xbox controller for the whole run, then a two-minute keyboard spot-check with `-Keyboard`.
- Renderer: the RTV default.
- Logging: info level plus `-FrameTiming`. Use `-DebugLog` only when reproducing a specific defect.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\run-port.ps1 -FrameTiming
```

Keep logs and screenshots local. Note the boot log path that the launcher prints.

## Checklist

### Movies (closes or keeps [#38](https://github.com/TheRealestNwah/skate-recomp/issues/38))

- [ ] The EA HD bumper and intro play with moving video.
- [ ] The title-screen attract movie plays. Note any repeated frame ([#53](https://github.com/TheRealestNwah/skate-recomp/issues/53)).
- [ ] Movie audio is audible.

### Audio

- [ ] Menu music and UI sounds.
- [ ] In-game music, skate SFX (push, grind, bail) and ambience.
- [ ] Crackle, dropouts, desync, or any hang that starts with audio. Note the time and location.

### Saves

- [ ] Create a new Career save and confirm the game reports a successful save.
- [ ] Quit, relaunch and load it. Progress persists.
- [ ] Record where the Host wrote it. Expected under `build/boot/user/`, the launcher's `--user_data_root`.

### Career

- [ ] Start a new Career from the save above.
- [ ] Play until the first blocker or the end of the session. Record the last goal reached in the [Career progress log](06-career-progress.md).

### Photo and Skate Reel

- [ ] Take one photo. Note whether the preview and the saved photo show the scene.
- [ ] Record, play back and save one Skate Reel clip. Note what breaks.

### Input

- [ ] Flick-It: ollies, kickflips and grabs register as expected with the controller at the default 60 Hz guest refresh.
- [ ] Keyboard spot-check: pushing, steering, menu navigation and Shift+arrow Flick-It.

## Results

| Area | Result | Issue |
| --- | --- | --- |
| Movies | | |
| Audio | | |
| Saves | | |
| Career | | |
| Photo and Skate Reel | | |
| Input | | |
