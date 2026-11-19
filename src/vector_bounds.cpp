#include "vector_bounds.h"

#include <rex/ppc.h>

void SkateCompareBounds(PPCVRegister& destination, PPCVRegister& source,
                        PPCVRegister& bounds, PPCFPSCRRegister& fpscr) {
    fpscr.enableFlushMode();
    // Snapshot both inputs before writing a destination that may alias either.
    const auto value = simde_mm_load_ps(source.f32);
    const auto limit = simde_mm_load_ps(bounds.f32);
    const auto sign = simde_mm_castsi128_ps(simde_mm_set1_epi32(INT32_MIN));
    // Include unordered lanes explicitly; the pinned SIMDe cmpn* aliases are ordered.
    const auto unordered = simde_mm_cmpunord_ps(value, limit);
    const auto upper = simde_mm_and_ps(
        simde_mm_or_ps(simde_mm_cmpgt_ps(value, limit), unordered), sign);
    const auto lower = simde_mm_and_ps(
        simde_mm_or_ps(simde_mm_cmplt_ps(value, simde_mm_xor_ps(limit, sign)), unordered),
        simde_mm_castsi128_ps(simde_mm_set1_epi32(0x40000000)));
    simde_mm_store_ps(destination.f32, simde_mm_or_ps(upper, lower));
}

void SkateCompareBoundsLeft(PPCVRegister& vector, PPCVRegister& bounds,
                            PPCFPSCRRegister& fpscr) {
    SkateCompareBounds(vector, vector, bounds, fpscr);
}

void SkateCompareBoundsRight(PPCVRegister& vector, PPCVRegister& source,
                             PPCFPSCRRegister& fpscr) {
    SkateCompareBounds(vector, source, vector, fpscr);
}

void SkateCompareBoundsLeftRecord(PPCVRegister& vector, PPCVRegister& bounds,
                                  PPCFPSCRRegister& fpscr, PPCCRRegister& condition) {
    SkateCompareBoundsLeft(vector, bounds, fpscr);
    condition.set_raw((vector.u64[0] | vector.u64[1]) == 0 ? 2 : 0);
}

void SkateCompareBoundsRightRecord(PPCVRegister& vector, PPCVRegister& source,
                                   PPCFPSCRRegister& fpscr, PPCCRRegister& condition) {
    SkateCompareBoundsRight(vector, source, fpscr);
    condition.set_raw((vector.u64[0] | vector.u64[1]) == 0 ? 2 : 0);
}

