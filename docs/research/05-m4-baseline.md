# M4 baseline playtest

The maintainer runs this checklist on an Xbox controller. Start with Movies through Saves in one session. Career can span as many sessions as needed. The maintainer records the observed results below and in the [Career progress log](06-career-progress.md).

## Setup

- Build current `main` with stock ReXGlue SDK 0.10.1 and the original USA disc. Record the build commit from `git rev-parse --short HEAD`.
- Use an Xbox controller and the default RTV path at 60 Hz. Leave both experimental flags off.
- Set `port.muteAudio` to `false` in `local.json` for the movie and audio checks. Verify the Port is audible in Windows Volume Mixer and record the playback device. Restore your preferred mute setting after the audio checks.
- Use the default audio backend for the first pass. Use `-DebugLog` when repeating a defect.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\run-port.ps1 -FrameTiming
```

Record the boot log path that the launcher prints on each launch. Keep logs and screenshots local. For a failure, record the menu, lesson, or location, the last action, what happened, and the local time. If the Port exits, record the last error line. Each confirmed defect gets its own labeled GitHub issue.

## Checklist

### Movies (closes or keeps [#38](https://github.com/TheRealestNwah/skate-recomp/issues/38))

- [ ] The EA HD bumper and intro play with moving video.
- [ ] The title-screen attract movie plays. Note any repeated frame ([#53](https://github.com/TheRealestNwah/skate-recomp/issues/53)).
- [ ] Movie audio is audible.

### Audio and input

- [ ] Listen for menu music and UI sounds. In Free Skate, check in-game music, push/grind/bail sounds and ambience.
- [ ] Load Community Center, skate for a few minutes, return to the menu, and load the Library in Downtown. Return to the menu and load Community Center again. After each transition, check whether music, ambience, and skate sounds return. For a crackle, dropout, desync, or hang, record the location and whether it began during loading or skating. [XMA streaming trace](08-xma-audio.md#streaming-smoke-test)
- [ ] If audio goes silent, repeat the same steps with `-DirectSound` on the same playback device and volume settings. Record which launch lost sound. [Audio issue #60](https://github.com/TheRealestNwah/skate-recomp/issues/60)
- [ ] Try a short D-pad tap and a held D-pad press in the main and pause menus. Check that each tap moves one item and that holding repeats steadily. Try ollies, kickflips and grabs in Free Skate.
- [ ] On a separate `-Keyboard` launch, check pushing, steering, menu navigation, and Shift+arrow Flickit.

### Saves

- [ ] Create a new test skater in Career. Complete a lesson and use Pause → Save Game. Confirm the save prompt names that skater.
- [ ] Quit the Port completely, relaunch, and pick Continue Career. Record the lesson before saving and the lesson offered after loading.
- [ ] Record where the Host wrote it. Expected under `build/boot/user/`, the launcher's `--user_data_root`.

### Career

- [ ] During the opening lessons, choose Continue on the Flickit, Pushing & Timing, and Air & Grabs completion screens. Check that each next invitation opens. [Previous lesson transition failure](https://github.com/TheRealestNwah/skate-recomp/issues/67)
- [ ] Continue the same test skater each session until the credits. Save before ending a session and confirm the checkpoint on the next launch.
- [ ] Record the last completed objective and any blocker in the [Career progress log](06-career-progress.md). If a blocker recurs after relaunch, record the shortest sequence that reproduces it.

### Photo and Skate Reel

- [ ] Once the photo feature is available, take a photo at a recognizable spot. Compare the preview, the saved photo, and the photo after relaunch. Record if any is blank, stale, or corrupted.
- [ ] Skate a short line, then open the Replay Editor. Play it, save a clip, relaunch, and reopen the saved clip. Check that playback shows the same line.

While playing through the checklist, record the scene and last action if geometry or animation visibly distorts, or if the log reports an unregistered Guest function. Include the function address when the log prints one. [Vector pack fix](https://github.com/TheRealestNwah/skate-recomp/pull/80) · [Guest method fixes](https://github.com/TheRealestNwah/skate-recomp/pull/86) · [Additional forwarding and field methods](https://github.com/TheRealestNwah/skate-recomp/issues/87) · [Selector method](https://github.com/TheRealestNwah/skate-recomp/issues/88)

## Results

| Area | Result | Issue |
| --- | --- | --- |
| Movies | | |
| Audio | | |
| Saves | | |
| Career | | |
| Photo and Skate Reel | | |
| Input | | |
