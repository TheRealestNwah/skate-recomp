# skate. and Skate 3 engine comparison

Compared the user-supplied Xbox 360 retail discs on November 6, 2026. Tracking issue [#4](https://github.com/TheRealestNwah/skate-recomp/issues/4).

## Executables and imports

| Observation | skate. | Skate 3 |
| --- | --- | --- |
| Title ID / media ID | `45410813` / `7D3C6C51` | `454108E6` / `5C087C2C` |
| Disc execution and base version | `0.0.0.1` | `0.0.0.3` |
| XEX payload | AES and LZX, 32 KiB window | AES and LZX, 32 KiB window |
| Image base / entry point | `0x82000000` / `0x82DF9450` | `0x82000000` / `0x82EB4F80` |
| Loaded image bytes | 19,202,048 | 18,022,400 |
| Linked kernel and XAM version | `2.0.5766.0` | `2.0.9328.0` |
| Minimum kernel and XAM version | `2.0.5766.0` | `2.0.8955.0` |
| Additional XEX on disc | None | `data/webkit/EAWebkit.xex` |

The import comparison is between the two disc `default.xex` images. It keys on library and ordinal. ReXGlue 0.10.1 resolves the same names for every shared ordinal. The Skate 3 image contains an explicit path to `EAWebkit.xex`. That module is separate from the main XEX and excluded from the import counts.

| Library | skate. | Skate 3 | Shared | skate. only | Skate 3 only |
| --- | ---: | ---: | ---: | ---: | ---: |
| `xboxkrnl` | 191 | 178 | 157 | 34 | 21 |
| `xam` | 159 | 120 | 120 | 39 | 0 |
| **Total** | **350** | **298** | **277** | **73** | **21** |

Both import `VdInitializeEngines`, `VdInitializeRingBuffer`, `VdSetGraphicsInterruptCallback` and `VdSwap`. skate. has additional network and XAM action/input APIs such as `NetDll_WSASendTo`, `XCustomSetDynamicActions` and `XamInputGetKeystrokeEx`. Skate 3 adds kernel imports including `MmMapIoSpace`, `VdGlobalXamDevice` and `XMASetInputBufferReadOffset`.

**[Inference]** The shared graphics calls support starting with upstream ReXGlue's Xenos backend. The 73 skate.-only imports need their own implementation audit. Import overlap does not establish that calls occur on the same boot paths.

## Engine and middleware

| Marker in the decoded main XEX | skate. | Skate 3 |
| --- | ---: | ---: |
| `RW4xb2` | 1 | 1 |
| `rw::core::BigHandler` | 3 | 5 |
| `rw::audio::core::` | 46 | 73 |
| `EA::Jobs::JobScheduler` | 1 | 1 |
| `XMA input buffers` | 1 | 1 |
| `VideoEncoder_VP6` | 5 | 5 |
| `VP6 Decode Job` | 1 | 0 |
| `VideoDecoder_Vp6` | 0 | 4 |
| `EAWebkit` | 0 | 2 |

The counts use case-insensitive byte searches for each literal in the loaded image. Build paths also identify EASTL versions `1.05.01` in skate. and `1.10.05` in Skate 3. The search found no Bink, Havok, PhysX or Endorphin markers in either main XEX. This search only covers retained strings in those two binaries.

**[Inference]** The shared RW4 asset signature, named RenderWare subsystems, RefPack strings and EA job scheduler support a common engine lineage. The XEXs have different entry points, import sets and middleware builds. Function addresses, compiler helpers and jump tables should be derived from the skate. image.

## Archives and assets

| Observation | skate. | Skate 3 |
| --- | ---: | ---: |
| Standalone `.big` files | 33 | 51 |
| `BIG4` / `BIGF` | 29 / 4 | 1 / 0 |
| `EB` version 3 | 0 | 50 |
| Member count in outer archive headers | 16,531 | 45,217 |
| Standalone `.rx2` files with `89 52 57 34` signature | 12 | 23 |
| Standalone `.bh` files with `Viv4` signature | 13 | 1 |

The skate. archive layouts are recorded in the [dump inspection](02-dump-inspection.md). In Skate 3, `data/audio/music/overlays.big` retains a named `BIG4` directory. The other 50 archive headers have `EB 00 03`, a big-endian member count, an alignment field of 16, and an index variant byte of 4 (33 files) or 6 (17 files). Their declared lengths match the disc file sizes. The inventory reads EB headers and counts without unpacking the member payloads.

**[Inference]** The `.big` suffix and overlapping content categories are poor guides to the directory layout. The shared RW4 signature supports comparing assets after they are located inside each title's own archive format.

## Video and audio

Every standalone movie starts with the EA Multimedia `MVhd` header. All 31 skate. movies use the `vp60` codec tag and all 11 Skate 3 movies use `VP60`. Both attract movies carry `MV0K` video chunks. The skate. attract movie uses `SCHl` audio headers and `SCDl` audio data. The Skate 3 sample has separate `SHEN`, `SHFR` and `SHGE` audio headers. [FFmpeg's EA Multimedia demuxer](https://github.com/FFmpeg/FFmpeg/blob/master/libavformat/electronicarts.c)

Both discs carry three `.mpf` files with a `PFDx` header and three `.mus` files. Both main XEXs import XMA context management and XAudio render-driver calls, and both contain `rw::audio::core::` strings. The string diagnostics name an XAudio output voice in skate. and an XAudio2 output voice in Skate 3.

**[Inference]** Startup video and game audio need separate checks. The VP6 container evidence identifies the video codec, while XMA imports and the sound bank files point to a separate game audio path. XMA timing and graphics initialization are the first runtime risks to observe once the disc XEX builds. [Xenia Canary skate. compatibility report](https://github.com/xenia-canary/game-compatibility/issues/319)

## Reproduction

Use the [local inspection commands](../../CLAUDE.md#local-disc-inspection) for both titles. The JSON inventories record file and archive counts, and `.inspection/xex.tsv` records decoded PE sections and imports. Count the marker bytes in `.inspection/image.bin` and read the first 12 bytes of each `.vp6` to reproduce the video tag counts. The Skate 3 extraction contained 103 files and 6,404,940,920 bytes, matching the extractor's totals. The comparison used the disc images as supplied.
