#include "vector_minmax.h"

#include <rex/ppc.h>

namespace {

template<bool maximum>
void MinMax(PPCVRegister& destination, const PPCVRegister& left,
            const PPCVRegister& right, PPCFPSCRRegister& fpscr) {
    fpscr.enableFlushMode();
    const auto a = simde_mm_load_ps(left.f32);
    const auto b = simde_mm_load_ps(right.f32);
    const auto forward = maximum ? simde_mm_max_ps(a, b) : simde_mm_min_ps(a, b);
    const auto reverse = maximum ? simde_mm_max_ps(b, a) : simde_mm_min_ps(b, a);
    // Combine equal results so mixed signed zeros select +0 for max and -0 for min.
    const auto ordered = maximum ? simde_mm_and_ps(forward, reverse)
                                 : simde_mm_or_ps(forward, reverse);
    // Xenon selects the left NaN when present, otherwise the right, and quiets it.
    const auto nan = simde_mm_blendv_ps(b, a, simde_mm_cmpunord_ps(a, a));
    const auto quiet_nan = simde_mm_or_ps(
        nan, simde_mm_castsi128_ps(simde_mm_set1_epi32(0x00400000)));
    simde_mm_store_ps(destination.f32,
                     simde_mm_blendv_ps(ordered, quiet_nan, simde_mm_cmpunord_ps(a, b)));
}

} // namespace

void SkateVectorMax(PPCVRegister& destination, PPCVRegister& left,
                    PPCVRegister& right, PPCFPSCRRegister& fpscr) {
    MinMax<true>(destination, left, right, fpscr);
}

void SkateVectorMaxLeft(PPCVRegister& vector, PPCVRegister& right, PPCFPSCRRegister& fpscr) {
    SkateVectorMax(vector, vector, right, fpscr);
}

void SkateVectorMaxRight(PPCVRegister& vector, PPCVRegister& left, PPCFPSCRRegister& fpscr) {
    SkateVectorMax(vector, left, vector, fpscr);
}

void SkateVectorMin(PPCVRegister& destination, PPCVRegister& left,
                    PPCVRegister& right, PPCFPSCRRegister& fpscr) {
    MinMax<false>(destination, left, right, fpscr);
}

void SkateVectorMinLeft(PPCVRegister& vector, PPCVRegister& right, PPCFPSCRRegister& fpscr) {
    SkateVectorMin(vector, vector, right, fpscr);
}

void SkateVectorMinRight(PPCVRegister& vector, PPCVRegister& left, PPCFPSCRRegister& fpscr) {
    SkateVectorMin(vector, left, vector, fpscr);
}
