# Vector floating-point minimum and maximum

## Reproduced defects

ReXGlue SDK 0.10.1 emits direct SSE min/max operations for `vminfp`, `vmaxfp`, and their VMX128 forms. With a NaN operand, SSE selects the right input. This discards a NaN in the left input and leaves a signaling NaN in the right input unquieted. Xenon selects the left NaN when present, otherwise the right, and quiets the selected value while retaining its sign and payload. [SDK vector emitter](https://github.com/rexglue/rexglue-sdk/blob/v0.10.1/src/codegen/builders/vector.cpp) · [Xenia Canary hardware correction](https://github.com/xenia-canary/xenia-canary/commit/105452a)

SSE also chooses the right operand when both values compare equal. For mixed signed zeros, this makes the result depend on operand order. The target instructions select positive zero for maximum and negative zero for minimum. [AltiVec manual, vmaxfp](https://www.nxp.com/docs/en/reference-manual/ALTIVECPEM.pdf#page=184) · [AltiVec manual, vminfp](https://www.nxp.com/docs/en/reference-manual/ALTIVECPEM.pdf#page=193) · [Xenia vector implementation](https://github.com/xenia-canary/xenia-canary/blob/105452a/src/xenia/cpu/backend/x64/x64_sequences.cc)

An offline executable compiled an actual Generated `vmaxfp` body from the current Port. `max(quiet NaN, 1.0)` returned `3F800000` rather than a quiet NaN. `max(+0,-0)` returned `80000000` rather than `00000000`. With 1.0 on the left and signaling NaN `7F800001` on the right, the result stayed `7F800001` instead of quieting to `7FC00001`. No game was launched. The NaN and zero-ordering defects have separate issues. [NaN issue #110](https://github.com/TheRealestNwah/skate-recomp/issues/110) · [Signed-zero issue #111](https://github.com/TheRealestNwah/skate-recomp/issues/111)

## Host correction

The Host loads both operands before writing any destination. It combines forward and reverse ordered results to select the correct zero sign, then explicitly selects and quiets a NaN when either input is unordered. The hooks enable the SDK's vector flush mode and preserve the current rounding mode. Separate wrappers handle destination aliases while avoiding duplicate register parameters in SDK hook declarations. Stock SDK 0.10.1 stays unchanged. [Host implementation](../../src/vector_minmax.cpp) · [Hook declaration defect](https://github.com/TheRealestNwah/skate-recomp/issues/97)

All 387 hook sites matched the original USA XEX instruction words and operands: 192 maximum and 195 minimum sites. These include 345 standard VMX and 42 VMX128 instructions. There are 133 sites with distinct registers, 114 overwriting the left operand, and 140 overwriting the right. None uses the same register for both inputs. Each hook continues at the following instruction. [Hook metadata](../../config/skate-vector-minmax.toml) · [USA XEX identification](02-dump-inspection.md)

## Offline verification

The independent reference orders finite IEEE float bit patterns using integer keys. It handles NaNs, denormals, and signed zeros explicitly, without relying on the Host floating-point min/max instructions. Its output matched all 652 published maximum and 652 minimum vector cases at the upstream correction. These are public synthetic instruction tests; this audit did not run Xbox hardware. [Published maximum cases](https://github.com/xenia-canary/xenia-canary/blob/105452a/src/xenia/cpu/ppc/testing/instr__gen_vmaxfp.s) · [Published minimum cases](https://github.com/xenia-canary/xenia-canary/blob/105452a/src/xenia/cpu/ppc/testing/instr__gen_vminfp.s)

The extended local probe passed 14,427,648 Host hook cases. It covered all six adapters, source preservation, all four rounding modes, both initial denormal modes, every pair of 24 special values, and 300,000 random vector pairs per mode. The special values include both signs of zero, normal/subnormal boundaries, infinities, and quiet/signaling NaNs with distinct signs and payloads. Probe files remain under ignored `build/m4-technical/`. [Host implementation](../../src/vector_minmax.cpp)

The permanent CTest target runs 224,256 cases against the same Host implementation using 4,096 random vector pairs per mode. The Host-only build and both vector CTest targets passed locally without Game files or Generated code. [Synthetic test](../../tools/test_vector_minmax.cpp) · [CTest configuration](../../CMakeLists.txt) · [CI workflow](../../.github/workflows/ci.yml)

Three separate local mutations removed NaN handling, signed-zero ordering, or NaN quieting. The permanent test rejected each with exit code 1 and printed the differing lane values. These mutated copies remain ignored under `build/m4-technical/`. [Synthetic test](../../tools/test_vector_minmax.cpp)

The full local Port build passed with the existing `-FastCodegen` option and stock SDK runtime. The emission audit found all 387 min/max calls followed by their next-instruction jumps, 771 unique configured hooks overall, and all 115,508 dispatch registrations retained. Markdown lint passed for the changed guidance and notes. [Build entry point](../../scripts/build-port.ps1) · [Hook includes](../../config/skate.toml)

**[Inference]** Incorrect min/max results could affect Guest calculations that consume them, but these instruction probes do not establish a specific gameplay symptom. The existing skater, board, geometry, animation, and material checks in the [controller test plan](05-m4-baseline.md) now link this audit. Human playtest results and the Career log remain unchanged.
