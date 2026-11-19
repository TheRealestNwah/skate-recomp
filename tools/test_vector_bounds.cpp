#include "vector_bounds.h"

#include <array>
#include <bit>
#include <cstdio>
#include <cstring>

#include <rex/ppc.h>

namespace {

uint32_t ReferenceLane(uint32_t value, uint32_t bound) {
    // Normalize Guest vector denormals before the independent scalar comparison.
    if ((value & 0x7F800000) == 0) value &= 0x80000000;
    if ((bound & 0x7F800000) == 0) bound &= 0x80000000;
    if ((value & 0x7FFFFFFF) > 0x7F800000 || (bound & 0x7FFFFFFF) > 0x7F800000) {
        return 0xC0000000;
    }
    const double x = std::bit_cast<float>(value);
    const double limit = std::bit_cast<float>(bound);
    return (x > limit ? 0x80000000 : 0) | (x < -limit ? 0x40000000 : 0);
}

bool Check(const PPCVRegister& source, const PPCVRegister& bounds,
           unsigned rounding, bool initial_flush, uint64_t& checked) {
    PPCVRegister expected{};
    for (size_t lane = 0; lane < 4; ++lane) {
        expected.u32[lane] = ReferenceLane(source.u32[lane], bounds.u32[lane]);
    }
    const uint32_t expected_cr6 = (expected.u64[0] | expected.u64[1]) == 0 ? 2 : 0;
    for (unsigned variant = 0; variant < 5; ++variant) {
        _mm_setcsr(0x1F80 | (rounding << 13) | (initial_flush ? 0x8040 : 0));
        PPCFPSCRRegister fpscr{};
        fpscr.InitHost();
        PPCVRegister left = source, right = bounds, result{};
        PPCCRRegister condition{};
        condition.set_raw(15);
        switch (variant) {
            case 0: SkateCompareBounds(result, left, right, fpscr); break;
            case 1: SkateCompareBoundsLeft(left, right, fpscr); result = left; break;
            case 2: SkateCompareBoundsRight(right, left, fpscr); result = right; break;
            case 3:
                SkateCompareBoundsLeftRecord(left, right, fpscr, condition);
                result = left;
                break;
            case 4:
                SkateCompareBoundsRightRecord(right, left, fpscr, condition);
                result = right;
                break;
        }
        const bool left_preserved = variant == 1 || variant == 3 ||
                                    std::memcmp(&left, &source, sizeof(left)) == 0;
        const bool right_preserved = variant == 2 || variant == 4 ||
                                     std::memcmp(&right, &bounds, sizeof(right)) == 0;
        if (std::memcmp(&result, &expected, sizeof(result)) != 0 ||
            condition.raw() != (variant >= 3 ? expected_cr6 : 15) ||
            !left_preserved || !right_preserved ||
            (_mm_getcsr() & 0x8040) != 0x8040 ||
            (_mm_getcsr() & 0x6000) != (rounding << 13)) {
            std::fprintf(stderr, "Bounds failure at case %llu, adapter %u, rounding %u, flush %u\n",
                         static_cast<unsigned long long>(checked), variant, rounding, initial_flush);
            for (size_t lane = 0; lane < 4; ++lane) {
                std::fprintf(stderr, "lane %zu: value=%08X bound=%08X result=%08X expected=%08X\n",
                             lane, source.u32[lane], bounds.u32[lane],
                             result.u32[lane], expected.u32[lane]);
            }
            std::fprintf(stderr, "CR6=%X expected=%X\n", condition.raw(),
                         variant >= 3 ? expected_cr6 : 15);
            return false;
        }
        ++checked;
    }
    return true;
}

bool Run(uint64_t& checked) {
    constexpr std::array<uint32_t, 20> values{
        0, 0x80000000, 1, 0x80000001, 0x007FFFFF, 0x807FFFFF,
        0x00800000, 0x80800000, 0x3F800000, 0xBF800000, 0x3F800001, 0xBF800001,
        0x7F7FFFFF, 0xFF7FFFFF, 0x7F800000, 0xFF800000,
        0x7FC00000, 0xFFC00001, 0x7F800001, 0xFFBFFFFF,
    };
    uint32_t state = 0x81358ACE;
    const auto random = [&state] {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        return state;
    };
    for (unsigned rounding = 0; rounding < 4; ++rounding) {
        for (bool flush : {false, true}) {
            for (uint32_t value : values) {
                for (uint32_t bound : values) {
                    PPCVRegister left{}, right{};
                    for (size_t lane = 0; lane < 4; ++lane) {
                        left.u32[lane] = value;
                        right.u32[lane] = bound;
                    }
                    if (!Check(left, right, rounding, flush, checked)) return false;
                }
            }
            // Every combination of in/out-of-bounds lanes, including boundary equality.
            for (unsigned mask = 0; mask < 16; ++mask) {
                PPCVRegister left{}, right{};
                for (size_t lane = 0; lane < 4; ++lane) {
                    left.f32[lane] = (mask & (1U << lane)) ? 2.0f : 1.0f;
                    right.f32[lane] = 1.0f;
                }
                if (!Check(left, right, rounding, flush, checked)) return false;
            }
            for (unsigned sample = 0; sample < 4096; ++sample) {
                PPCVRegister left{}, right{};
                for (size_t lane = 0; lane < 4; ++lane) {
                    left.u32[lane] = random();
                    right.u32[lane] = random();
                }
                if (!Check(left, right, rounding, flush, checked)) return false;
            }
        }
    }
    return true;
}

} // namespace

int main() {
    const unsigned saved_mxcsr = _mm_getcsr();
    uint64_t checked = 0;
    const bool passed = Run(checked);
    _mm_setcsr(saved_mxcsr);
    if (!passed) return 1;
    std::printf("Passed %llu Host bounds cases\n", static_cast<unsigned long long>(checked));
    return 0;
}
