# Initial survey: running skate. (Xbox 360) natively on PC

Status as of 2026-11-03. Tracking issue: #1.

Each claim cites a source. Lines marked **[Inference]** are our reasoning and still need checking.

## TL;DR

- **Approach:** static recompilation of the 360 `default.xex` into C++. This is the Unleashed Recompiled model: the toolchain translates the PPC code, and a hand-built runtime replaces the kernel, audio and GPU layers. It is not emulation. Full decompilation can come later, function by function, on top of a working recomp.
- **Toolchain:** **ReXGlue SDK**. It pairs XenonRecomp-style AOT codegen with Xenia's kernel, memory, threading and VFS runtime, plus Xenia's GPU backend as a first renderer.
- **Closest precedent:** **Skate 3 already has a working ReXGlue recomp** ([mchughalex/skate3recomp](https://github.com/mchughalex/skate3recomp)). It comes from the same studio and console, and probably shares engine lineage. Its v2.0.0 added a native D3D12/Vulkan renderer with hand-ported material shaders.
- **Existing skate 1 / skate 2 work:** we found no recomp or decomp of either.
- **Expected hard parts:**
  - XMA audio and its threading
  - Render-target resolve/readback (photos, replays, previews)
  - Memory lifetimes during asset streaming
  - 120 Hz input polling for "flick it"
  - Eventually, a native renderer

## 1. Toolchain

| Tool | Role | Notes |
| --- | --- | --- |
| [XenonRecomp](https://github.com/hedge-dev/XenonRecomp) (MIT) | PPC → C++ codegen | See the paragraph below the table. |
| [XenosRecomp](https://github.com/hedge-dev/XenosRecomp) (MIT) | Xenos shader → HLSL → DXIL/SPIR-V | No memexport, point size, vfetch bindings, integer constants or most texture-fetch features. Hardcodes Unleashed-specific assumptions; expect heavy changes. |
| [UnleashedRecomp](https://github.com/hedge-dev/UnleashedRecomp) (GPL-3.0) | Reference runtime | Kernel translation layer, plus a renderer on the plume D3D12/Vulkan HAL with background pipeline compilation. Installer checks that user-supplied files are original. |
| [ReXGlue SDK](https://github.com/rexglue/rexglue-sdk) | AOT codegen + Xenia runtime | A `FunctionDispatcher` maps guest addresses to host functions. Clang only. Latest nightly is `nightly-20261002`. Function boundaries and missing imports are still manual work. |

**XenonRecomp details** ([README](https://github.com/hedge-dev/XenonRecomp)):

- Function boundaries come from `.pdata` and `bl` targets.
- `XenonAnalyse` finds jump tables, but its patterns were tuned for Sonic Unleashed.
- setjmp/longjmp are redirected to native versions through config.
- Mid-asm hooks are supported.
- VMX128 is supported.
- Not supported: C++/SEH exceptions and MMIO.

**Other ReXGlue ports:**

- Blue Dragon
- Ninja Gaiden 2
- DBZ Budokai HD
- Raging Blast 2
- Banjo-Kazooie: Nuts & Bolts
- GoldenEye XBLA

Sources: [notebookcheck](https://www.notebookcheck.net/ReXGlue-SDK-debut-sparks-Xbox-360-to-PC-ports-for-Blue-Dragon-more.1231442.0.html), [interview](https://readonlymemo.com/rexglue-xbox-360-recompilation-interview/).

There are also **forum-only** reports of ReXGlue ports of EA titles from the same era: FIFA 07, FIFA 08, FIFA Street and NHL Legacy ([evoweb](https://evoweb.uk/threads/xbox-360-recompilations.106908/)).

### Skate 3 Recomp: what it teaches us

From the [skate3recomp releases](https://github.com/mchughalex/skate3recomp/releases) and [GamingOnLinux](https://gamingonlinux.com/2026/07/skate-3-recomp-2-0-brings-native-rendering-to-the-pc-port-of-the-xbox-classic-skate-3):

- It uses a skate-specific rexglue fork based on the 0.8.0 line.
- It needs `default.xex` plus `EAWebkit.xex`, and **Title Update 3 is required**. Without the TU, codegen leaves unresolved calls.
- Problems it hit:
  - Audio crackle and underruns.
  - A crash when world streaming freed memory the renderer was still reading.
  - Antivirus flagged the runtime's executable-memory allocation, so it had to drop it.
- Still open: skater edit, Hall of Meat, park editor, and streaming pop-in.
- **License:** the repo has **no license file** (checked via the GitHub API). We can read it for reference, but we cannot copy code from it without permission. Contacting the author is a follow-up.

## 2. The game

- **Release:** skate. by EA Black Box, Xbox 360, released 14 Sep 2007 (NA) ([Wikipedia](https://en.wikipedia.org/wiki/Skate_(2007_video_game))).
- **Title ID: `45410813`.** Related IDs: demo `4541881C`, skate 2 `4541087F`, skate 3 `454108E6` ([xenia compat](https://github.com/xenia-project/game-compatibility/issues/147)).
- **Physics:** a RenderWare package ("Drives") for the skater's body joints. Havok and Endorphin were evaluated but not used (Wikipedia).
- **Input:** read at **120 Hz** for flick-it. The runtime's input and timing layer must keep this rate.
- **[Inference] Engine:** probably the EA-internal, RenderWare-4-derived tech also used in Skate 3. Skate 3 stores `.psg` assets in `.big` archives ([Skate-3-Texture-Tools](https://github.com/Shellywell123/Skate-3-Texture-Tools)). We can verify this by diffing imports and strings between the skate 1 and Skate 3 XEXs.
- **Symbols:** we found no skate 1 or skate 2 prototype, PDB or map file. Hidden Palace lists a [Skate 3 Dec 2009 prototype](https://hiddenpalace.org/Skate_3_(Dec_7,_2009_prototype)) with debug flavors (`sk83_na_zd.xex`), but the download is dead. Its asserts and strings could help name shared engine functions.

## 3. Xenia behaviour as a hint

- **skate. is `state-playable` in Xenia Canary** ([#319](https://github.com/xenia-canary/game-compatibility/issues/319)):
  - Audio freezes can escalate into hangs. The workaround is `use_dedicated_xma_thread=false` with `xma_decoder="old"`.
  - Use D3D12; Vulkan is called incomplete.
  - Testers say the 360 version runs better than the PS3 version on RPCS3.
- **Older master tracker** ([#147](https://github.com/xenia-project/game-compatibility/issues/147)): asserts in `ppc_emit_altivec.cc` point to heavy VMX use.
- **Skate 3** depends on `readback_resolve` for billboard photos and the skater preview ([#349](https://github.com/xenia-canary/game-compatibility/issues/349)). skate 1 likely does the same for photos and Skate Reel.

## 4. Legal model

We follow the established model used by UnleashedRecomp, skate3recomp and ReXGlue:

- **We ship:** only configs, runtime code, tools and native code we wrote ourselves.
- **Users supply:** their own dumped XEX and assets.
- **Generated C++ derived from the XEX:** produced locally and never committed.
- **Affiliation:** none with EA or Microsoft.

CI enforces the first point by rejecting game-file extensions.

## 5. Proposed roadmap

1. **Dump and inspect.** Using a user-supplied dump, record the media ID, any title updates, and the XEX's compression and encryption. Extract `default.xex`, list its imports, and inventory the archives.
2. **Engine diff against Skate 3.** Compare imports, strings and middleware signatures (EA audio, Bink or other video, RW-derived libraries) to confirm how much carries over.
3. **ReXGlue bring-up.**
   - Run codegen and iterate on function boundaries, jump tables, `savegprlr`/`restgprlr` addresses and setjmp.
   - Stub missing kernel and XAM imports.
   - Goal: boot to the title screen on Xenia's GPU backend.
4. **Stability.** Fix XMA and audio threading, streaming memory lifetimes, and 120 Hz input.
5. **Native renderer.** Port materials and shaders, following skate3recomp v2's approach. Handle resolve/readback for photos and replays.
6. **PC niceties.** Resolution, uncapped framerate where safe, and keyboard/mouse.
7. **Decomp (optional).** Replace recompiled functions with hand-written C++ one at a time.

## Open questions

- Media ID, title updates, and XEX size and compression for skate 1.
- How much engine and asset format skate 1 shares with Skate 3, including whether skate 1's `.big` uses the generic `BIGF`/`BIG4` layout.
- Video codec: Bink or something else.
- License and public availability of the skate3recomp rexglue fork. The skate3recomp repo itself only has a `main` branch.
- How ReXGlue currently handles jump tables, SEH and setjmp. This needs a read of the codegen source.
- Whether the forum-only EA-title ReXGlue ports are real.
