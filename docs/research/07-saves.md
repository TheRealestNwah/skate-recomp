# Local save persistence

The original USA disc imports 16 `XamContent*` functions. The Host audits the resolved imports at boot. ReXGlue SDK 0.10.1 supplies the content manager and filesystem implementation. [XEX import inventory](02-dump-inspection.md) · [Host import audit](../../src/kernel_audit.cpp) · [Pinned SDK release](https://github.com/rexglue/rexglue-sdk/releases/tag/v0.10.1)

## Imported functions

| XAM ordinal | Import | ReXGlue SDK 0.10.1 behavior |
| --- | --- | --- |
| `0259` | `XamContentCreateEx` | Creates or opens the package according to the disposition flags, writes a header on creation, and mounts a writable Guest root. |
| `025A` | `XamContentClose` | Closes the package and removes its Guest mount. |
| `025B` | `XamContentDelete` | Deletes the package through the content manager. |
| `025C` | `XamContentCreateEnumerator` | Lists packages on disk for the SDK profile and the common XUID. Uses per-package headers when present. |
| `025D` | `XamContentCreateDeviceEnumerator` | Enumerates the SDK's synthetic HDD and optical drive. |
| `025E` | `XamContentGetDeviceData` | Returns synthetic device data, including 20 GiB total and 3 GiB free for the HDD. |
| `025F` | `XamContentGetDeviceName` | Returns the synthetic device's name. |
| `0260` | `XamContentSetThumbnail` | Writes the content thumbnail through the content manager. |
| `0261` | `XamContentGetThumbnail` | Reads the content thumbnail through the content manager. |
| `0262` | `XamContentGetCreator` | Checks that the package exists. For saved games it reports the SDK profile as the creator. |
| `0263` | `XamContentLaunchImage` | SDK export stub. |
| `0265` | `XamContentGetDeviceState` | Reports a recognized synthetic device as connected. |
| `0266` | `XamContentGetLicenseMask` | Returns the `license_mask` SDK setting, which defaults to zero. |
| `0267` | `XamContentFlush` | Acknowledges success without flushing. |
| `0268` | `XamContentResolve` | Unimplemented body with an assertion and a not-found return. The Host audit classifies it as a stub. |
| `026A` | `XamContentInstall` | SDK export stub. |

[SDK content exports](https://github.com/rexglue/rexglue-sdk/blob/v0.10.1/src/kernel/xam/xam_content.cpp) · [SDK device exports](https://github.com/rexglue/rexglue-sdk/blob/v0.10.1/src/kernel/xam/xam_content_device.cpp) · [SDK content manager](https://github.com/rexglue/rexglue-sdk/blob/v0.10.1/src/system/xam/content_manager.cpp)

The Host reports the three unavailable imports in the table as SDK stubs. It reports `XamContentFlush`, `NtFlushBuffersFile` and `XamShowDeviceSelectorUI` as SDK fixed fallbacks. Both flush calls acknowledge success without flushing. The selector supplies the synthetic HDD (device 1). [Host import audit](../../src/kernel_audit.cpp) · [SDK device selection](https://github.com/rexglue/rexglue-sdk/blob/v0.10.1/src/kernel/xam/xam_ui.cpp#L535-L547) · [SDK file I/O](https://github.com/rexglue/rexglue-sdk/blob/v0.10.1/src/kernel/xboxkrnl/xboxkrnl_io.cpp#L600-L608) · [Save flush issue #57](https://github.com/TheRealestNwah/skate-recomp/issues/57) · [Audit issue #58](https://github.com/TheRealestNwah/skate-recomp/issues/58)

## Host files

The launcher passes the absolute `build/boot/user` path as `--user_data_root` on each launch. The SDK uses that directory as its content root and mounts each open package as a writable Host directory. skate. mounts its skater save as `rmcsave:`. With the stock offline profile and this disc, the paths are:

```text
build/boot/user/B13EBABEBABEBABE/45410813/
  00000001/ALIAS_<name>/ALIAS_<name>
  Headers/00000001/ALIAS_<name>.header
```

The first component under `user` is the SDK profile XUID, `45410813` is the title ID, and `00000001` identifies saved-game content. The header holds package metadata. The Guest writes its payload inside the mounted directory through regular file I/O. [Launcher](../../scripts/run-port.ps1) · [SDK kernel state](https://github.com/rexglue/rexglue-sdk/blob/v0.10.1/src/system/kernel_state.cpp#L71-L75) · [SDK profile](https://github.com/rexglue/rexglue-sdk/blob/v0.10.1/src/system/xam/user_profile.cpp#L22-L30) · [SDK content manager](https://github.com/rexglue/rexglue-sdk/blob/v0.10.1/src/system/xam/content_manager.cpp)

## Unattended Career save check

On November 15, 2026, the test used the existing scripted controller from the renderer runs, the original USA disc, stock ReXGlue SDK 0.10.1, and RTV with draw extent estimation at the default 60 Hz. The test used a new, isolated `--user_data_root` under ignored `build/`, with audio muted. Local logs and frame captures stayed with the test. [Scripted controller precedent](04-renderer-performance.md)

1. At the title screen, the game reported that it found no skate. profile, and the boot log showed zero content items. The controller created skater `12` and enabled autosave. The game displayed a saving prompt and proceeded to the main menu. The SDK mounted and unmounted `rmcsave:`, and Windows showed a 4,194,368-byte `ALIAS_12` payload and a 328-byte header.
2. The controller started New Career, changed the stance from Goofy to Regular, and confirmed the Create a Skater save. The log showed another enumeration and two open/close cycles of `rmcsave:`. The payload's SHA-256 changed from the initial skater save.
3. The test closed the Port through its window and verified exit code 0. The payload and header hashes matched before and after exit.
4. A second process started with the same data root. The log showed one content item and fresh mounts of `rmcsave:`. Skater Select listed `12`, selecting it loaded the skater, and the main menu offered Continue Career. Both files still matched their pre-quit hashes after loading the skater.
5. Continue Career loaded the Suburbs and presented the first Flickit tutorial in the skate park.

**[Inference]** The matching files and resumed Career show that the stock save path persists through a normal quit and relaunch. The observed path requires no Host or SDK change. This run covered the initial Career save. The controller baseline should repeat the check after completing a goal and confirm the resumed goal or location using the normal launcher.
