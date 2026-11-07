# Glossary

**skate.** — The 2007 Xbox 360 game by EA Black Box that this project ports. Called "skate 1" only when it needs telling apart from skate 2 or Skate 3.

**Port** — The PC build of skate. that this project produces. It runs natively, with no emulator.

**Recomp (static recompilation)** — Translating the game's PowerPC machine code into C++ ahead of time, then compiling that C++ for PC. It is not emulation: no code is translated while the game runs.

**Decomp (decompilation)** — Rewriting recompiled functions as readable, hand-written C++. Optional, and only after 1.0.

**Guest** — Anything belonging to the original Xbox 360 side: guest code, guest addresses, guest memory.

**Host** — The PC the Port runs on, and its code, memory and APIs.

**Runtime** — The host code that stands in for the Xbox 360's kernel, audio, input, file system and GPU, so the recompiled game code has something to call.

**Game files** — Anything from the user's own copy of the game: the disc image, the executable (XEX), archives, assets, and anything extracted or generated from them. Game files never enter the repository.

**Generated code** — The C++ that recomp produces from the XEX. It counts as Game files: it is produced on the user's machine and never committed.

**Title update (TU)** — An official patch for the game, applied over the disc XEX. A TU is Game files.

**Title screen boot** — The milestone where the Port starts and reaches the game's title screen on the user's PC.

**1.0** — Full career playable start to finish, a native renderer, and PC options (resolution, framerate, keyboard/mouse), on Windows.
